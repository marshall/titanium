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

#ifndef GEARS_LOCALSERVER_RESOURCE_STORE_MODULE_H__
#define GEARS_LOCALSERVER_RESOURCE_STORE_MODULE_H__

#include <deque>
#include "gears/base/common/base_class.h"
#include "gears/base/common/common.h"
#include "gears/base/common/js_runner.h"
#include "gears/base/common/string16.h"
#include "gears/localserver/common/capture_task.h"
#include "gears/localserver/common/resource_store.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

#if BROWSER_IE
class GearsResourceStoreMessageHwnd;
#endif

//-----------------------------------------------------------------------------
// GearsResourceStore
//-----------------------------------------------------------------------------
class GearsResourceStore
    : public ModuleImplBaseClass,
#if BROWSER_IE
      // On IE, AsyncTask uses a GearsResourceStoreMessageHwnd instead.
#else
      public AsyncTask::Listener,
#endif
      public JsEventHandlerInterface {
 public:
  static const std::string kModuleName;

  GearsResourceStore();

  // IN: -
  // OUT: string
  void GetName(JsCallContext *context);

  // IN: -
  // OUT: string
  void GetRequiredCookie(JsCallContext *context);

  // IN: -
  // OUT: bool
  void GetEnabled(JsCallContext *context);
  // IN: bool enabled
  // OUT: -
  void SetEnabled(JsCallContext *context);

  // IN: string url | string[] url_array, function completion_callback
  // OUT: int capture_id
  void Capture(JsCallContext *context);

  // IN: int capture_id
  // OUT: -
  void AbortCapture(JsCallContext *context);

  // IN: string url
  // OUT: bool
  void IsCaptured(JsCallContext *context);

  // IN: string url
  // OUT: -
  void Remove(JsCallContext *context);

  // IN: string src_url, string dst_url
  // OUT: -
  void Rename(JsCallContext *context);

  // IN: string src_url, string dst_url
  // OUT: -
  void Copy(JsCallContext *context);

  // IN: string url, string header_name
  // OUT: string
  void GetHeader(JsCallContext *context);

  // IN: string url
  // OUT: string
  void GetAllHeaders(JsCallContext *context);

  // IN: GearsBlob blob, string url
  // OUT: -
  void CaptureBlob(JsCallContext *context);

  // IN: HtmlElement file_input_element, string url
  // OUT: -
  void CaptureFile(JsCallContext *context);

  // IN: string url
  // OUT: string
  void GetCapturedFileName(JsCallContext *context);

  // IN: -
  // OUT: GearsFileSubmitter
  void CreateFileSubmitter(JsCallContext *context);

 protected:
  ~GearsResourceStore();

 private:
  bool ResolveAndAppendUrl(const std::string16 &url, CaptureRequest *request);
  bool ResolveUrl(const std::string16 &url, std::string16 *resolved_url);
  bool StartCaptureTaskIfNeeded(bool fire_events_on_failure);
  void FireFailedEvents(CaptureRequest *request);
  void InvokeCompletionCallback(CaptureRequest *request,
                                const std::string16 &capture_url,
                                int capture_id,
                                bool succeeded);
  // JsEventHandlerInterface
  virtual void HandleEvent(JsEventType event_type);
#if BROWSER_IE
  // On IE, AsyncTask uses a GearsResourceStoreMessageHwnd instead.
#else
  // AsyncTask::Listener
  virtual void HandleEvent(int code, int param, AsyncTask *source);
#endif
  void OnCaptureUrlComplete(int index, bool success);
  void OnCaptureTaskComplete();

  void AbortAllRequests();

  scoped_ptr<JsEventMonitor> unload_monitor_;
  int next_capture_id_;
  std::deque<CaptureRequest*> pending_requests_;
  scoped_ptr<CaptureRequest> current_request_;
  scoped_ptr<CaptureTask> capture_task_;
  bool page_is_unloaded_;
  std::string16 exception_message_;
  ResourceStore store_;
  // This flag is set to true until the first OnCaptureUrlComplete is called by 
  // the capture_task_. If current_request_ is aborted before it gets a chance
  // to begin the capture, OnCaptureUrlComplete will not be called for any of
  // its urls, which implies that none of the corresponding capture callbacks
  // will be called, either. To prevent this, we inspect this flag in
  // OnCaptureTaskComplete and, if set, we call FireFailedEvents for
  // current_request_.
  bool need_to_fire_failed_events_;

  friend class GearsLocalServer;

#if BROWSER_IE
  GearsResourceStoreMessageHwnd *message_hwnd_;
  friend class GearsResourceStoreMessageHwnd;
#endif

  DISALLOW_EVIL_CONSTRUCTORS(GearsResourceStore);
};

#endif // GEARS_LOCALSERVER_RESOURCE_STORE_MODULE_H__
