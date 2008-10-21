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

#ifndef GEARS_HTTPREQUEST_HTTPREQUEST_H__
#define GEARS_HTTPREQUEST_HTTPREQUEST_H__

#include "gears/base/common/base_class.h"
#include "gears/base/common/common.h"
#include "gears/base/common/js_runner.h"
#include "gears/blob/blob.h"
#include "gears/localserver/common/http_request.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

class GearsHttpRequestUpload;

class GearsHttpRequest
    : public ModuleImplBaseClass,
      public JsEventHandlerInterface,
      public HttpRequest::HttpListener {
 public:
  static const std::string kModuleName;

  GearsHttpRequest();

  // IN: -
  // OUT: -
  void Abort(JsCallContext *context);

  // IN: string method, string URL
  // OUT: -
  void Open(JsCallContext *context);

  // IN: -
  // OUT: string
  void GetAllResponseHeaders(JsCallContext *context);

  // IN: -
  // OUT: function
  void GetOnProgress(JsCallContext *context);

  // IN: -
  // OUT: function
  void GetOnReadyStateChange(JsCallContext *context);

  // IN: -
  // OUT: int
  void GetReadyState(JsCallContext *context);

  // IN: string headerName
  // OUT: string
  void GetResponseHeader(JsCallContext *context);

  // IN: -
  // OUT: GearsBlob
  void GetResponseBlob(JsCallContext *context);

  // IN: -
  // OUT: string
  void GetResponseText(JsCallContext *context);

  // IN: -
  // OUT: int
  void GetStatus(JsCallContext *context);

  // IN: -
  // OUT: string
  void GetStatusText(JsCallContext *context);

  // IN: -
  // OUT: GearsHttpRequestUpload
  void GetUpload(JsCallContext *context);

  // IN: optional string|GearsBlob postData
  // OUT: -
  void Send(JsCallContext *context);

  // IN: function
  // OUT: -
  void SetOnProgress(JsCallContext *context);

  // IN: function
  // OUT: -
  void SetOnReadyStateChange(JsCallContext *context);

  // IN: string name, string value
  // OUT: -
  void SetRequestHeader(JsCallContext *context);

 private:
  scoped_refptr<HttpRequest> request_;
  bool content_type_header_was_set_;
  bool has_fired_completion_event_;
  bool length_computable_;
  int content_length_;
  scoped_ptr<JsRootedCallback> onprogresshandler_;
  scoped_ptr<JsRootedCallback> onreadystatechangehandler_;
  scoped_ptr<JsEventMonitor> unload_monitor_;
  scoped_refptr<GearsHttpRequestUpload> upload_;
  scoped_ptr<std::string16> response_text_;
  scoped_refptr<GearsBlob> response_blob_;

  void AbortRequest();
  void CreateRequest();
  void ReleaseRequest();

  HttpRequest::ReadyState GetState();
  bool IsUninitialized() { return GetState() == HttpRequest::UNINITIALIZED; }
  bool IsOpen()          { return GetState() == HttpRequest::OPEN; }
  bool IsSent()          { return GetState() == HttpRequest::SENT; }
  bool IsInteractive()   { return GetState() == HttpRequest::INTERACTIVE; }
  bool IsComplete()      { return GetState() == HttpRequest::COMPLETE; }
  bool IsValidResponse();

  bool GetResponseBlobImpl(JsCallContext *context,
                           scoped_refptr<BlobInterface> *blob);
  bool ResolveUrl(const std::string16 &url, std::string16 *resolved_url,
                  std::string16 *exception_message);

  // This needs to be called when a request is created, or if
  // onprogresshangehandler_ or onreadystatechangehandler_ is set.
  // It is safe to call multiple times.
  void InitUnloadMonitor();

  // HttpRequest::HttpListener implementation.
  virtual void DataAvailable(HttpRequest *source, int64 position);
  virtual void ReadyStateChanged(HttpRequest *source);
  virtual void UploadProgress(HttpRequest *source, int64 position, int64 total);

  // JsEventHandlerInterface implementation.
  virtual void HandleEvent(JsEventType event_type);

  ~GearsHttpRequest();
  DISALLOW_EVIL_CONSTRUCTORS(GearsHttpRequest);
};

#endif // GEARS_HTTPREQUEST_HTTPREQUEST_H__
