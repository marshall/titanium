// Copyright 2007, Google Inc.
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
// Adds timers that are usable in worker threads.

#ifndef GEARS_TIMER_TIMER_H__
#define GEARS_TIMER_TIMER_H__

#if BROWSER_FF
#include <gecko_internal/nsITimer.h>
#endif

#include <map>
#include "third_party/linked_ptr/linked_ptr.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

#include "gears/base/common/base_class.h"
#include "gears/base/common/common.h"
#include "gears/base/common/js_runner.h"
#include "gears/base/common/scoped_token.h"

// TODO(mpcomplete): remove when we have a cross-platform timer abstraction
#if BROWSER_NPAPI && defined(WIN32)
#define BROWSER_IE 1
#endif

// As an implementation detail, on IE we have a single WindowsPlatformTimer
// per GearsTimer.  This is basically a Window (in the HWND sense) that
// registers for WM_TIMER messages.  We re-use the same WindowsPlatformTimer
// for all timeouts and intervals set on a single GearsTimer object (the
// result of a factory.create('beta.timer') call in JavaScript).
//
// For Firefox, we use multiple "@mozilla.org/timer;1" XPCOM timers per
// GearsTimer objects, one for each setTimeout or setInterval call.
#if BROWSER_IE
class GearsTimer;

class WindowsPlatformTimer
    : public CWindowImpl<WindowsPlatformTimer> {
 public:
  BEGIN_MSG_MAP(WindowsPlatformTimer)
    MESSAGE_HANDLER(WM_TIMER, OnTimer)
  END_MSG_MAP()

  WindowsPlatformTimer(GearsTimer *gears_timer);
  ~WindowsPlatformTimer();

  void Initialize();
  void CancelGearsTimer(int timer_id);

 private:
  void OnFinalMessage(HWND hwnd);
  LRESULT OnTimer(UINT msg, WPARAM timer_id,
                  LPARAM unused_param, BOOL& handled);

  GearsTimer *gears_timer_;
  bool in_handler_;

  DISALLOW_EVIL_CONSTRUCTORS(WindowsPlatformTimer);
};
#endif

#if defined(OS_ANDROID)
#include "gears/base/common/async_router.h"
#include "gears/geolocation/timed_callback.h"

class GearsTimer;

// Android timer implementation. Uses TimedCallback to post delayed
// messages to the module's thread.
class AndroidPlatformTimer : public TimedCallback::ListenerInterface {
 public:
  AndroidPlatformTimer(GearsTimer *gears_timer);
  virtual ~AndroidPlatformTimer();

  void AddTimer(int timer_id);
  void RemoveTimer(int timer_id);

  // Called on timeout, on the TimedCallback thread.
  virtual void OnTimeout(TimedCallback *caller, void *user_data);
  // Called on timeout, on the owning thread, from AndroidTimerFunctor.
  void OnMessage(TimedCallback *caller, int timer_id);

 private:
  typedef std::map<int, TimedCallback *> TimerMap;

  GearsTimer *gears_timer_;
  ThreadId owner_id_;
  Mutex mutex_;
  TimerMap timer_map_;
  DECL_SINGLE_THREAD;
};
#endif // defined(OS_ANDROID)

class GearsTimer
    : public ModuleImplBaseClass
    , public JsEventHandlerInterface {
 public:
  GearsTimer()
      : ModuleImplBaseClass("GearsTimer"),
#if BROWSER_IE
        platform_timer_(new WindowsPlatformTimer(this)),
#elif defined(OS_ANDROID)
        platform_timer_(new AndroidPlatformTimer(this)),
#endif
        next_timer_id_(1) {}
  ~GearsTimer() {
#if BROWSER_IE
    if (platform_timer_->IsWindow()) {
      platform_timer_->DestroyWindow();
    } else {
      delete platform_timer_;
    }
#endif
  }

  // IN: variant timer_code, long timeout
  // OUT: long
  void SetTimeout(JsCallContext *context);

  // IN: long timer_id
  // OUT: nothing
  void ClearTimeout(JsCallContext *context);

  // IN: variant timer_code, long timeout
  // OUT: long
  void SetInterval(JsCallContext *context);

  // IN: long timer_id
  // OUT: nothing
  void ClearInterval(JsCallContext *context);

  // This is the callback used to handle the 'JSEVENT_UNLOAD' event.
  void HandleEvent(JsEventType event_type);

 private:
  // Contains the information that represents a single timeout or interval.
  // This includes the data representing the platform's timer structure, and
  // the callback information for when the timer fires.
  struct TimerInfo {
    TimerInfo() : timer_id(0) {}
    ~TimerInfo();

    void SetOwner(GearsTimer *new_owner) {
      owner = new_owner;
    }

    linked_ptr<JsRootedCallback> callback;
    std::string16 script;
    bool repeat;
    int timeout;
    int timer_id;
    scoped_refptr<GearsTimer> owner;
#if BROWSER_WEBKIT
    // TimerInfo is stored in an STL container, so we need a linked_ptr to
    // act as an intermediary.
    // scoped_timer ensures that timer is released properly by OS when
    // deleted.
    class CFRunLoopTimerTraits {
     public:
      static void Free(CFRunLoopTimerRef x) {
        CFRunLoopTimerInvalidate(x);
        CFRelease(x);
      }
      static CFRunLoopTimerRef Default() { return NULL; }
    };
    typedef scoped_token<CFRunLoopTimerRef, CFRunLoopTimerTraits> scoped_timer;
    linked_ptr<scoped_timer> platform_timer;
#elif BROWSER_FF
    nsCOMPtr<nsITimer> platform_timer;
#endif
  };

#if BROWSER_IE
  WindowsPlatformTimer *platform_timer_;
  friend class WindowsPlatformTimer;
#elif defined(OS_ANDROID)
  scoped_ptr<AndroidPlatformTimer> platform_timer_;
  friend class AndroidPlatformTimer;
#endif

  // SetTimeout and SetInterval are very similar - the only difference being
  // that the first one is a one-off and the second one repeats.  This function
  // is the common implementation.
  void SetTimeoutOrInterval(JsCallContext *context, bool repeat);

  // Similarly, ClearTimeout and ClearInterval are just two names for the same
  // thing.
  void ClearTimeoutOrInterval(JsCallContext *context);

  // Sets up any structures that aren't needed until the interface is used.
  // It's ok to call this multiple times.
  void Initialize();

  // Creates the timer.  Returns the id of the new timer, or 0 on failure.
  int CreateTimer(const TimerInfo &timer_info, int timeout);

  // Callback for when the timer fires.
  void HandleTimer(TimerInfo *timer_info);

  std::map<int, TimerInfo> timers_;
  int next_timer_id_;
  scoped_ptr<JsEventMonitor> unload_monitor_;

#ifdef BROWSER_WEBKIT
  friend void TimerCallback(CFRunLoopTimerRef ref, void* closure);
#elif BROWSER_FF
  static void TimerCallback(nsITimer *timer, void *closure);
#endif

  DISALLOW_EVIL_CONSTRUCTORS(GearsTimer);
};

// TODO(mpcomplete): remove when we have a cross-platform timer abstraction
#if BROWSER_NPAPI && defined(WIN32)
#undef BROWSER_IE
#endif

#endif  // GEARS_TIMER_TIMER_H__
