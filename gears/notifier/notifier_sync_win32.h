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

#ifndef GEARS_NOTIFIER_SYNC_WIN32_H__
#define GEARS_NOTIFIER_SYNC_WIN32_H__

#ifdef OFFICIAL_BUILD
  // The notification API has not been finalized for official builds.
#else

#include "gears/base/common/event.h"

// TODO(jianli): Extend to other platforms.
class NotifierSyncGate {
 public:
   NotifierSyncGate(const char16 *name) : handle_(NULL), opened_(false) {
    handle_ = ::CreateEvent(NULL, true, false, name);
    assert(handle_);
  }

  ~NotifierSyncGate() {
    if (opened_) {
      BOOL res = ::ResetEvent(handle_);
      assert(res);
    }
    if (handle_) {
      ::CloseHandle(handle_);
    }
  }

  bool Open() {
    assert(handle_);
    assert(!opened_);
    if (!::SetEvent(handle_)) {
      return false;
    }
    opened_ = true;
    return true;
  }

  bool Wait(Event *stop_event, int timeout_ms) {
    assert(handle_);
    // TODO (jianli): Add support for WaitMultiple.
    while (timeout_ms > 0) {
      if (::WaitForSingleObject(handle_, 1000) == WAIT_OBJECT_0) {
        return true;
      }
      if (stop_event && stop_event->WaitWithTimeout(1)) {
        return false;
      }
      timeout_ms -= 1000;
    }
    return false;
  }

 private:
  HANDLE handle_;
  bool opened_;
};

#endif  // OFFICIAL_BUILD

#endif  // GEARS_NOTIFIER_SYNC_WIN32_H__
