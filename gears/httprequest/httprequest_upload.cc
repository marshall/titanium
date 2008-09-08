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

#include "gears/httprequest/httprequest_upload.h"

#include "gears/base/common/dispatcher.h"
#include "gears/base/common/js_runner.h"
#include "gears/base/common/js_types.h"
#include "gears/base/common/string16.h"

DECLARE_DISPATCHER(GearsHttpRequestUpload);

// static
template<>
void Dispatcher<GearsHttpRequestUpload>::Init() {
  RegisterProperty("onprogress", &GearsHttpRequestUpload::GetOnProgress,
                   &GearsHttpRequestUpload::SetOnProgress);
}

const std::string GearsHttpRequestUpload::kModuleName("GearsHttpRequestUpload");

void GearsHttpRequestUpload::GetOnProgress(JsCallContext *context) {
  JsRootedCallback *callback = onprogress_handler_.get();
  if (callback == NULL) {
    context->SetReturnValue(JSPARAM_NULL, 0);
  } else {
    context->SetReturnValue(JSPARAM_FUNCTION, callback);
  }
}

void GearsHttpRequestUpload::SetOnProgress(JsCallContext *context) {
  if (!unload_monitor_.get()) {
    unload_monitor_.reset(
      new JsEventMonitor(GetJsRunner(), JSEVENT_UNLOAD, this));
  }

  JsRootedCallback *function = NULL;
  JsArgument argv[] = {
    { JSPARAM_OPTIONAL, JSPARAM_FUNCTION, &function },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set()) {
    return;
  }
  onprogress_handler_.reset(function);
}

void GearsHttpRequestUpload::ReportProgress(int64 position, int64 total) {
  JsRootedCallback *handler = onprogress_handler_.get();
  if (handler) {
    JsRunnerInterface *runner = GetJsRunner();
    assert(runner);
    if (runner) {
      scoped_ptr<JsObject> js_object(runner->NewObject());
      assert(js_object.get());
      js_object->SetPropertyDouble(STRING16(L"total"),
                                   static_cast<double>(total));
      js_object->SetPropertyDouble(STRING16(L"loaded"),
                                   static_cast<double>(position));
      js_object->SetPropertyBool(STRING16(L"lengthComputable"), true);
      JsParamToSend argv[] = {
        { JSPARAM_OBJECT, js_object.get() },
      };
      runner->InvokeCallback(handler, ARRAYSIZE(argv), argv, NULL);
    }
  }
}

void GearsHttpRequestUpload::ResetOnProgressHandler() {
  onprogress_handler_.reset();
}

void GearsHttpRequestUpload::HandleEvent(JsEventType event_type) {
  assert(event_type == JSEVENT_UNLOAD);
  onprogress_handler_.reset();
  unload_monitor_.reset();
}
