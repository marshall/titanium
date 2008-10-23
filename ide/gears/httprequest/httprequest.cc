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

#include "gears/httprequest/httprequest.h"

#include <limits>

#include "gears/base/common/base_class.h"
#include "gears/base/common/common.h"
#include "gears/base/common/dispatcher.h"
#include "gears/base/common/module_wrapper.h"
#include "gears/base/common/js_runner.h"
#include "gears/base/common/string_utils.h"
#include "gears/base/common/url_utils.h"
#include "gears/blob/blob.h"
#include "gears/blob/blob_interface.h"
#include "gears/blob/blob_utils.h"
#include "gears/blob/buffer_blob.h"
#include "gears/httprequest/httprequest_upload.h"

// Error messages.
static const char16 *kRequestFailedError = STRING16(L"The request failed.");
static const char16 *kInternalError = STRING16(L"Internal error.");
static const char16 *kAlreadyOpenError =  STRING16(L"Request is already open.");
static const char16 *kFailedURLResolveError =
                         STRING16(L"Failed to resolve URL.");
static const char16 *kNotCompleteError = STRING16(L"Request is not done.");
static const char16 *kNotOpenError = STRING16(L"Request is not open.");
static const char16 *kNotInteractiveError =
                        STRING16(L"Request is not loading or done.");
static const char16 *kURLNotFromSameOriginError =
                         STRING16(L"URL is not from the same origin.");

DECLARE_DISPATCHER(GearsHttpRequest);

// static
template<>
void Dispatcher<GearsHttpRequest>::Init() {
  RegisterMethod("abort", &GearsHttpRequest::Abort);
  RegisterMethod("getResponseHeader", &GearsHttpRequest::GetResponseHeader);
  RegisterMethod("getAllResponseHeaders",
                 &GearsHttpRequest::GetAllResponseHeaders);
  RegisterMethod("open", &GearsHttpRequest::Open);
  RegisterMethod("setRequestHeader", &GearsHttpRequest::SetRequestHeader);
  RegisterMethod("send", &GearsHttpRequest::Send);
  RegisterProperty("onprogress", &GearsHttpRequest::GetOnProgress,
                   &GearsHttpRequest::SetOnProgress);
  RegisterProperty("onreadystatechange",
                   &GearsHttpRequest::GetOnReadyStateChange,
                   &GearsHttpRequest::SetOnReadyStateChange);
  RegisterProperty("readyState", &GearsHttpRequest::GetReadyState, NULL);
  RegisterProperty("responseBlob", &GearsHttpRequest::GetResponseBlob, NULL);
  RegisterProperty("responseText", &GearsHttpRequest::GetResponseText, NULL);
  RegisterProperty("status", &GearsHttpRequest::GetStatus, NULL);
  RegisterProperty("statusText", &GearsHttpRequest::GetStatusText, NULL);
  RegisterProperty("upload", &GearsHttpRequest::GetUpload, NULL);
}

const std::string GearsHttpRequest::kModuleName("GearsHttpRequest");

GearsHttpRequest::GearsHttpRequest()
    : ModuleImplBaseClass(kModuleName),
      request_(NULL),
      content_type_header_was_set_(false),
      has_fired_completion_event_(false),
      length_computable_(false),
      content_length_(-1) {
}

GearsHttpRequest::~GearsHttpRequest() {
  AbortRequest();
}

void GearsHttpRequest::HandleEvent(JsEventType event_type) {
  assert(event_type == JSEVENT_UNLOAD);
  onprogresshandler_.reset();
  onreadystatechangehandler_.reset();
  upload_.reset();
  unload_monitor_.reset();
  AbortRequest();
}

void GearsHttpRequest::Open(JsCallContext *context) {
  if (IsComplete()) {
    ReleaseRequest();
  }
  if (!IsUninitialized()) {
    context->SetException(kAlreadyOpenError);
    return;
  }

  std::string16 method;
  std::string16 url;
  bool async;  // We ignore this parameter.
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &method },
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &url },
    { JSPARAM_OPTIONAL, JSPARAM_BOOL, &async },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set()) {
    return;
  }
  if (method.empty()) {
    context->SetException(STRING16(L"The method parameter is required."));
    return;
  }
  if (url.empty()) {
    context->SetException(STRING16(L"The url parameter is required."));
    return;
  }
  std::string16 full_url;
  std::string16 exception_message;
  if (!ResolveUrl(url, &full_url, &exception_message)) {
    context->SetException(exception_message);
    return;
  }

  CreateRequest();
  InitUnloadMonitor();
  content_type_header_was_set_ = false;
  has_fired_completion_event_ = false;
  if (!request_->Open(method.c_str(), full_url.c_str(), true,
                      EnvPageBrowsingContext())) {
    ReleaseRequest();
    context->SetException(kInternalError);
    return;
  }
}

static bool IsDisallowedHeader(const char16 *header) {
  // Headers which cannot be set according to the w3c spec.
  static const char16* kDisallowedHeaders[] = {
      STRING16(L"Accept-Charset"),
      STRING16(L"Accept-Encoding"),
      STRING16(L"Connection"),
      STRING16(L"Content-Length"),
      STRING16(L"Content-Transfer-Encoding"),
      STRING16(L"Date"),
      STRING16(L"Expect"),
      STRING16(L"Host"),
      STRING16(L"Keep-Alive"),
      STRING16(L"Referer"),
      STRING16(L"TE"),
      STRING16(L"Trailer"),
      STRING16(L"Transfer-Encoding"),
      STRING16(L"Upgrade"),
      STRING16(L"Via") };
  for (int i = 0; i < static_cast<int>(ARRAYSIZE(kDisallowedHeaders)); ++i) {
    if (StringCompareIgnoreCase(header, kDisallowedHeaders[i]) == 0) {
      return true;
    }
  }
  return false;
}

void GearsHttpRequest::SetRequestHeader(JsCallContext *context) {
  std::string16 name;
  std::string16 value;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &name },
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &value }
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set()) {
    return;
  }

  if (!IsOpen()) {
    context->SetException(kNotOpenError);
    return;
  }
  if (IsDisallowedHeader(name.c_str())) {
    context->SetException(STRING16(L"This header may not be set."));
    return;
  }
  if (!request_->SetRequestHeader(name.c_str(), value.c_str())) {
    context->SetException(kInternalError);
    return;
  }
  if (StringCompareIgnoreCase(name.c_str(),
      HttpConstants::kContentTypeHeader) == 0) {
    content_type_header_was_set_ = true;
  }
}

void GearsHttpRequest::Send(JsCallContext *context) {
  if (!IsOpen()) {
    context->SetException(kNotOpenError);
    return;
  }

  std::string16 post_data_string;
  ModuleImplBaseClass *post_data_module = NULL;

  int post_data_type = context->GetArgumentType(0);
  // The Gears JS API treats a (JavaScript) null and undefined the same.
  // Furthermore, if there is no arg at all (rather than an explicit null or
  // undefined arg) then GetArgumentType will return JSPARAM_UNKNOWN.
  if ((post_data_type != JSPARAM_NULL) &&
      (post_data_type != JSPARAM_UNDEFINED) &&
      (post_data_type != JSPARAM_UNKNOWN)) {
    JsArgument argv[] = {
      { JSPARAM_OPTIONAL, JSPARAM_UNKNOWN, NULL }
    };
    if (post_data_type == JSPARAM_STRING16) {
      argv[0].type = JSPARAM_STRING16;
      argv[0].value_ptr = &post_data_string;
    // Dispatcher modules are also JavaScript objects, and so it is valid for
    // GetArgumentType to return JSPARAM_OBJECT for a Dispatcher module.
    // TODO(nigeltao): fix this, so that it's always just
    // JSPARAM_MODULE, and not JSPARAM_OBJECT.
    } else if ((post_data_type == JSPARAM_MODULE) ||
               (post_data_type == JSPARAM_OBJECT)) {
      argv[0].type = JSPARAM_MODULE;
      argv[0].value_ptr = &post_data_module;
    } else {
      context->SetException(
          STRING16(L"First parameter must be a Blob or a string."));
      return;
    }
    context->GetArguments(ARRAYSIZE(argv), argv);
    if (context->is_exception_set()) {
      return;
    }
    if (post_data_module &&
        GearsBlob::kModuleName != post_data_module->get_module_name()) {
      context->SetException(
          STRING16(L"First parameter must be a Blob or a string."));
      return;
    }
  }

  scoped_refptr<HttpRequest> request_being_sent = request_;

  scoped_refptr<BlobInterface> blob;
  if (!post_data_string.empty()) {
    if (!content_type_header_was_set_) {
      request_->SetRequestHeader(HttpConstants::kContentTypeHeader,
                                 HttpConstants::kMimeTextPlain);
    }
    std::string utf8_string;
    String16ToUTF8(post_data_string.data(), post_data_string.length(),
                   &utf8_string);
    blob.reset(new BufferBlob(utf8_string.data(), utf8_string.length()));
  } else if (post_data_module) {
    if (!content_type_header_was_set_) {
      request_->SetRequestHeader(HttpConstants::kContentTypeHeader,
                                 HttpConstants::kMimeApplicationOctetStream);
    }
    static_cast<GearsBlob*>(post_data_module)->GetContents(&blob);
  }
  bool ok = request_->Send(blob.get());

  if (!ok) {
    if (!has_fired_completion_event_) {
      // We only throw here if we haven't surfaced the error through
      // an onreadystatechange callback. Since the JS code for
      // xhr.onreadystatechange might call xhr.open(), check whether
      // 'request_' has changed, which indicates that happened.
      // Also, we don't trust IsComplete() to indicate that we actually
      // fired the event, the underlying C++ object *may* declare itself
      // complete without having called our callback. We're being defensive.
      if (request_ == request_being_sent) {
        // To remove cyclic dependencies we drop our reference to callbacks
        // when the request is complete.
        onprogresshandler_.reset();
        onreadystatechangehandler_.reset();
        if (upload_.get()) {
          upload_->ResetOnProgressHandler();
        }
        context->SetException(kInternalError);
        return;
      }
    }
  }
}

void GearsHttpRequest::Abort(JsCallContext *context) {
  AbortRequest();
}

void GearsHttpRequest::GetResponseHeader(JsCallContext *context) {
  std::string16 header_name;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &header_name },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set()) {
    return;
  }
  if (!(IsInteractive() || IsComplete())) {
    context->SetException(kNotInteractiveError);
    return;
  }
  if (!IsValidResponse()) {
    std::string16 empty_string;
    context->SetReturnValue(JSPARAM_STRING16, &empty_string);
    return;
  }
  std::string16 header_value;
  if (!request_->GetResponseHeader(header_name.c_str(), &header_value)) {
    context->SetException(kInternalError);
    return;
  }
  context->SetReturnValue(JSPARAM_STRING16, &header_value);
}

void GearsHttpRequest::GetAllResponseHeaders(JsCallContext *context) {
  if (!(IsInteractive() || IsComplete())) {
    context->SetException(kNotInteractiveError);
    return;
  }
  if (!IsValidResponse()) {
    std::string16 empty_string;
    context->SetReturnValue(JSPARAM_STRING16, &empty_string);
    return;
  }
  std::string16 all_headers;
  if (!request_->GetAllResponseHeaders(&all_headers)) {
    context->SetException(kInternalError);
    return;
  }
  context->SetReturnValue(JSPARAM_STRING16, &all_headers);
}

void GearsHttpRequest::GetUpload(JsCallContext *context) {
  if (upload_.get() == NULL) {
    InitUnloadMonitor();

    if (!CreateModule<GearsHttpRequestUpload>(module_environment_.get(),
                                              context, &upload_)) {
      return;
    }
  }
  context->SetReturnValue(JSPARAM_MODULE, upload_.get());
}

void GearsHttpRequest::GetOnProgress(JsCallContext *context) {
  JsRootedCallback *callback = onprogresshandler_.get();
  if (callback == NULL) {
    context->SetReturnValue(JSPARAM_NULL, 0);
  } else {
    context->SetReturnValue(JSPARAM_FUNCTION, callback);
  }
}

void GearsHttpRequest::SetOnProgress(JsCallContext *context) {
  JsRootedCallback *function = NULL;
  JsArgument argv[] = {
    { JSPARAM_OPTIONAL, JSPARAM_FUNCTION, &function },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set()) {
    return;
  }
  onprogresshandler_.reset(function);
  InitUnloadMonitor();
}

void GearsHttpRequest::GetOnReadyStateChange(JsCallContext *context) {
  JsRootedCallback *callback = onreadystatechangehandler_.get();
  // TODO(nigeltao): possibly move the null check into
  // JsCallContext::SetReturnValue, so that I can call
  // SetReturnValue(JSPARAM_FUNCTION, NULL) to get a (JavaScript) null, rather
  // than having to explicitly check for a NULL JsRootedCallback*.
  if (callback == NULL) {
    context->SetReturnValue(JSPARAM_NULL, 0);
  } else {
    context->SetReturnValue(JSPARAM_FUNCTION, callback);
  }
}

void GearsHttpRequest::SetOnReadyStateChange(JsCallContext *context) {
  JsRootedCallback *function = NULL;
  JsArgument argv[] = {
    { JSPARAM_OPTIONAL, JSPARAM_FUNCTION, &function },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set()) {
    return;
  }
  onreadystatechangehandler_.reset(function);
  InitUnloadMonitor();
}

void GearsHttpRequest::GetReadyState(JsCallContext *context) {
  int ready_state = GetState();
  context->SetReturnValue(JSPARAM_INT, &ready_state);
}

void GearsHttpRequest::GetResponseBlob(JsCallContext *context) {
  if (!IsComplete()) {
    context->SetException(kNotCompleteError);
    return;
  }
  if (!IsValidResponse()) {
    context->SetReturnValue(JSPARAM_NULL, 0);
    return;
  }
  scoped_refptr<BlobInterface> unused_blob;
  if (GetResponseBlobImpl(context, &unused_blob)) {
    context->SetReturnValue(JSPARAM_MODULE, response_blob_.get());
    return;
  }
  // Exception set by GetResponseBlobImpl if it fails.
}

void GearsHttpRequest::GetResponseText(JsCallContext *context) {
  if (!(IsInteractive() || IsComplete())) {
    context->SetException(kNotInteractiveError);
    return;
  }
  if (!IsValidResponse()) {
    std::string empty_string;
    context->SetReturnValue(JSPARAM_STRING16, &empty_string);
    return;
  }
  // First, check the cached result (the response_text_ member variable).  It
  // is only set if GetResponseText was previously called when IsComplete().
  if (response_text_.get()) {
    context->SetReturnValue(JSPARAM_STRING16, response_text_.get());
    return;
  }
  scoped_refptr<BlobInterface> blob;
  if (!GetResponseBlobImpl(context, &blob)) {
    // Exception set by GetResponseBlobImpl.
    return;
  }
  scoped_ptr<std::string16> text(new std::string16);
  if (!BlobToString16(blob.get(), request_->GetResponseCharset(), text.get())) {
    context->SetException(kInternalError);
    return;
  }
  // Return and optionally cache the returned value.
  context->SetReturnValue(JSPARAM_STRING16, text.get());
  if (IsComplete()) {
    response_text_.swap(text);
  }
}

bool GearsHttpRequest::GetResponseBlobImpl(JsCallContext *context,
                                           scoped_refptr<BlobInterface> *blob) {
  // Re-use the previously created GearsBlob object, if it exists.
  if (response_blob_.get()) {
    response_blob_->GetContents(blob);
    return true;
  }
  assert(IsValidResponse());
  if (!request_->GetResponseBody(blob)) {
    context->SetException(kInternalError);
    return false;
  }
  if (IsComplete()) {
    if (!CreateModule<GearsBlob>(module_environment_.get(),
                                 context, &response_blob_)) {
      return false;
    }
    response_blob_->Reset(blob->get());
  }
  return true;
}

void GearsHttpRequest::GetStatus(JsCallContext *context) {
  if (!(IsInteractive() || IsComplete())) {
    context->SetException(kNotInteractiveError);
    return;
  }
  int status_code;
  if (!request_->GetStatus(&status_code)) {
    context->SetException(kInternalError);
    return;
  }
  if (!IsValidResponseCode(status_code)) {
    context->SetException(kRequestFailedError);
    return;
  }
  context->SetReturnValue(JSPARAM_INT, &status_code);
}

void GearsHttpRequest::GetStatusText(JsCallContext *context) {
  if (!(IsInteractive() || IsComplete())) {
    context->SetException(kNotInteractiveError);
    return;
  }
  if (!IsValidResponse()) {
    context->SetException(kRequestFailedError);
    return;
  }
  std::string16 status_str;
  if (!request_->GetStatusText(&status_str)) {
    context->SetException(kInternalError);
    return;
  }
  context->SetReturnValue(JSPARAM_STRING16, &status_str);
}

void GearsHttpRequest::AbortRequest() {
  if (request_.get()) {
    request_->SetListener(NULL, false);
    request_->Abort();
    ReleaseRequest();
  }
}

void GearsHttpRequest::CreateRequest() {
  ReleaseRequest();
  HttpRequest::CreateSafeRequest(&request_);
  request_->SetListener(this, true);
  request_->SetCachingBehavior(HttpRequest::USE_ALL_CACHES);
  request_->SetRedirectBehavior(HttpRequest::FOLLOW_WITHIN_ORIGIN);
}


void GearsHttpRequest::ReleaseRequest() {
  if (request_.get()) {
    request_->SetListener(NULL, false);
    request_.reset();
  }
  response_text_.reset();
  response_blob_.reset();
}

HttpRequest::ReadyState GearsHttpRequest::GetState() {
  HttpRequest::ReadyState state = HttpRequest::UNINITIALIZED;
  if (request_.get()) {
    request_->GetReadyState(&state);
  }
  return state;
}

bool GearsHttpRequest::IsValidResponse() {
  assert(IsInteractive() || IsComplete());
  int status_code = -1;
  if (!request_->GetStatus(&status_code))
    return false;
  return ::IsValidResponseCode(status_code);
}

//------------------------------------------------------------------------------
// This helper does several things:
// - resolve relative urls based on the page location, the 'url' may also
//   be an absolute url to start with, if so this step does not modify it.
// - normalizes the resulting absolute url, ie. removes path navigation.
// - removes the fragment part of the url, ie. truncates at the '#' character.
// - ensures the the resulting url is from the same-origin.
// - ensures the requested url is HTTP or HTTPS.
//------------------------------------------------------------------------------
bool GearsHttpRequest::ResolveUrl(const std::string16 &url,
                                  std::string16 *resolved_url,
                                  std::string16 *exception_message) {
  assert(resolved_url && exception_message);
  if (!ResolveAndNormalize(EnvPageLocationUrl().c_str(), url.c_str(),
                           resolved_url)) {
    *exception_message = kFailedURLResolveError;
    return false;
  }
  SecurityOrigin url_origin;
  if (!url_origin.InitFromUrl(resolved_url->c_str()) ||
      !url_origin.IsSameOrigin(EnvPageSecurityOrigin())) {
    *exception_message = kURLNotFromSameOriginError;
    return false;
  }
  if (!HttpRequest::IsSchemeSupported(url_origin.scheme().c_str())) {
    *exception_message = STRING16(L"URL scheme '");
    *exception_message += url_origin.scheme();
    *exception_message += STRING16(L"' is not supported.");
    return false;
  }
  return true;
}

void GearsHttpRequest::DataAvailable(HttpRequest *source, int64 position) {
  assert(source == request_.get());

  // Guard against being destroyed in the first script callback.
  scoped_refptr<GearsHttpRequest> reference(this);

  // We repeatedly fire ready state changed, in state 3, as data arrives.
  ReadyStateChanged(source);
  if (source != request_.get()) {
    return;  // we may have been aborted
  }

  // report progress
  JsRootedCallback *handler = onprogresshandler_.get();
  if (handler) {
    JsRunnerInterface *runner = GetJsRunner();
    assert(runner);
    if (runner) {
      scoped_ptr<JsObject> js_object(runner->NewObject());
      assert(js_object.get());
      if (content_length_ < 0) {
        // cache on first access
        content_length_ = 0;
        int content_length = 0;
        std::string16 header_value;
        request_->GetResponseHeader(HttpConstants::kContentEncodingHeader,
                                    &header_value);
        // can only compute total if Content-Encoding is not used
        if (header_value.empty()) {
          request_->GetResponseHeader(HttpConstants::kContentLengthHeader,
                                      &header_value);
          if (!header_value.empty()) {
            const char16 *endptr;
            content_length = ParseLeadingInteger(header_value.c_str(), &endptr);
            if (*endptr == '\0' && content_length >= 0) {
              // and Content-Length is set
              length_computable_ = true;
              content_length_ = content_length;
            }
          }
        }
      }
      js_object->SetPropertyDouble(STRING16(L"total"),
                                   static_cast<double>(content_length_));
      js_object->SetPropertyDouble(STRING16(L"loaded"),
                                   static_cast<double>(position));
      js_object->SetPropertyBool(STRING16(L"lengthComputable"),
                                 length_computable_);
      JsParamToSend argv[] = {
        { JSPARAM_OBJECT, js_object.get() },
      };
      runner->InvokeCallback(handler, ARRAYSIZE(argv), argv, NULL);
    }
  }
}

void GearsHttpRequest::ReadyStateChanged(HttpRequest *source) {
  assert(source == request_.get());

  // To remove cyclic dependencies we drop our reference to callbacks when
  // the request is complete.
  bool is_complete = IsComplete();
  if (is_complete) {
    has_fired_completion_event_ = true;
    onprogresshandler_.reset();
    if (upload_.get()) {
      upload_->ResetOnProgressHandler();
    }
  }
  JsRootedCallback *handler = is_complete
      ? onreadystatechangehandler_.release()
      : onreadystatechangehandler_.get();
  if (handler) {
    JsRunnerInterface *runner = GetJsRunner();
    assert(runner);
    if (runner) {
      runner->InvokeCallback(handler, 0, NULL, NULL);
    }
    if (is_complete) {
      delete handler;
    }
  }
}

void GearsHttpRequest::InitUnloadMonitor() {
  if (unload_monitor_ == NULL) {
    unload_monitor_.reset(
        new JsEventMonitor(GetJsRunner(), JSEVENT_UNLOAD, this));
  }
}

void GearsHttpRequest::UploadProgress(HttpRequest *source,
                                      int64 position, int64 total) {
  assert(source == request_.get());
  if (upload_.get()) {
    upload_->ReportProgress(position, total);
  }
}
