// Copyright 2008, Google Inc.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//  3. Neither the name of Google Inc. nor the names of its contributors may be
//     used to endorse or promote products derived from this software without
//     specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
// EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <string>

#include "gears/installer/iemobile/cab_updater.h"

#include "gears/base/common/common_ie.h"
#include "gears/base/common/thread_locals.h"
#include "gears/base/common/wince_compatibility.h"
#include "gears/base/ie/activex_utils.h"
#include "gears/base/ie/bho.h"
#include "gears/blob/blob_interface.h"
#include "gears/blob/blob_utils.h"
#include "gears/installer/iemobile/resource.h"

const char16* kUpgradeUrl = reinterpret_cast<const char16*>(  // STRING16()
    // TODO(andreip): When updating the Gears 'guid' field, consider switching
    // 'application' to a GUID that means Pocket IE (to match Firefox pings).
    // Also remind cprince to update the stats logic when 'guid' changes.
    L"http://tools.google.com/service/update2/ff?"
    L"guid=%7Bc3fc95dBb-cd75-4f3d-a586-bcb7D004784c%7D"
    L"&version=" PRODUCT_VERSION_STRING
    L"&application=%7Bc3fc95dBb-cd75-4f3d-a586-bcb7D004784c%7D"
    L"&appversion=1.0"
#ifdef OFFICIAL_BUILD
    L"&os=wince&dist=google");
#else
    L"&os=wince&dist=google&dev=1");
#endif
// The topic for the message system.
const char16* kTopic = STRING16(L"Cab Update Event");
// The key for ThreadLocals.
const ThreadLocals::Slot kThreadLocalKey = ThreadLocals::Alloc();
// String constants for XML parsing
// Query language
const char16* kQueryLanguage = STRING16(L"XPath");
const char16* kQueryLanguageProperty = STRING16(L"SelectionLanguage");
// The document namespaces (space delimited)
const char16* kNamespace =
    STRING16(L"xmlns:RDF='http://www.w3.org/1999/02/22-rdf-syntax-ns#' "
             L"xmlns:em='http://www.mozilla.org/2004/em-rdf#'");
const char16* kNamespaceProperty = STRING16(L"SelectionNamespaces");
// The query for version elements: select the first em:version descendant
// of the root element.
const char16* kVersionQuery = STRING16(L"//em:version[1]");
// The query for update link elements: select the first em:updateLink descendant
// of the root element.
const char16* kUpdateLinkQuery = STRING16(L"//em:updateLink[1]");
// The delay before the first update.
const int32 kFirstUpdatePeriod = (1000 * 60 * 2);  // 2 minutes
// The time interval between the rest of the update checks.
const int32 kUpdatePeriod = (1000 * 60 * 60 * 24);  // 24 hours
// A grace period after a connection event. Setting up a new connection
// can take time so we wait a little after the event.
const int32 kGracePeriod = (1000 * 30);  // 30 seconds

// A simple task that visits the upgrade url and stops after the first redirect.
class VersionFetchTask : public AsyncTask {
 public:
  VersionFetchTask() : AsyncTask(NULL) {}
  bool Init();

  enum {
    VERSION_FETCH_TASK_COMPLETE = 0
  };

  const char16* Url() const;
  const char16* LatestVersion() const;

 private:
  // AsyncTask
  virtual void Run();
  // Internal

  // Parses XML and extracts the Gears version number and download URL.
  bool ExtractVersionAndDownloadUrl(const std::string16& xml);

  std::string16 url_;
  std::string16 latest_version_;

  DISALLOW_EVIL_CONSTRUCTORS(VersionFetchTask);
};

// A thread that wakes up periodically and does the check.
class PeriodicChecker : public CWindowImpl<PeriodicChecker> {
 public:
  // Simple factory method.
  static PeriodicChecker* CreateChecker();
  // Initializes the checker and sets the desired times between the updates.
  bool Init(int32 first_period, int32 normal_period, int32 grace_period);
  // Starts the checker thread.
  void Start();
  // Stops the checker thread.
  void Stop();
  ~PeriodicChecker();

 private:
  // CWindowImpl
  BEGIN_MSG_MAP(PeriodicChecker)
    MESSAGE_HANDLER(WM_TIMER, OnTimer)
    MESSAGE_HANDLER(WM_FETCH_TASK_COMPLETE, OnComplete)
  END_MSG_MAP()

    // Internal
  PeriodicChecker();
  // TODO(andreip): aggregate all Gears WM_USER messages in the same
  // header to avoid overlap.
  static const int kUpdateTaskMessageBase = WM_USER;
  static const int
    WM_FETCH_TASK_COMPLETE =
        VersionFetchTask::VERSION_FETCH_TASK_COMPLETE + kUpdateTaskMessageBase;
  // Message handlers.
  LRESULT OnTimer(UINT message, WPARAM w, LPARAM l, BOOL& handled);
  LRESULT OnComplete(UINT message, WPARAM success, LPARAM task, BOOL& handled);
  // This message handler is different because it needs to be registered at
  // runtime with RegisterWindowMessage (it's comming from the connection
  // manager process and that API only exposes a string).
  void OnConnectionStatusChanged();
  // Thread entry.
  static unsigned int _stdcall ThreadMain(void* checker);
  // Runs the Windows message loop machinery (essentially a lot of boilerplate).
  void RunMessageLoop();
  // Timer utilitiy.
  void ResetTimer(int32 period);  
  // The delay before the first check.
  int32 first_period_;
  // The normal time interval between checks.
  int32 normal_period_;
  // The time interval to wait after a connection up event.
  int32 grace_period_;
  // When this is signaled, the thread exists.
  CEvent stop_event_;
  // The thread handle.
  HANDLE thread_;
  // The window handle.
  HWND window_;
  // The timer ID.
  int timer_;
  // The task responsible for doing the HTTP requests to upgrade URL.
  VersionFetchTask task_;
  // Is the task running?
  bool is_running_;
  // Was this a normal timer evenr?
  bool normal_timer_event_;
  // The message ID used for listening to connection status events.
  uint32 con_mgr_msg_;

  DISALLOW_EVIL_CONSTRUCTORS(PeriodicChecker);
};

// -----------------------------------------------------------------------------
// CabUpdater

CabUpdater::CabUpdater()
    : checker_(PeriodicChecker::CreateChecker()),
      is_showing_update_dialog_(false) {
}

CabUpdater::~CabUpdater() {
  ASSERT(checker_);
  delete checker_;
}

void CabUpdater::SetSiteAndStart(IWebBrowser2* browser) {
  ASSERT(checker_);
  browser_ = browser;
  if (checker_->Init(kFirstUpdatePeriod, kUpdatePeriod, kGracePeriod)) {
    MessageService::GetInstance()->AddObserver(this, kTopic);
    // We get stopped when the DLL unloads.
    ThreadLocals::SetValue(kThreadLocalKey, this, &Stop);
    checker_->Start();
  }
}

void CabUpdater::OnNotify(MessageService *service,
                          const char16 *topic,
                          const NotificationData *data) {
  // The user is already looking at the update dialog. Can only happen
  // in rare circumstances, when an update was found, the browser is running
  // but the user hasn't looked at it in the last 24 hours.
  if (is_showing_update_dialog_) return;

  is_showing_update_dialog_ = true;
  // Get the event in the right format.
  const SerializableString16* update_event =
      static_cast<const SerializableString16*>(data);
  // Check if the browser is busy.
  VARIANT_BOOL is_busy;
  browser_->get_Busy(&is_busy);
  // Detect the browser state.
  READYSTATE state;
  browser_->get_ReadyState(&state);
  // Only show the update dialog if the browser isn't in the middle of a
  // download or showing some other notification to the user.
  if (!is_busy && state == READYSTATE_COMPLETE) {
    HWND browser_window = BrowserHelperObject::GetBrowserWindow();
    if (browser_window && ShowUpdateDialog(browser_window)) {
        // Convert the URL to the format expected by the browser object.
        CComBSTR url(update_event->string_.c_str());
        browser_->Navigate(url, NULL, NULL, NULL, NULL);
    }
  }
  is_showing_update_dialog_ = false;
}

// Internal

// static
void CabUpdater::Stop(void* self) {
  ASSERT(self);
  CabUpdater* updater = reinterpret_cast<CabUpdater*>(self);
  updater->checker_->Stop();
  MessageService::GetInstance()->RemoveObserver(updater, kTopic);
}

bool CabUpdater::ShowUpdateDialog(HWND browser_window) {
  HMODULE module = GetModuleHandle(PRODUCT_SHORT_NAME);
  if (!module) return false;
  // Load the dialog strings
  LPCTSTR message = reinterpret_cast<LPCTSTR>(
      LoadString(module,
                 IDS_UPGRADE_MESSAGE,
                 NULL,
                 0));
  LPCTSTR title = reinterpret_cast<LPCTSTR>(
      LoadString(module,
                 IDS_UPGRADE_DIALOG_TITLE,
                 NULL,
                 0));
  // Can't communicate with the user. Something is terribly wrong. Abort.
  if (!message || !title) return false;

  return (IDYES == MessageBox(browser_window,
                              message,
                              title,
                              MB_YESNO | MB_ICONQUESTION));
}

// -----------------------------------------------------------------------------
// VersionFetchTask

bool VersionFetchTask::Init() {
  if (is_initialized_) return true;
  return AsyncTask::Init();
}

const char16* VersionFetchTask::LatestVersion() const {
  return latest_version_.c_str();
}

const char16* VersionFetchTask::Url() const {
  return url_.c_str();
}

// AsyncTask
void VersionFetchTask::Run() {
  WebCacheDB::PayloadInfo payload;
  scoped_refptr<BlobInterface> payload_data;
  bool was_redirected;
  std::string16 error_msg;
  std::string16 url;
  bool success = false;
  if (ActiveXUtils::IsOnline() &&
      AsyncTask::HttpGet(kUpgradeUrl,
                         true,
                         NULL,
                         NULL,
                         NULL,
                         &payload,
                         &payload_data,
                         &was_redirected,
                         &url,
                         &error_msg)) {
    std::string16 xml;
    const std::string16 charset;
    // payload.data can be empty in case of a 30x response.
    // The update server does not redirect, so we treat this as an error.
    if (payload_data->Length() &&
        BlobToString16(payload_data.get(), charset, &xml) &&
        ExtractVersionAndDownloadUrl(xml)) {
      success = true;
    }
  }
  NotifyListener(VERSION_FETCH_TASK_COMPLETE, success);
}

// Internal
bool VersionFetchTask::ExtractVersionAndDownloadUrl(const std::string16& xml) {
  CComPtr<IXMLDOMDocument2> document;
  // Note that CoInitializeEx is called by the parent class.
  if (FAILED(document.CoCreateInstance(
      CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER))) {
    return false;
  }
  ASSERT(document);
  document->put_async(VARIANT_FALSE);
  VARIANT_BOOL success;
  // Load the document
  if (FAILED(document->loadXML(const_cast<BSTR>(xml.c_str()), &success)) ||
      success == VARIANT_FALSE) {
    return false;
  }
  // Set the query language and namespace
  VARIANT  var = {0};
  CComBSTR bstr(kNamespace);
  bstr.CopyTo(&var);
  HRESULT hr = document->setProperty(const_cast<BSTR>(kNamespaceProperty), var);
  VariantClear(&var);
  if (FAILED(hr)) return false;

  bstr = kQueryLanguage;  // this frees the old string stored by bstr.
  bstr.CopyTo(&var);
  hr = document->setProperty(const_cast<BSTR>(kQueryLanguageProperty), var);
  VariantClear(&var);
  if (FAILED(hr)) return false;

  // Run the version query
  IXMLDOMNodeList* node_list = NULL;
  long number_of_nodes = 0;  // get_length() needs a ptr to long
  if (FAILED(document->selectNodes(const_cast<BSTR>(kVersionQuery),
                                   &node_list)) || 
      FAILED(node_list->get_length(&number_of_nodes))) {
    return false;
  }
  // We expect 0 or 1 element, depending on whether an update is available.
  // We return early if we get 0 elements (no updates available).
  if (number_of_nodes == 0) return false;
  ASSERT(number_of_nodes == 1);
  IXMLDOMNode* node;
  if (FAILED(node_list->get_item(0, &node))) {
    return false;
  }
  BSTR version_text;
  if (FAILED(node->get_text(&version_text))) {
    return false;
  }
  latest_version_ = version_text;
  // Hurray, done with the version. Now the update link.
  number_of_nodes = 0;
  if (FAILED(document->selectNodes(const_cast<BSTR>(kUpdateLinkQuery),
                                   &node_list)) ||
      FAILED(node_list->get_length(&number_of_nodes))) {
    return false;
  }
  // We expect 0 or 1 element, depending on whether an update is available.
  // We return early if we get 0 elements (no updates available).
  if (number_of_nodes == 0) return false;
  ASSERT(number_of_nodes == 1);
  if (FAILED(node_list->get_item(0, &node))) {
    return false;
  }
  BSTR update_link;
  if (FAILED(node->get_text(&update_link))) {
    return false;
  }
  url_ = update_link;
  return true;
}

// -----------------------------------------------------------------------------
// PeriodicChecker

// static
PeriodicChecker* PeriodicChecker::CreateChecker() {
  return new PeriodicChecker();
}

bool PeriodicChecker::Init(int32 first_period, int32 normal_period,
                           int32 grace_period) {
  first_period_ = first_period;
  normal_period_ = normal_period;
  grace_period_ = grace_period;
  return (TRUE == stop_event_.Create(NULL, FALSE, FALSE, NULL));
}

PeriodicChecker::PeriodicChecker()
    : first_period_(0),
      normal_period_(0),
      grace_period_(0),
      thread_(NULL),
      timer_(1),
      window_(NULL),
      is_running_(false),
      normal_timer_event_(false) {
}

PeriodicChecker::~PeriodicChecker() {
  ASSERT(!thread_);
}

void PeriodicChecker::Start() {
  ASSERT(!thread_);
  // Start the thread.
  thread_ = reinterpret_cast<HANDLE>(_beginthreadex(NULL, 0, &ThreadMain,
                                                    this, 0, NULL));
  ASSERT(thread_ != NULL);
}

void PeriodicChecker::Stop() {
  // This is called only from the main thread, when the DLL unloads.
  ASSERT(thread_);
  stop_event_.Set();
}

unsigned int __stdcall PeriodicChecker::ThreadMain(void* checker) {
  PeriodicChecker* self = reinterpret_cast<PeriodicChecker*>(checker);
  self->RunMessageLoop();
  return 0;
}

void PeriodicChecker::RunMessageLoop() {
  // Create the thread window.
  window_ = Create(kMessageOnlyWindowParent,    // parent
                   NULL,                        // position
                   NULL,                        // name
                   kMessageOnlyWindowStyle);    // style
  // Set the timer for the first wait.
  SetTimer(static_cast<unsigned>(timer_),
           static_cast<unsigned>(first_period_),
           NULL);
  con_mgr_msg_ = RegisterWindowMessage(CONNMGR_STATUS_CHANGE_NOTIFICATION_MSG);
  // Run the loop.
  bool done = false;
  HANDLE handles[] = { stop_event_.m_h };
  // Run the message loop.
  while (!done) {
    DWORD rv = MsgWaitForMultipleObjectsEx(ARRAYSIZE(handles),
                                           handles,
                                           INFINITE,
                                           QS_POSTMESSAGE | WM_TIMER,
                                           0);
    if (rv == WAIT_OBJECT_0) {
      // stop_event was signalled.
      done = true;
    } else if (rv == WAIT_OBJECT_0 + ARRAYSIZE(handles)) {
      // We have a posted message or a WM_TIMER in the queue. Dispatch them
      // and go back to sleep.
      MSG msg;
      while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        // We handle this here since, at compile time, we don't know
        // the value of con_mgr_msg_.
        if (msg.message == con_mgr_msg_) OnConnectionStatusChanged();
        DispatchMessage(&msg);
      }
    } else {
      // Something else happened.
      done = true;
    }
  }
  // Kill the timer.
  KillTimer(timer_);
  // Stop any running tasks.
  task_.Abort();
  // Destroy the thread window.
  DestroyWindow();
  // Close and null the thread handle.
  CloseHandle(thread_);
  thread_ = NULL;
}

LRESULT PeriodicChecker::OnTimer(UINT message,
                                 WPARAM w,
                                 LPARAM l,
                                 BOOL& handled) {
  if (!normal_timer_event_) {
    normal_timer_event_ = true;
    ResetTimer(normal_period_);
  }
  // Depending on how the periodic checker is configured, this can fire
  // while a task is running. We ignore such events.
  if (!is_running_) {
    if (task_.Init()) {
      is_running_ = true;
      task_.SetListenerWindow(window_, kUpdateTaskMessageBase);
      task_.Start();
    }
  }
  handled = TRUE;
  return TRUE;
}

LRESULT PeriodicChecker::OnComplete(UINT message,
                                    WPARAM success,
                                    LPARAM task,
                                    BOOL& handled) {
  is_running_ = false;
  if (success) {
    // We check for differences between the version strings. If there is a
    // difference, the server version wins.
    if (wcslen(task_.LatestVersion()) &&
        wcslen(task_.Url()) &&
        wcscmp(task_.LatestVersion(), PRODUCT_VERSION_STRING) != 0) {
      SerializableString16* update_event =
          new SerializableString16(task_.Url());
      MessageService::GetInstance()->NotifyObservers(kTopic,
                                                     update_event);
    }
  } else {
    // Failed attempt.
    // If it appears that we don't have a connection,
    // wait for one to appear. Otherwise, the error must have
    // been on the network, so we'll try again tomorrow.
    if (!ActiveXUtils::IsOnline()) {
      ConnMgrRegisterForStatusChangeNotification(TRUE, window_);
    }
  }
  task_.SetListenerWindow(NULL, 0);
  handled = TRUE;
  return TRUE;
}

void PeriodicChecker::OnConnectionStatusChanged() {
  // Unregister from connection events. We'll register again
  // if the update check fails.
  if (ActiveXUtils::IsOnline()) {
    ConnMgrRegisterForStatusChangeNotification(FALSE, window_);
    // Schedule a check after the grace period.
    normal_timer_event_ = false;
    ResetTimer(grace_period_);
  }
}

void PeriodicChecker::ResetTimer(int32 period) {
  KillTimer(timer_);
  SetTimer(static_cast<unsigned>(timer_),
           static_cast<unsigned>(period),
           NULL);
}
