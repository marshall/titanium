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
//
// When a TimedCallback object is created, it launches a new thread which calls
// back its listener after the specified time interval has elapsed.

#ifndef GEARS_GEOLOCATION_TIMED_CALLBACK_H__
#define GEARS_GEOLOCATION_TIMED_CALLBACK_H__

#include "gears/base/common/event.h"
#include "gears/base/common/thread.h"

class TimedCallback : public Thread {
 public:
  class ListenerInterface {
   public:
    virtual void OnTimeout(TimedCallback *caller, void *user_data) = 0;
    virtual ~ListenerInterface() {}
  };

  TimedCallback(ListenerInterface *listener,
                int timeout_milliseconds,
                void *user_data);
  ~TimedCallback();

 private:
  // Thread implementation.
  void Run();

  ListenerInterface *listener_;
  int timeout_;
  void *user_data_;

  Event stop_event_;

  DISALLOW_EVIL_CONSTRUCTORS(TimedCallback);
};

#endif  // GEARS_GEOLOCATION_TIMED_CALLBACK_H__
