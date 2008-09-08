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

#include "gears/media/audio.h"
#include "gears/media/time_ranges.h"

#include "gears/base/common/module_wrapper.h"
#include "gears/base/common/js_runner.h"

DECLARE_GEARS_WRAPPER(GearsAudio);
REGISTER_MEDIA_PROPERTIES_AND_METHODS(GearsAudio);

GearsAudio::GearsAudio() : ModuleImplBaseClass("GearsAudio") {
}

GearsAudio::~GearsAudio() {
}

void GearsAudio::GetError(JsCallContext *context) {
  int last_error = media_data_->GetLastError();
  if (last_error == MediaConstants::MEDIA_NO_ERROR) {
    context->SetReturnValue(JSPARAM_NULL, MediaConstants::MEDIA_NO_ERROR);
    return;
  }

  JsRunnerInterface* js_runner = this->GetJsRunner();
  scoped_ptr<JsObject> error_object(js_runner->NewObject());
  if (!error_object.get()) {
    context->SetException(STRING16(L"Failed to create new javascript object."));
  }

  error_object->SetPropertyInt(STRING16(L"code"), last_error);
  error_object->SetPropertyInt(STRING16(L"MEDIA_ERR_ABORTED"),
                               MediaConstants::MEDIA_ERR_ABORTED);
  error_object->SetPropertyInt(STRING16(L"MEDIA_ERR_NETWORK"),
                               MediaConstants::MEDIA_ERR_NETWORK);
  error_object->SetPropertyInt(STRING16(L"MEDIA_ERR_DECODE"),
                               MediaConstants::MEDIA_ERR_DECODE);
  context->SetReturnValue(JSPARAM_OBJECT, error_object.get());
}

void GearsAudio::GetBuffered(JsCallContext *context) {
  scoped_refptr<GearsTimeRanges> buffered;
  if (!CreateModule<GearsTimeRanges>(module_environment_.get(),
                                     context, &buffered)) {
    return;
  }
  context->SetReturnValue(JSPARAM_MODULE, buffered.get());
  // TODO(aprasath): Above code is just for testing purposes. Implement me.
}

