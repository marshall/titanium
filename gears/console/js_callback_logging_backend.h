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

#ifndef GEARS_CONSOLE_JS_CALLBACK_LOGGING_BACKEND_H__
#define GEARS_CONSOLE_JS_CALLBACK_LOGGING_BACKEND_H__

#include "gears/base/common/common.h"
#include "gears/base/common/js_runner.h"
#include "gears/base/common/string16.h"
#include "gears/console/log_event.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

class GearsConsole;

// JsCallbackLoggingBackend provides a means of accessing the log stream
// via a JavaScript callback function.
// Console instances provide access to a JavaScript callback backend via the
// onlog property.
// JsCallbackLoggingBackend is an example implementation of a logging backend.
class JsCallbackLoggingBackend : public MessageObserverInterface {
 public:
  JsCallbackLoggingBackend(const std::string16 &topic,
                           JsRunnerInterface *js_runner,
                           GearsConsole *console);
  ~JsCallbackLoggingBackend();

  virtual void OnNotify(MessageService *service,
                        const char16 *topic,
                        const NotificationData *data);

  JsRootedCallback *GetCallback();
  void SetCallback(JsRootedCallback *callback);
  void ClearCallback();

 private:
  std::string16 observer_topic_;

  // A callback for receiving log messages
  scoped_ptr<JsRootedCallback> callback_;

  // The JavaScript context for accessing and running callbacks
  JsRunnerInterface *js_runner_;

  // A weak pointer to the GearsConsole object that owns us.  We Addref this
  // during callback dispatch so that it doesn't get released and delete us.
  GearsConsole *console_;

  DISALLOW_EVIL_CONSTRUCTORS(JsCallbackLoggingBackend);
};

#endif // GEARS_CONSOLE_JS_CALLBACK_LOGGING_BACKEND_H__
