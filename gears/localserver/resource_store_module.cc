// Copyright 2005, Google Inc.
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

#include "gears/localserver/resource_store_module.h"

#if BROWSER_IE
#include <windows.h>
#endif

#include "gears/base/common/js_dom_element.h"
#include "gears/base/common/mime_detect.h"
#include "gears/base/common/module_wrapper.h"
#include "gears/base/common/url_utils.h"
#include "gears/blob/blob.h"
#include "gears/blob/file_blob.h"
#include "gears/localserver/file_submitter.h"


//-----------------------------------------------------------------------------
// GearsResourceStoreMessageHwnd
//-----------------------------------------------------------------------------
#if BROWSER_IE
class GearsResourceStoreMessageHwnd
    : public CWindowImpl<GearsResourceStoreMessageHwnd> {
 public:
  static const int kCaptureTaskMessageBase = WM_USER;
  static const int
      WM_CAPTURE_TASK_COMPLETE = CaptureTask::CAPTURE_TASK_COMPLETE
                                 + kCaptureTaskMessageBase;
  static const int
      WM_CAPTURE_URL_SUCCEEDED = CaptureTask::CAPTURE_URL_SUCCEEDED
                                 + kCaptureTaskMessageBase;
  static const int
      WM_CAPTURE_URL_FAILED = CaptureTask::CAPTURE_URL_FAILED
                              + kCaptureTaskMessageBase;

  BEGIN_MSG_MAP(GearsResourceStoreMessageHwnd)
    MESSAGE_HANDLER(WM_CAPTURE_TASK_COMPLETE, OnCaptureTaskComplete)
    MESSAGE_HANDLER(WM_CAPTURE_URL_SUCCEEDED, OnCaptureUrlComplete)
    MESSAGE_HANDLER(WM_CAPTURE_URL_FAILED, OnCaptureUrlComplete)
  END_MSG_MAP()

  GearsResourceStoreMessageHwnd(GearsResourceStore *resource_store)
      : resource_store_(resource_store) {}

  void Initialize() {
    // Make sure we have an HWND
    if (!IsWindow()) {
      if (!Create(kMessageOnlyWindowParent,    // parent
                  NULL,                        // position
                  NULL,                        // name
                  kMessageOnlyWindowStyle)) {  // style
        assert(false);
      }
    }
  }

  LRESULT OnCaptureTaskComplete(UINT uMsg,
                                WPARAM wParam,
                                LPARAM lParam,
                                BOOL& bHandled) {
    CaptureTask* task = reinterpret_cast<CaptureTask*>(lParam);
    if (task && (task == resource_store_->capture_task_.get())) {
      resource_store_->OnCaptureTaskComplete();
    }
    bHandled = TRUE;
    return 0;
  }

  LRESULT OnCaptureUrlComplete(UINT uMsg,
                               WPARAM wParam,
                               LPARAM lParam,
                               BOOL& bHandled) {
    CaptureTask* task = reinterpret_cast<CaptureTask*>(lParam);
    if (task && (task == resource_store_->capture_task_.get())) {
      int index = wParam;
      bool success = (uMsg == WM_CAPTURE_URL_SUCCEEDED);
      resource_store_->OnCaptureUrlComplete(index, success);
    }
    bHandled = TRUE;
    return 0;
  }

  void OnFinalMessage(HWND hwnd) {
    delete this;
  }

 private:
  GearsResourceStore *resource_store_;
  DISALLOW_EVIL_CONSTRUCTORS(GearsResourceStoreMessageHwnd);
};
#endif


//------------------------------------------------------------------------------
// GearsResourceStore
//------------------------------------------------------------------------------
DECLARE_GEARS_WRAPPER(GearsResourceStore);

// static
template<>
void Dispatcher<GearsResourceStore>::Init() {
  RegisterProperty("name", &GearsResourceStore::GetName, NULL);
  RegisterProperty("requiredCookie", &GearsResourceStore::GetRequiredCookie,
                   NULL);
  RegisterProperty("enabled", &GearsResourceStore::GetEnabled,
                   &GearsResourceStore::SetEnabled);

  RegisterMethod("capture", &GearsResourceStore::Capture);
  RegisterMethod("abortCapture", &GearsResourceStore::AbortCapture);
  RegisterMethod("isCaptured", &GearsResourceStore::IsCaptured);
  RegisterMethod("remove", &GearsResourceStore::Remove);
  RegisterMethod("rename", &GearsResourceStore::Rename);
  RegisterMethod("copy", &GearsResourceStore::Copy);
  RegisterMethod("getHeader", &GearsResourceStore::GetHeader);
  RegisterMethod("getAllHeaders", &GearsResourceStore::GetAllHeaders);
  RegisterMethod("captureBlob", &GearsResourceStore::CaptureBlob);
  RegisterMethod("captureFile", &GearsResourceStore::CaptureFile);
  RegisterMethod("getCapturedFileName",
                 &GearsResourceStore::GetCapturedFileName);
  RegisterMethod("createFileSubmitter",
                 &GearsResourceStore::CreateFileSubmitter);
}

const std::string GearsResourceStore::kModuleName("GearsResourceStore");

GearsResourceStore::GearsResourceStore()
    : ModuleImplBaseClass(kModuleName),
#if BROWSER_IE
      message_hwnd_(new GearsResourceStoreMessageHwnd(this)),
#endif
      next_capture_id_(0), page_is_unloaded_(false) {}

GearsResourceStore::~GearsResourceStore() {
  AbortAllRequests();
#if BROWSER_IE
  if (message_hwnd_->IsWindow()) {
    message_hwnd_->DestroyWindow();
  } else {
    delete message_hwnd_;
  }
#endif
}

//------------------------------------------------------------------------------
// GetName
//------------------------------------------------------------------------------
void GearsResourceStore::GetName(JsCallContext *context) {
  std::string16 name(store_.GetName());
  context->SetReturnValue(JSPARAM_STRING16, &name);
}

//------------------------------------------------------------------------------
// GetRequiredCookie
//------------------------------------------------------------------------------
void GearsResourceStore::GetRequiredCookie(JsCallContext *context) {
  std::string16 cookie(store_.GetRequiredCookie());
  context->SetReturnValue(JSPARAM_STRING16, &cookie);
}

//------------------------------------------------------------------------------
// GetEnabled
//------------------------------------------------------------------------------
void GearsResourceStore::GetEnabled(JsCallContext *context) {
  bool enabled = store_.IsEnabled();
  context->SetReturnValue(JSPARAM_BOOL, &enabled);
}

//------------------------------------------------------------------------------
// SetEnabled
//------------------------------------------------------------------------------
void GearsResourceStore::SetEnabled(JsCallContext *context) {
  bool enabled;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_BOOL, &enabled },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set())
    return;

  if (!store_.SetEnabled(enabled)) {
    context->SetException(STRING16(L"Failed to set the enabled property."));
    return;
  }
}

//------------------------------------------------------------------------------
// Capture
//------------------------------------------------------------------------------
void GearsResourceStore::Capture(JsCallContext *context) {
  std::string16 url;
  JsArray url_array;
  JsRootedCallback *callback = NULL;

  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_UNKNOWN, NULL },
    { JSPARAM_OPTIONAL, JSPARAM_FUNCTION, &callback },
  };

  // TODO(aa): Consider coercing anything except array to string. This would
  // make it consistent with XMLHttpRequest and window.setTimeout.
  int url_arg_type = context->GetArgumentType(0);
  if (url_arg_type == JSPARAM_ARRAY) {
    argv[0].type = JSPARAM_ARRAY;
    argv[0].value_ptr = &url_array;
  } else if (url_arg_type == JSPARAM_STRING16) {
    argv[0].type = JSPARAM_STRING16;
    argv[0].value_ptr = &url;
  } else {
    context->SetException(
      STRING16(L"First parameter must be an array or string."));
    return;
  }

  context->GetArguments(ARRAYSIZE(argv), argv);
  scoped_ptr<JsRootedCallback> scoped_callback(callback);
  if (context->is_exception_set())
    return;

  int capture_id = ++next_capture_id_;
  LOG(("ResourceStore::capture - id = %d\n", capture_id));

  scoped_ptr<CaptureRequest> request(new CaptureRequest);
  request->id = capture_id;
  request->callback.swap(scoped_callback);  // transfer ownership

  if (url_arg_type == JSPARAM_ARRAY) {
    // 'urls' was an array of strings
    int array_length;
    if (!url_array.GetLength(&array_length)) {
      context->SetException(GET_INTERNAL_ERROR_MESSAGE());
      return;
    }

    for (int i = 0; i < array_length; ++i) {
      if (!url_array.GetElementAsString(i, &url)) {
        context->SetException(STRING16(L"Invalid parameter."));
        return;
      }

      if (!ResolveAndAppendUrl(url.c_str(), request.get())) {
        context->SetException(exception_message_.c_str());
        return;
      }
    }
  } else {
    // 'urls' was a string
    if (!ResolveAndAppendUrl(url.c_str(), request.get())) {
      context->SetException(exception_message_.c_str());
      return;
    }
  }

  pending_requests_.push_back(request.release());

  if (!StartCaptureTaskIfNeeded(false)) {
    context->SetException(exception_message_);
    return;
  }

  context->SetReturnValue(JSPARAM_INT, &capture_id);
}

//------------------------------------------------------------------------------
// AbortCapture
//------------------------------------------------------------------------------
void GearsResourceStore::AbortCapture(JsCallContext *context) {
  int capture_id;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_INT, &capture_id },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set())
    return;

  if (current_request_.get() && (current_request_->id == capture_id)) {
    // The caller is aborting the task that we're running
    assert(capture_task_.get());
    if (capture_task_.get()) {
      capture_task_->Abort();
    }
    return;
  }

  // Search for capture_id in our pending queue
  std::deque<CaptureRequest*>::iterator iter;
  for (iter = pending_requests_.begin();
       iter < pending_requests_.end();
       iter++) {
    if ((*iter)->id == capture_id) {
      // Remove it from the queue and fire completion events
      CaptureRequest *request = (*iter);
      pending_requests_.erase(iter);
      FireFailedEvents(request);
      delete request;
      return;
      // Note: the deque.erase() call is safe here since we return and
      // do not continue the iteration
    }
  }
}

//------------------------------------------------------------------------------
// IsCaptured
//------------------------------------------------------------------------------
void GearsResourceStore::IsCaptured(JsCallContext *context) {
  std::string16 url;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &url },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set())
    return;

  std::string16 full_url;
  if (!ResolveUrl(url, &full_url)) {
    context->SetException(exception_message_.c_str());
    return;
  }
  bool is_captured = store_.IsCaptured(full_url.c_str());
  context->SetReturnValue(JSPARAM_BOOL, &is_captured);
}

//------------------------------------------------------------------------------
// Remove
//------------------------------------------------------------------------------
void GearsResourceStore::Remove(JsCallContext *context) {
  std::string16 url;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &url },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set())
    return;

  std::string16 full_url;
  if (!ResolveUrl(url, &full_url)) {
    context->SetException(exception_message_.c_str());
    return;
  }

  if (!store_.Delete(full_url.c_str())) {
    context->SetException(STRING16(L"Failure removing url."));
    return;
  }
}

//------------------------------------------------------------------------------
// Rename
//------------------------------------------------------------------------------
void GearsResourceStore::Rename(JsCallContext *context) {
  std::string16 src_url;
  std::string16 dest_url;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &src_url },
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &dest_url },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set())
    return;

  std::string16 full_src_url;
  if (!ResolveUrl(src_url, &full_src_url)) {
    context->SetException(exception_message_.c_str());
    return;
  }

  std::string16 full_dest_url;
  if (!ResolveUrl(dest_url, &full_dest_url)) {
    context->SetException(exception_message_.c_str());
    return;
  }

  if (!store_.Rename(full_src_url.c_str(), full_dest_url.c_str())) {
    context->SetException(STRING16(L"Failure renaming url."));
    return;
  }
}

//------------------------------------------------------------------------------
// Copy
//------------------------------------------------------------------------------
void GearsResourceStore::Copy(JsCallContext *context) {
  std::string16 src_url;
  std::string16 dest_url;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &src_url },
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &dest_url },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set())
    return;

  std::string16 full_src_url;
  if (!ResolveUrl(src_url, &full_src_url)) {
    context->SetException(exception_message_.c_str());
    return;
  }

  std::string16 full_dest_url;
  if (!ResolveUrl(dest_url, &full_dest_url)) {
    context->SetException(exception_message_.c_str());
    return;
  }

  if (!store_.Copy(full_src_url.c_str(), full_dest_url.c_str())) {
    context->SetException(STRING16(L"Failure copying url."));
    return;
  }
}

//------------------------------------------------------------------------------
// CaptureBlob
//------------------------------------------------------------------------------
void GearsResourceStore::CaptureBlob(JsCallContext *context) {
#ifdef WINCE
  context->SetException(STRING16(L"captureBlob is not implemented."));
#else

  ModuleImplBaseClass *other_module = NULL;
  std::string16 url;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_MODULE, &other_module },
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &url }
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set()) {
    return;
  }
  std::string16 full_url;
  if (!ResolveUrl(url.c_str(), &full_url)) {
    context->SetException(STRING16(L"Failed to resolve url."));
    return;
  }

  if (GearsBlob::kModuleName != other_module->get_module_name()) {
    context->SetException(STRING16(L"First argument must be a Blob."));
    return;
  }
  scoped_refptr<BlobInterface> blob;
  static_cast<GearsBlob*>(other_module)->GetContents(&blob);
  assert(blob.get());

  ResourceStore::Item item;
  if (!ResourceStore::BlobToItem(blob.get(), full_url.c_str(),
                                 NULL, NULL, &item) ||
      !store_.PutItem(&item)) {
    context->SetException(STRING16(L"The blob could not be captured."));
    return;
  }
#endif
}

//------------------------------------------------------------------------------
// CaptureFile
//------------------------------------------------------------------------------
void GearsResourceStore::CaptureFile(JsCallContext *context) {
#if BROWSER_NPAPI
  // TODO(nigeltao): implement on NPAPI. To do this, I need to figure out how
  // a FileInputElement is represented.
  context->SetException(STRING16(L"captureFile is not implemented."));
#else
  if (EnvIsWorker()) {
    context->SetException(
        STRING16(L"captureFile is not supported in workers."));
    return;
  }

  JsDomElement dom_element;
  std::string16 url;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_DOM_ELEMENT, &dom_element },
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &url }
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set()) {
    return;
  }
  std::string16 full_url;
  if (!ResolveUrl(url.c_str(), &full_url)) {
    context->SetException(STRING16(L"Failed to resolve url."));
    return;
  }

  std::string16 file_name;
  if (!dom_element.GetFileInputElementValue(&file_name)) {
    context->SetException(
        STRING16(L"Failed to get the file name from the file input element."));
    return;
  }
  if (file_name.empty()) {
    context->SetException(STRING16(L"File path is empty."));
    return;
  }
  if (!File::Exists(file_name.c_str())) {
    context->SetException(STRING16(L"File does not exist."));
    return;
  }
  std::string16 file_base_name;
  if (!File::GetBaseName(file_name, &file_base_name)) {
    context->SetException(STRING16(L"Could not extract the file's base name."));
    return;
  }
  std::string16 mime_type = DetectMimeTypeOfFile(file_name);
  scoped_refptr<BlobInterface> blob(new FileBlob(file_name));

  ResourceStore::Item item;
  if (!ResourceStore::BlobToItem(blob.get(), full_url.c_str(),
                                 mime_type.c_str(), file_base_name.c_str(),
                                 &item) ||
      !store_.PutItem(&item)) {
    context->SetException(STRING16(L"The file could not be captured."));
    return;
  }
#endif
}

//------------------------------------------------------------------------------
// GetCapturedFileName
//------------------------------------------------------------------------------
void GearsResourceStore::GetCapturedFileName(JsCallContext *context) {
  std::string16 url;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &url },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set())
    return;

  std::string16 full_url;
  if (!ResolveUrl(url, &full_url)) {
    context->SetException(exception_message_.c_str());
    return;
  }

  std::string16 file_name;
  if (!store_.GetCapturedFileName(full_url.c_str(), &file_name)) {
    context->SetException(STRING16(L"GetCapturedFileName failed."));
    return;
  }

  context->SetReturnValue(JSPARAM_STRING16, &file_name);
}

//------------------------------------------------------------------------------
// GetHeader
//------------------------------------------------------------------------------
void GearsResourceStore::GetHeader(JsCallContext *context) {
  std::string16 url;
  std::string16 name;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &url },
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &name },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set())
    return;

  std::string16 full_url;
  if (!ResolveUrl(url, &full_url)) {
    context->SetException(exception_message_.c_str());
    return;
  }

  std::string16 value;
  store_.GetHeader(full_url.c_str(), name.c_str(), &value);
  context->SetReturnValue(JSPARAM_STRING16, &value);
}

//------------------------------------------------------------------------------
// GetAllHeaders
//------------------------------------------------------------------------------
void GearsResourceStore::GetAllHeaders(JsCallContext *context) {
  std::string16 url;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &url },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set())
    return;

  std::string16 full_url;
  if (!ResolveUrl(url, &full_url)) {
    context->SetException(exception_message_.c_str());
    return;
  }

  std::string16 all_headers;
  if (!store_.GetAllHeaders(full_url.c_str(), &all_headers)) {
    context->SetException(STRING16(L"GetAllHeaders failed."));
    return;
  }

  context->SetReturnValue(JSPARAM_STRING16, &all_headers);
}

//------------------------------------------------------------------------------
// CreateFileSubmitter
//------------------------------------------------------------------------------
void GearsResourceStore::CreateFileSubmitter(JsCallContext *context) {
#ifdef WINCE
  context->SetException(STRING16(L"createFileSubmitter is not implemented."));
  return;
#elif BROWSER_NPAPI
  // TODO(nigeltao): implement on NPAPI.
  context->SetException(STRING16(L"createFileSubmitter is not implemented."));
  return;
#else
  if (EnvIsWorker()) {
    context->SetException(
        STRING16(L"createFileSubmitter cannot be called in a worker."));
    return;
  }

  scoped_refptr<GearsFileSubmitter> submitter;
  if (!CreateModule<GearsFileSubmitter>(module_environment_.get(),
                                        context, &submitter)) {
    return;
  }
  if (!submitter->store_.Clone(&store_)) {
    context->SetException(STRING16(L"Error initializing base class."));
    return;
  }

  context->SetReturnValue(JSPARAM_MODULE, submitter.get());
#endif
}

// End Javascript API

//------------------------------------------------------------------------------
// HandleEvent
//------------------------------------------------------------------------------
void GearsResourceStore::HandleEvent(JsEventType event_type) {
  assert(event_type == JSEVENT_UNLOAD);

  page_is_unloaded_ = true;
  AbortAllRequests();
}


//------------------------------------------------------------------------------
// AbortAllRequests
//------------------------------------------------------------------------------
void GearsResourceStore::AbortAllRequests() {
  if (capture_task_.get()) {
#if BROWSER_IE
    capture_task_->SetListenerWindow(NULL, 0);
#else
    capture_task_->SetListener(NULL);
#endif
    // No need to fire failed events since the current page is being unloaded
    // or the resource store deleted for some other reason.
    need_to_fire_failed_events_ = false;
    capture_task_->Abort();
    capture_task_.release()->DeleteWhenDone();
  }

  if (current_request_.get()) {
    current_request_->callback.reset(NULL);
  }

  for (std::deque<CaptureRequest*>::iterator iter = pending_requests_.begin();
       iter != pending_requests_.end(); ++iter) {
    delete (*iter);
  }
  pending_requests_.clear();
}

//------------------------------------------------------------------------------
// StartCaptureTaskIfNeeded
//------------------------------------------------------------------------------
bool
GearsResourceStore::StartCaptureTaskIfNeeded(bool fire_events_on_failure) {
  if (page_is_unloaded_) {
    // We silently fail for this particular error condition to prevent callers
    // from detecting errors and making noises after the page has been unloaded
    return true;
  }

  // Create an event monitor to alert us when the page unloads.
  if (unload_monitor_ == NULL) {
    unload_monitor_.reset(new JsEventMonitor(GetJsRunner(), JSEVENT_UNLOAD,
                                             this));
  }

  if (capture_task_.get()) {
    assert(current_request_.get());
    return true;
  }

  if (pending_requests_.empty()) {
    return true;
  }

  assert(!current_request_.get());
  current_request_.reset(pending_requests_.front());
  pending_requests_.pop_front();

  capture_task_.reset(new CaptureTask(EnvPageBrowsingContext()));
  if (!capture_task_->Init(&store_, current_request_.get())) {
    scoped_ptr<CaptureRequest> failed_request(current_request_.release());
    capture_task_.reset(NULL);
    if (fire_events_on_failure) {
      FireFailedEvents(failed_request.get());
    }
    exception_message_ = STRING16(L"Failed to initialize capture task.");
    return false;
  }

#if BROWSER_IE
  message_hwnd_->Initialize();
  capture_task_->SetListenerWindow(
      message_hwnd_->m_hWnd,
      GearsResourceStoreMessageHwnd::kCaptureTaskMessageBase);
#else
  capture_task_->SetListener(this);
#endif
  need_to_fire_failed_events_ = true;
  if (!capture_task_->Start()) {
    scoped_ptr<CaptureRequest> failed_request(current_request_.release());
    capture_task_.reset(NULL);
    if (fire_events_on_failure) {
      FireFailedEvents(failed_request.get());
    }
    exception_message_ = STRING16(L"Failed to start capture task.");
    return false;
  }

  return true;
}

//------------------------------------------------------------------------------
// HandleEvent
//------------------------------------------------------------------------------
#if BROWSER_IE
  // On IE, AsyncTask uses a GearsResourceStoreMessageHwnd instead.
#else
void GearsResourceStore::HandleEvent(int code, int param,
                                     AsyncTask *source) {
  if (source && (source == capture_task_.get())) {
    if (code == CaptureTask::CAPTURE_TASK_COMPLETE) {
      OnCaptureTaskComplete();
    } else {
      // param = the index of the url that has been processed
      bool success = (code == CaptureTask::CAPTURE_URL_SUCCEEDED);
      OnCaptureUrlComplete(param, success);
    }
  }
}
#endif

//------------------------------------------------------------------------------
// OnCaptureUrlComplete
//------------------------------------------------------------------------------
void GearsResourceStore::OnCaptureUrlComplete(int index, bool success) {
  if (current_request_.get()) {
    need_to_fire_failed_events_ = false;
    InvokeCompletionCallback(current_request_.get(),
                             current_request_->urls[index],
                             current_request_->id,
                             success);
  }
}

//------------------------------------------------------------------------------
// OnCaptureTaskComplete
//------------------------------------------------------------------------------
void GearsResourceStore::OnCaptureTaskComplete() {
#if BROWSER_IE
  capture_task_->SetListenerWindow(NULL, 0);
#else
  capture_task_->SetListener(NULL);
#endif
  capture_task_.release()->DeleteWhenDone();
  if (need_to_fire_failed_events_) {
    assert(current_request_.get());
    FireFailedEvents(current_request_.get());
  }
  current_request_.reset(NULL);
  StartCaptureTaskIfNeeded(true);
}

//------------------------------------------------------------------------------
// FireFailedEvents
//------------------------------------------------------------------------------
void GearsResourceStore::FireFailedEvents(CaptureRequest *request) {
  assert(request);
  for (size_t i = 0; i < request->urls.size(); ++i) {
    InvokeCompletionCallback(request,
                             request->urls[i],
                             request->id,
                             false);
  }
}

//------------------------------------------------------------------------------
// InvokeCompletionCallback
//------------------------------------------------------------------------------
void GearsResourceStore::InvokeCompletionCallback(
                             CaptureRequest *request,
                             const std::string16 &capture_url,
                             int capture_id,
                             bool succeeded) {
  // If completion callback was not set, return immediately
  if (!request->callback.get()) { return; }

  const int argc = 3;
  JsParamToSend argv[argc] = {
    { JSPARAM_STRING16, &capture_url },
    { JSPARAM_BOOL, &succeeded },
    { JSPARAM_INT, &capture_id }
  };
  GetJsRunner()->InvokeCallback(request->callback.get(), argc, argv, NULL);
}

//------------------------------------------------------------------------------
// ResolveAndAppendUrl
//------------------------------------------------------------------------------
bool GearsResourceStore::ResolveAndAppendUrl(const std::string16 &url,
                                             CaptureRequest *request) {
  std::string16 full_url;
  if (!ResolveUrl(url.c_str(), &full_url)) {
    return false;
  }
  request->urls.push_back(url);
  request->full_urls.push_back(full_url);
  return true;
}

//------------------------------------------------------------------------------
// This helper does several things:
// - resolve relative urls based on the page location, the 'url' may also
//   be an absolute url to start with, if so this step does not modify it
// - normalizes the resulting absolute url, ie. removes path navigation
// - removes the fragment part of the url, ie. truncates at the '#' character
// - ensures the the resulting url is from the same-origin
//------------------------------------------------------------------------------
bool GearsResourceStore::ResolveUrl(const std::string16 &url,
                                    std::string16 *resolved_url) {
  if (!ResolveAndNormalize(EnvPageLocationUrl().c_str(), url.c_str(),
                           resolved_url)) {
    exception_message_ = STRING16(L"Failed to resolve url.");
    return false;
  }
  if (!EnvPageSecurityOrigin().IsSameOriginAsUrl(resolved_url->c_str())) {
    exception_message_ = STRING16(L"Url is not from the same origin");
    return false;
  }
  return true;
}
