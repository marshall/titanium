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

// This file contains the Chrome plugin entry point and hooks.
#include "gears/base/chrome/module_cr.h"

#include "gears/base/chrome/browsing_context_cr.h"
#include "gears/base/common/async_router.h"
#include "gears/base/common/base_class.h"
#include "gears/base/common/detect_version_collision.h"
#include "gears/base/common/paths.h"
#include "gears/base/common/string_utils.h"
#include "gears/base/npapi/browser_utils.h"
#include "gears/base/npapi/module.h"
#include "gears/desktop/desktop_cr.h"
#include "gears/localserver/common/localserver_db.h"
#include "gears/localserver/chrome/http_request_cr.h"
#include "gears/localserver/chrome/gears_protocol_handler.h"
#include "gears/localserver/chrome/network_intercept_cr.h"
#include "gears/localserver/chrome/update_task_cr.h"
#include "third_party/chrome/chrome_plugin_api.h"
#include "third_party/chrome/gears_api.h"
#include "gears/ui/common/settings_dialog.h"

bool g_cpwas_initialized = false;
bool g_cpinitialization_failed = false;
ThreadId g_cpthread_id;
CPID g_cpid;
CPProcessType g_cpprocess_type;
CPBrowserFuncs g_cpbrowser_funcs;
CPRequestFuncs g_cprequest_funcs;
CPResponseFuncs g_cpresponse_funcs;
std::string16 g_plugin_data_dir;

std::string16 CP::g_locale;
int CP::g_process_keep_alive_count = 0;
bool CP::g_gears_in_renderer = false;

// When this is true, we are in "offline" mode, and any requests issued by
// Chrome that are we don't handle are treated as errors.  This gives us a nice
// way to test offline-enabled apps.
//
// How to toggle it: visit "gears://debug/offline" or "gears://debug/online" in
// Chrome.
//
// How it works:
// Simple!  While we're offline, we claim to be able to service everything
// (CPP_ShouldInterceptRequest always returns true).  When Chrome actually asks
// us to intercept, we can either serve it or not.  If we fail, Chrome will show
// an error for us.
bool CP::g_offline_debug_mode = false;


// TODO(mpcomplete): move to SerializableClassId enumeration
#define SERIALIZABLE_OFFLINE_MODE_MESSAGE  ((SerializableClassId)1225)

// The message sent from browser process to plugin process for toggling the
// "offline" mode.
class OfflineModeMessage : public PluginMessage {
 public:
  OfflineModeMessage() : offline_(false) {}
  OfflineModeMessage(bool offline) : offline_(offline) {}

  bool offline_;

  // PluginMessage override
  virtual void OnMessageReceived() {
    CP::set_offline_debug_mode(offline_);
  }

  // Serailizable overrides
  virtual SerializableClassId GetSerializableClassId() const {
    return SERIALIZABLE_OFFLINE_MODE_MESSAGE;
  }
  virtual bool Serialize(Serializer *out) const {
    out->WriteBool(offline_);
    return true;
  }
  virtual bool Deserialize(Deserializer *in) {
    return in->ReadBool(&offline_);
  }

  // Serializable registration
  static Serializable *New() {
    return new OfflineModeMessage;
  }
  static void Register() {
    Serializable::RegisterClass(SERIALIZABLE_OFFLINE_MODE_MESSAGE, New);
  }  
};

// TODO(michaeln): move to SerializableClassId enumeration
#define SERIALIZABLE_SHORTCUTS_CHANGED_MESSAGE  ((SerializableClassId)1226)

// The message sent from plugin process to browser process which is
// where we have to inform chrome of these changes.
class ShortcutsChangedMessage : public PluginMessage {
 public:
  // PluginMessage override
  virtual void OnMessageReceived() {
    assert(CP::IsBrowserProcess());
    assert(CP::IsPluginThread());
    CPBrowsingContext ctx = 0;  // TODO(michaeln): support multiple profiles
    CP::browser_funcs().handle_command(
        CP::cpid(), ctx, GEARSBROWSERCOMMAND_NOTIFY_SHORTCUTS_CHANGED, NULL);
  }

  // Serailizable overrides
  virtual SerializableClassId GetSerializableClassId() const {
    return SERIALIZABLE_SHORTCUTS_CHANGED_MESSAGE;
  }
  virtual bool Serialize(Serializer *out) const {
    return true;
  }
  virtual bool Deserialize(Deserializer *in) {
    return true;
  }

  // Serializable registration
  static Serializable *New() {
    return new ShortcutsChangedMessage;
  }
  static void Register() {
    Serializable::RegisterClass(SERIALIZABLE_SHORTCUTS_CHANGED_MESSAGE, New);
  }  
};


// Gears MessageService observer that will be notified when changes to the
// shortcuts database are made.
class ShortcutsChangedObserver : public MessageObserverInterface {
  virtual void OnNotify(MessageService *service,
                        const char16 *topic,
                        const NotificationData *data) {
    assert(CP::IsPluginThread());
    CPBrowsingContext ctx = 0;  // TODO(michaeln): support multiple profiles
    if (CP::IsBrowserProcess()) {
      CP::browser_funcs().handle_command(
          CP::cpid(), ctx, GEARSBROWSERCOMMAND_NOTIFY_SHORTCUTS_CHANGED, NULL);
    } else {
      ShortcutsChangedMessage plugin_message;
      plugin_message.Send();
    }
  }  
};
static ShortcutsChangedObserver g_shortcuts_observer;


bool IsPluginThread() {
  return CP::IsPluginThread();
}

// Plugin hooks called by the browser
namespace {

CPError STDCALL CPP_Shutdown() {
  return CPERR_SUCCESS;
}

CPBool STDCALL CPP_ShouldInterceptRequest(CPRequest *request) {
  assert(request && !request->pdata);

  std::string16 url;
  if (!UTF8ToString16(request->url, &url))
    return false;

  scoped_refptr<CRBrowsingContext> context(
      new CRBrowsingContext(request->context));

  NetworkIntercept *intercept = NetworkIntercept::Intercept(request, url,
                                                            context.get());

  if (!intercept) {
    return CP::offline_debug_mode();
  }

  LOG(("Intercepting(%s %s)\n", request->method, request->url));
  request->pdata = intercept;
  return true;
}

void STDCALL CPP_OnMessage(void *data, uint32 data_len) {
  Deserializer deserializer(static_cast<uint8*>(data), data_len);
  PluginMessage *message = NULL;
  deserializer.CreateAndReadObject(reinterpret_cast<Serializable**>(&message));
  if (message) {
    message->OnMessageReceived();
  }
}

void STDCALL CPP_HtmlDialogClosed(void *plugin_context,
                                  const char *json_retval) {
  assert(plugin_context);
  HtmlDialogCallback *callback(
      static_cast<HtmlDialogCallback*>(plugin_context));

  std::string16 json_retval16;
  UTF8ToString16(json_retval, &json_retval16);

  // Callback is expected to delete itself when done.
  callback->DialogClosed(json_retval16.c_str());
}

CPError STDCALL CPP_HandleCommand(
      CPBrowsingContext context, int command, void* command_data) {
  if (command == GEARSPLUGINCOMMAND_SHOW_SETTINGS) {
    if (!SettingsDialog::IsVisible()) {
      SettingsDialog::Run(new CRBrowsingContext(context));
      return CPERR_SUCCESS;
    }
  } else if (command == GEARSPLUGINCOMMAND_CREATE_SHORTCUT) {
    GearsShortcutData2 *shortcut_data =
        static_cast<GearsShortcutData2*>(command_data);
    scoped_ptr<ModelessShortcutsDialog> dialog(
        new ModelessShortcutsDialog(context));
    if (dialog->ShowDialog(shortcut_data)) {
      dialog.release();  // Passed to CPAPI as plugin_context.
      // Tell Chrome to expect a callback via command
      // GEARSBROWSERCOMMAND_CREATE_SHORTCUT_DONE.
      return CPERR_IO_PENDING;
    }
  } else if (command == GEARSPLUGINCOMMAND_GET_SHORTCUT_LIST) {
    GearsShortcutList *shortcut_list =
        static_cast<GearsShortcutList*>(command_data);
    GetAllShortcuts(shortcut_list);
    return CPERR_SUCCESS;
  }
  return CPERR_FAILURE;
}

CPError STDCALL CPR_StartRequest(CPRequest *request) {
  LOG(("CPR_StartRequest(%s %s)\n", request->method, request->url));
  // CPP_ShouldInterceptRequest has done all of the work for us already
  NetworkIntercept *intercept = NetworkIntercept::FromCPRequest(request);
  if (!intercept)
    return CPERR_FAILURE;
  return CPERR_SUCCESS;
}

void STDCALL CPR_EndRequest(CPRequest *request, CPError reason) {
  LOG(("CPR_EndRequest(%s %s)\n", request->method, request->url));
  NetworkIntercept *intercept = NetworkIntercept::FromCPRequest(request);
  delete intercept;
  request->pdata = NULL;
}

void STDCALL CPR_SetExtraRequestHeaders(CPRequest* request,
                                        const char* headers) {
  // doesn't affect us
}

void STDCALL CPR_SetRequestLoadFlags(CPRequest* request, uint32 flags) {
  // doesn't affect us
}

void STDCALL CPR_AppendDataToUpload(CPRequest* request, const char* bytes,
                                    int bytes_len) {
  // doesn't affect us
}

CPError STDCALL CPR_AppendFileToUpload(
    CPRequest* request, const char* filepath, uint64 offset, uint64 length) {
  // doesn't affect us
  return CPERR_FAILURE;
}

int STDCALL CPR_GetResponseInfo(CPRequest* request, CPResponseInfoType type,
                                void* buf, uint32 buf_size) {
  NetworkIntercept *intercept = NetworkIntercept::FromCPRequest(request);
  return intercept->GetResponseInfo(type, buf, buf_size);
}

int STDCALL CPR_Read(CPRequest* request, void* buf, uint32 buf_size) {
  NetworkIntercept *intercept = NetworkIntercept::FromCPRequest(request);
  return intercept->Read(buf, buf_size);
}

void STDCALL CPRR_ReceivedRedirect(CPRequest* cprequest, const char* new_url) {
  CRHttpRequest *request = CRHttpRequest::FromCPRequest(cprequest);
  if (!request) return;  // defend against the host calling us when it shouldn't
  std::string16 new_url16;
  if (!UTF8ToString16(new_url, &new_url16))
    return;
  request->OnReceivedRedirect(new_url16);
}

void STDCALL CPRR_StartCompleted(CPRequest* cprequest, CPError result) {
  CRHttpRequest *request = CRHttpRequest::FromCPRequest(cprequest);
  if (!request) return;  // defend against the host calling us when it shouldn't
  request->OnStartCompleted(result);
}

void STDCALL CPRR_ReadCompleted(CPRequest* cprequest, int bytes_read) {
  CRHttpRequest *request = CRHttpRequest::FromCPRequest(cprequest);
  if (!request) return;  // defend against the host calling us when it shouldn't
  request->OnReadCompleted(bytes_read);
}

void STDCALL CPRR_UploadProgress(CPRequest* cprequest,
                                 uint64 position, uint64 size) {
  CRHttpRequest *request = CRHttpRequest::FromCPRequest(cprequest);
  if (!request) return;  // defend against the host calling us when it shouldn't
  request->OnUploadProgress(static_cast<int64>(position),
                            static_cast<int64>(size));
}

}  // anon namespace

CPError STDCALL CP_Initialize(CPID id, const CPBrowserFuncs *bfuncs,
                              CPPluginFuncs *pfuncs) {
  return CP::Initialize(id, bfuncs, pfuncs);
}

CPError CP::Initialize(CPID id, const CPBrowserFuncs *bfuncs,
                       CPPluginFuncs *pfuncs) {
  assert(!g_cpwas_initialized);

  // will be reset upon successful initialization
  g_cpinitialization_failed = true;
  AllowNPInit(false);

  if (DetectedVersionCollision()) {
    NotifyUserOfVersionCollision();
    return CPERR_FAILURE;
  }

  if (bfuncs == NULL || pfuncs == NULL)
    return CPERR_FAILURE;

  if (CP_GET_MAJOR_VERSION(bfuncs->version) != CP_MAJOR_VERSION)
    return CPERR_INVALID_VERSION;

  ThreadMessageQueue::GetInstance()->InitThreadMessageQueue();
  g_cpthread_id = ThreadMessageQueue::GetInstance()->GetCurrentThreadId();

  CPPluginFuncs plugin_funcs;
  // We report the browser's version, since we're compatible with minor version
  // changes.
  plugin_funcs.version = bfuncs->version;
  plugin_funcs.size = sizeof(plugin_funcs);
  plugin_funcs.shutdown = CPP_Shutdown;
  plugin_funcs.should_intercept_request = CPP_ShouldInterceptRequest;
  plugin_funcs.on_message = CPP_OnMessage;
  plugin_funcs.html_dialog_closed = CPP_HtmlDialogClosed;
  plugin_funcs.handle_command = CPP_HandleCommand;
  size_t pfuncs_size = pfuncs->size;
  memset(pfuncs, 0, pfuncs->size);
  memcpy(pfuncs, &plugin_funcs,
         std::min<size_t>(sizeof(plugin_funcs), pfuncs_size)); 

  static CPRequestFuncs request_funcs;
  memset(&request_funcs, 0, sizeof(request_funcs));
  request_funcs.start_request = CPR_StartRequest;
  request_funcs.end_request = CPR_EndRequest;
  request_funcs.set_extra_request_headers = CPR_SetExtraRequestHeaders;
  request_funcs.set_request_load_flags = CPR_SetRequestLoadFlags;
  request_funcs.append_data_to_upload = CPR_AppendDataToUpload;
  request_funcs.get_response_info = CPR_GetResponseInfo;
  request_funcs.read = CPR_Read;
  request_funcs.append_file_to_upload = CPR_AppendFileToUpload;
  pfuncs->request_funcs = &request_funcs;

  static CPResponseFuncs response_funcs;
  memset(&response_funcs, 0, sizeof(response_funcs));
  response_funcs.received_redirect = CPRR_ReceivedRedirect;
  response_funcs.start_completed = CPRR_StartCompleted;
  response_funcs.read_completed = CPRR_ReadCompleted;
  response_funcs.upload_progress = CPRR_UploadProgress;
  pfuncs->response_funcs = &response_funcs;

  g_cpid = id;
  memcpy(&g_cpbrowser_funcs, bfuncs,
         std::min<size_t>(sizeof(g_cpbrowser_funcs), bfuncs->size)); 
  memcpy(&g_cprequest_funcs, bfuncs->request_funcs,
         std::min<size_t>(sizeof(g_cprequest_funcs),
                          bfuncs->request_funcs->size)); 
  memcpy(&g_cpresponse_funcs, bfuncs->response_funcs,
         std::min<size_t>(sizeof(g_cpresponse_funcs),
                          bfuncs->response_funcs->size)); 

  const char *handled_schemes[] = {"http", "https", kGearsScheme};
  g_cpbrowser_funcs.enable_request_intercept(
      g_cpid, handled_schemes, ARRAYSIZE(handled_schemes));
  g_cpprocess_type = g_cpbrowser_funcs.get_process_type(g_cpid);

  // Get the base plugin data dir.
  char *data_dir = NULL;
  if (g_cpbrowser_funcs.get_browsing_context_info(
          g_cpid, NULL, CPBROWSINGCONTEXT_DATA_DIR_PTR,
          &data_dir, sizeof(data_dir)) == CPERR_SUCCESS) {
    UTF8ToString16(data_dir, &g_plugin_data_dir);
    g_cpbrowser_funcs.free(data_dir);
  }

  // Get the user's locale.
  char *locale = NULL;
  if (g_cpbrowser_funcs.get_browsing_context_info(
          g_cpid, NULL, CPBROWSINGCONTEXT_UI_LOCALE_PTR,
          &locale, sizeof(locale)) == CPERR_SUCCESS) {
    UTF8ToString16(locale, &g_locale);
    g_cpbrowser_funcs.free(locale);
  }

  if (g_cpbrowser_funcs.add_ui_command)
    g_cpbrowser_funcs.add_ui_command(g_cpid, GEARSPLUGINCOMMAND_SHOW_SETTINGS);

  const char16 *cmdline = GetCommandLineW();
  if (wcsstr(cmdline, STRING16(L"--gears-in-renderer")) != NULL)
    g_gears_in_renderer = true;

  g_cpwas_initialized = true;
  g_cpinitialization_failed = false;

  // TODO(michaeln): remove test for NULL when the CPAPI support in chrome
  // can be counted on (introduced in 0.3)
  if (g_cpbrowser_funcs.handle_command) {
    MessageService::GetInstance()->AddObserver(
        &g_shortcuts_observer, PermissionsDB::kShortcutsChangedTopic);
  }

  AllowNPInit(true);
  AutoUpdateMessage::Register();
  OfflineModeMessage::Register();
  ShortcutsChangedMessage::Register();
  return CPERR_SUCCESS;
}

class PluginMessageOnPluginThread : public AsyncFunctor {
 public:
  PluginMessageOnPluginThread(std::vector<uint8>& message) {
    message_.swap(message);
  }
  virtual void Run() {
    CP::browser_funcs().send_message(g_cpid, &message_[0], message_.size());
  }

 private:
  std::vector<uint8> message_;
};

bool PluginMessage::Send() {
  std::vector<uint8> buf;
  Serializer serializer(&buf);
  if (!serializer.WriteObject(this))
    return false;
  
  if (!IsPluginThread()) {
    AsyncRouter::GetInstance()->CallAsync(
        CP::plugin_thread_id(), new PluginMessageOnPluginThread(buf));
    return true;
  } else {
    return CPERR_SUCCESS == CP::browser_funcs().send_message(g_cpid, &buf[0],
                                                             buf.size());
  }
}

CPBrowsingContext CP::GetBrowsingContext(JsContextPtr context) {
  // TODO(mpcomplete): This only works for the plugin process.  We need to
  // support something like this in the browser process as well.
  assert(IsPluginProcess());
  assert(IsPluginThread());

  return g_cpbrowser_funcs.get_browsing_context_from_npp(context);
}

char *CP::StringDup(const std::string &str) {
  char *cpstr = CP::Alloc<char>(str.length() + 1);
  memcpy(cpstr, str.c_str(), str.length() + 1);
  return cpstr;
}

char *CP::String16ToUTF8Dup(const std::string16 &str) {
  std::string str_utf8;
  if (!String16ToUTF8(str.data(), str.length(), &str_utf8))
    return NULL;
  return CP::StringDup(str_utf8);
}

void CP::set_offline_debug_mode(bool offline) {
  g_offline_debug_mode = offline;
  if (CP::IsBrowserProcess()) {
    // Let the plugin process know about it.
    OfflineModeMessage(offline).Send();
  }
}


class CP::AdjustProcessKeepAliveOnPluginThread : public AsyncFunctor {
 public:
  virtual void Run() {
    AdjustProcessKeepAlive();
  }
};

// Adjusts the 'keepAlive' property of chrome's plugin process
void CP::AdjustProcessKeepAlive() {
  if (IsPluginProcess() && !gears_in_renderer()) {
    if (!IsPluginThread()) {
      AsyncRouter::GetInstance()->CallAsync(
          plugin_thread_id(), new AdjustProcessKeepAliveOnPluginThread());
    } else {
      int count = AtomicIncrement(&g_process_keep_alive_count, 0);
      browser_funcs().set_keep_process_alive(g_cpid, count != 0);
    }
  }
}

void CP::IncrementProcessKeepAliveCount() {
  int count = AtomicIncrement(&g_process_keep_alive_count, 1);
  if (count == 1) {
    AdjustProcessKeepAlive();
  }
}

void CP::DecrementProcessKeepAliveCount() {
  int count = AtomicIncrement(&g_process_keep_alive_count, -1);
  assert(count >= 0);
  if (count == 0) {
    AdjustProcessKeepAlive();
  }
}
