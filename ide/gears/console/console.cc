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

#include "gears/console/console.h"

#include "gears/base/common/message_service.h"
#include "gears/base/common/module_wrapper.h"
#include "gears/console/log_event.h"

DECLARE_GEARS_WRAPPER(GearsConsole);

const std::string GearsConsole::kModuleName("GearsConsole");

template<>
void Dispatcher<GearsConsole>::Init() {
  RegisterMethod("log", &GearsConsole::Log);
  RegisterProperty("onlog", &GearsConsole::GetOnLog, &GearsConsole::SetOnLog);
}

void GearsConsole::Log(JsCallContext *context) {
  Initialize();
  
  // Get and sanitize parameters.
  std::string16 type_str;
  std::string16 message;
  JsArray args_array;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &type_str },
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &message },
    { JSPARAM_OPTIONAL, JSPARAM_ARRAY, &args_array},
  };
  int argc = context->GetArguments(ARRAYSIZE(argv), argv);
  
  if (context->is_exception_set())
    return;
  
  // Check input validity.
  if (type_str.length() == 0) {
    context->SetException(STRING16(L"type cannot be an empty string."));
    return;
  }
  
  if (message.length() == 0) {
    context->SetException(STRING16(L"message cannot be an empty string."));
    return;
  }
  
  std::string16 msg = message;
  if (argc == 3) {
    InterpolateArgs(&message, &args_array);
  }
  LogEvent *log_event = new LogEvent(message, type_str, EnvPageLocationUrl());
  MessageService::GetInstance()->NotifyObservers(observer_topic_.c_str(),
                                                 log_event);
}

void GearsConsole::GetOnLog(JsCallContext *context) {
  JsRootedCallback *callback = callback_backend_->GetCallback();
  if (callback == NULL) {
    context->SetReturnValue(JSPARAM_NULL, 0);
  } else {
    context->SetReturnValue(JSPARAM_FUNCTION, callback);
  }
}

void GearsConsole::SetOnLog(JsCallContext *context) {
  Initialize();
  
  // Get & sanitize parameters.
  JsRootedCallback *function = NULL;
  JsArgument argv[] = {
    { JSPARAM_OPTIONAL, JSPARAM_FUNCTION, &function },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  scoped_ptr<JsRootedCallback> scoped_function(function);
  
  if (context->is_exception_set())
    return;

  callback_backend_->SetCallback(scoped_function.release());
}

void GearsConsole::Initialize() {
  if (!callback_backend_.get()) {
    observer_topic_ =
        STRING16(L"console:logstream-") + EnvPageSecurityOrigin().url();
    callback_backend_.reset(
        new JsCallbackLoggingBackend(observer_topic_, GetJsRunner(), this));
  }

  // Create an event monitor to alert us when the page unloads.
  if (!unload_monitor_.get()) {
    unload_monitor_.reset(new JsEventMonitor(GetJsRunner(), JSEVENT_UNLOAD,
                                             this));
  }
}

// static
void GearsConsole::InterpolateArgs(std::string16 *message,
                                   const JsArray *args) {
  std::string16::size_type location = 0;
  int args_length;
  if (!args->GetLength(&args_length)) return;

  for (int i = 0; i < args_length; i++) {
    // Find the _next_ occurance of %s
    location = message->find(STRING16(L"%s"), location);
    if (location == std::string16::npos) break;

    JsScopedToken token;
    std::string16 string_value(STRING16(L"<Error converting to string>"));
    if (args->GetElement(i, &token)) {
      JsTokenToString_Coerce(token, args->context(), &string_value);
    }

    message->replace(location, 2, string_value);
    location += string_value.size();
  }
}

void GearsConsole::HandleEvent(JsEventType event_type) {
  assert(event_type == JSEVENT_UNLOAD);
  // TODO(nigeltao): do we really need to listen to the unload event!?
  callback_backend_.reset();
}
