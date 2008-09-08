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

#include "gears/base/common/timed_call.h"

#include <assert.h>

#if defined(OS_MACOSX)
#include <CoreFoundation/CoreFoundation.h>
#endif
#if defined(BROWSER_NONE) && defined(LINUX) && !defined(OS_MACOSX)
#include <gtk/gtk.h>
#endif


#include <set>

#include "gears/base/common/stopwatch.h"  // for GetCurrentTimeMillis()

#if !defined(BROWSER_NONE)
#include "gears/base/common/thread_locals.h"
#endif

// This timer is designed as a shared timer, so there is only one OS timer
// per thread handling all the timer callbacks we are asked to handle
// in that thread.

// TODO(chimene) What if the user changes the system time or if an NTP
// sync happens?  gettimeofday() and CFAbsoluteTimeGetCurrent() are not
// monotonically increasing in those cases.

class PlatformTimer;
class TimerSingleton;

// less-than operation on pointers to Timer
struct TimedCallPtrLessThan {
  bool operator()(const TimedCall *a, const TimedCall *b) const {
    if (a->deadline() != b->deadline()) {
      return a->deadline() < b->deadline();
    }
    return a < b;
  }
};

// TimerSingleton manages the queue of Timers that will fire,
// and is thread-local.
class TimerSingleton {
 public:
  static TimerSingleton *GetLocalSingleton();
  static void StaticCallback();

  void Insert(TimedCall *timer);
  void Erase(TimedCall *timer);

 private:
  void Callback();
  void RearmTimer();

  // deletes self, used when thread is destroyed
  static void DestructCallback(void *victim);

  TimerSingleton();
  ~TimerSingleton();

  std::set<TimedCall*, TimedCallPtrLessThan> *timer_queue_;
  PlatformTimer *platform_timer_;

#if defined(BROWSER_NONE) && defined(WIN32) && defined(DEBUG)
  DWORD main_thread_id_;
#endif

  // For BROWSER_NONE (notifier.exe) we have only one thread
  // and ThreadLocals is browser-specific, so we just limit ourselves to one
  // thread to eliminate the dependency on ThreadLocals.
#if defined(BROWSER_NONE)
  static TimerSingleton *timer_singleton;
#else
  static const ThreadLocals::Slot kTimerSingletonKey;
#endif

  DISALLOW_EVIL_CONSTRUCTORS(TimerSingleton);
};

#if defined(BROWSER_NONE)
TimerSingleton *TimerSingleton::timer_singleton = NULL;
#else
const ThreadLocals::Slot
  TimerSingleton::kTimerSingletonKey = ThreadLocals::Alloc();
#endif

// PlatformTimer contains the platform-specific timer implementations,
// and has the method SetNextFire, which takes a timeout in milliseconds.

#if defined(OS_MACOSX)

class PlatformTimer {
 public:
  PlatformTimer() {
    // We construct a timer that fires a long time from now
    // (3ish years in the future) and later change the fire date in SetNextFire.
    // See: developer.apple.com/documentation/CoreFoundation
    // /Reference/CFRunLoopTimerRef/Reference/reference.html
    // specifically, the discussion for CFRunLoopTimerSetNextFireDate.
    CFAbsoluteTime fire_date = CFAbsoluteTimeGetCurrent() + 100000000.0;
    CFTimeInterval interval = 100000000.0;

    timer_ = CFRunLoopTimerCreate(NULL, fire_date,
                                  interval, 0, 0,
                                  TimerCallback, NULL);
    assert(CFRunLoopTimerIsValid(timer_));
    CFRunLoopAddTimer(CFRunLoopGetCurrent(), timer_, kCFRunLoopCommonModes);
  }

  ~PlatformTimer() {
    CFRunLoopTimerInvalidate(timer_);
    CFRelease(timer_);
  }

  void Cancel() {
    // Mac doesn't need cancel.
  }

  void SetNextFire(int64 timeout) {
    // CFAbsoluteTime is a double, specified in seconds.

    // TODO(chimene) is gettimeofday's version of time close enough to
    // CFAbsoluteTimeGetCurrent that we can use it?

    CFAbsoluteTime fireDate = CFAbsoluteTimeGetCurrent() + (1.0e-3 * timeout);
    CFRunLoopTimerSetNextFireDate(timer_, fireDate);
  }

  static void TimerCallback(CFRunLoopTimerRef ref, void *context) {
    TimerSingleton::StaticCallback();
  }

  CFRunLoopTimerRef timer_;
  DISALLOW_EVIL_CONSTRUCTORS(PlatformTimer);
};
#elif defined(WIN32)
class PlatformTimer {
 public:
  PlatformTimer()
      : timer_id_(0) {
  }

  ~PlatformTimer() {
    Cancel();
  }

  // Windows timers automatically fire again unless you KillTimer them,
  // or reset them with the same timer_id.
  void Cancel() {
    if (timer_id_ != 0) {
      KillTimer(NULL, timer_id_);
    }
    timer_id_ = 0;
  }

  void SetNextFire(int64 timeout) {
    // clamp timeout to USER_TIMER_MAXIMUM, because on Windows XP pre-SP2,
    // if it's larger than USER_TIMER_MAXIMUM it is changed to a time out of 1.
    // This would be bad.
    // USER_TIMER_MAXIMUM is approximately 24.8 days.  If we have a timer
    // that should go off after longer than this period, we will wake up after
    // the 24 days and just rearm the timer again.
    // WinCE doesn't define USER_TIMER_MAXIMUM and USER_TIMER_MINIMUM,
    // so we just use magic numbers.

    if (timeout > 0x7FFFFFFF)
      timeout = 0x7FFFFFFF;

    // Avoid negative timer delays doing weird things
    if (timeout < 0x0000000A)
      timeout = 0x0000000A;

    timer_id_ = SetTimer(NULL, timer_id_, (UINT)timeout, &OnTimer);

    if (timer_id_ == 0) {
      // There was an error
      // TODO(chimene) Log it instead of asserting
      assert(timer_id_ != 0);
    }
  }

 private:
  static void CALLBACK OnTimer(HWND wnd, UINT msg,
                               UINT_PTR event_id, DWORD time) {
    TimerSingleton::StaticCallback();
  }

  UINT_PTR timer_id_;
  DISALLOW_EVIL_CONSTRUCTORS(PlatformTimer);
};
#elif defined(BROWSER_NONE) && defined(LINUX) && !defined(OS_MACOSX)
class PlatformTimer {
 public:
  PlatformTimer()
      : timer_id_(0) {
  }

  ~PlatformTimer() {
    Cancel();
  }

  void Cancel() {
    if (timer_id_) {
      g_source_remove(timer_id_);
      timer_id_ = 0;
    }
  }

  void SetNextFire(int64 timeout) {
    Cancel();

    // Bound timeout to reasonable values.
    if (timeout > 0x7FFFFFFF)
      timeout = 0x7FFFFFFF;
    if (timeout < 0x0000000A)
      timeout = 0x0000000A;
    g_timeout_add(timeout, &PlatformTimer::OnTimer, NULL);
  }

 private:
  static gboolean OnTimer(gpointer) {
    TimerSingleton::StaticCallback();
    return false;
  }

  guint timer_id_;
  DISALLOW_EVIL_CONSTRUCTORS(PlatformTimer);
};
#else  // TODO(chimene): Linux FF, wince, etc.
class PlatformTimer {
 public:
  PlatformTimer() {
    // Reminder that this doesn't work yet.
    assert(0);
  }
  ~PlatformTimer() {}
  void Cancel() {}
  void SetNextFire(int64 timeout) {}

 private:
  DISALLOW_EVIL_CONSTRUCTORS(PlatformTimer);
};
#endif

//
// TimerSingleton
//

void TimerSingleton::Insert(TimedCall *call) {
  timer_queue_->insert(call);
  RearmTimer();
}

void TimerSingleton::Erase(TimedCall *call) {
  timer_queue_->erase(call);
  RearmTimer();
}

TimerSingleton *TimerSingleton::GetLocalSingleton() {
#if defined(BROWSER_NONE)
  if (timer_singleton == NULL)
    timer_singleton = new TimerSingleton();

#if defined(POSIX)
  assert(pthread_main_np() == 1);
#elif defined(WIN32)
  assert(timer_singleton->main_thread_id_ == GetCurrentThreadId());
#endif

  return timer_singleton;

#else
  TimerSingleton *singleton = reinterpret_cast<TimerSingleton*>
    (ThreadLocals::GetValue(kTimerSingletonKey));

  if (singleton != NULL)
    return singleton;

  singleton = new TimerSingleton();
  ThreadLocals::SetValue(kTimerSingletonKey, singleton, &DestructCallback);

  return singleton;
#endif
}

void TimerSingleton::StaticCallback() {
  GetLocalSingleton()->Callback();
}

void TimerSingleton::Callback() {
  int64 now = GetCurrentTimeMillis();

  // Copy the timer queue so that callbacks can't add more TimedCalls
  // that fire immediately and get us stuck in this while loop.
  // If a callback adds an immediate callback, we'll schedule the
  // platform timer for 0 ms in the future, but we'll return to give the
  // message loop time to process other messages.
  std::set<TimedCall*, TimedCallPtrLessThan>
    current_queue(timer_queue_->begin(), timer_queue_->end());

  std::set<TimedCall*, TimedCallPtrLessThan>::iterator i =
    current_queue.begin();

  // Activate all timers that are past the deadline
  while (i != current_queue.end() && (*i)->deadline() <= now) {
    TimedCall *current_timer = (*i);
    timer_queue_->erase(current_timer);

    current_timer->Fire();

    ++i;
  }

  RearmTimer();
}

void TimerSingleton::RearmTimer() {
  if (timer_queue_->empty()) {
    platform_timer_->Cancel();
    return;
  }

  int64 next_deadline = (*timer_queue_->begin())->deadline();

  int64 now = GetCurrentTimeMillis();
  int64 next_fire = next_deadline - now;

  if (next_fire < 0) {
    next_fire = 0;
  }

  platform_timer_->SetNextFire(next_fire);
}

TimerSingleton::TimerSingleton() {
  timer_queue_ = new std::set<TimedCall*, TimedCallPtrLessThan>;
  platform_timer_ = new PlatformTimer();

#if defined(BROWSER_NONE) && defined(WIN32) && defined(DEBUG)
  main_thread_id_ = GetCurrentThreadId();
#endif
}

void TimerSingleton::DestructCallback(void *victim) {
  TimerSingleton *timer_singleton_victim =
    reinterpret_cast<TimerSingleton*>(victim);
  delete timer_singleton_victim;
}

TimerSingleton::~TimerSingleton() {
  delete timer_queue_;
  timer_queue_ = NULL;
  delete platform_timer_;
  platform_timer_ = NULL;
}

//
// TimedCall
//
TimedCall::TimedCall(int64 delay_millis, bool repeat,
                     TimedCallback callback, void *callback_parameter)
    : delay_(delay_millis),
      repeat_(repeat),
      deadline_(GetCurrentTimeMillis() + delay_millis),
      callback_(callback),
      callback_parameter_(callback_parameter) {
  assert(callback_);
  TimerSingleton::GetLocalSingleton()->Insert(this);
}

void TimedCall::Fire() {
  // Re-schedule a repeating timer
  // Re-scheduling after the callback would be bad
  // because ~TimedCall() can be called inside the callback.
  if (repeat_) {
    int64 now = GetCurrentTimeMillis();
    deadline_ = now + delay_;
    TimerSingleton::GetLocalSingleton()->Insert(this);
  }

  // Callback is called here
  if (callback_ != NULL) {
    (*callback_)(callback_parameter_);
  }
}

void TimedCall::Cancel() {
  // Works even if we've already fired
  TimerSingleton::GetLocalSingleton()->Erase(this);
}

TimedCall::~TimedCall() {
  Cancel();
}
