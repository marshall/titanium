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

#include "gears/timer/timer.h"

#if BROWSER_FF
#if BROWSER_FF2
#include <gecko_internal/nsITimerInternal.h>
#endif
#endif

#include "gears/base/common/dispatcher.h"
#include "gears/base/common/js_types.h"
#include "gears/base/common/module_wrapper.h"

// TODO(mpcomplete): remove when we have a cross-platform timer abstraction
#if BROWSER_NPAPI && defined(WIN32)
#define BROWSER_IE 1
#endif

#if BROWSER_IE
WindowsPlatformTimer::WindowsPlatformTimer(GearsTimer *gears_timer)
    : gears_timer_(gears_timer),
      in_handler_(false) {
}

WindowsPlatformTimer::~WindowsPlatformTimer() {
}

void WindowsPlatformTimer::OnFinalMessage(HWND hwnd) {
  delete this;
}

LRESULT WindowsPlatformTimer::OnTimer(UINT msg,
                                      WPARAM timer_id,
                                      LPARAM unused_param,
                                      BOOL& handled) {
  handled = TRUE;

  // Prevent re-entry.
  if (in_handler_) {
    return 0;
  }
  in_handler_ = true;
  // Hold an extra reference to protect against deletion while running the
  // timer handler.
  scoped_refptr<GearsTimer> hold(gears_timer_);

  std::map<int, GearsTimer::TimerInfo>::iterator timer =
      gears_timer_->timers_.find(timer_id);
  if (timer == gears_timer_->timers_.end()) {
    // This can happen if the event has already been posted, but the timer was
    // deleted.
    KillTimer(timer_id);
  } else {
    gears_timer_->HandleTimer(&(timer->second));
  }

  in_handler_ = false;
  return 0;
}

void WindowsPlatformTimer::Initialize() {
  // Make sure we have an HWND
  if (!IsWindow()) {
    if (!Create(kMessageOnlyWindowParent,    // parent
                NULL,                        // position
                NULL,                        // name
                kMessageOnlyWindowStyle)) {  // style
      assert(false);
    }
  }
}

void WindowsPlatformTimer::CancelGearsTimer(int timer_id) {
  if (IsWindow() && timer_id != 0) {
    KillTimer(timer_id);
  }
}
#endif

#if defined(OS_ANDROID)
// Functor used to marshal a timeout to the thread which initiated it.
class AndroidTimeoutFunctor : public AsyncFunctor {
 public:
  AndroidTimeoutFunctor(AndroidPlatformTimer *platform_timer,
                        TimedCallback *caller,
                        int timer_id)
      : platform_timer_(platform_timer),
        caller_(caller),
        timer_id_(timer_id) { }
  virtual void Run() { platform_timer_->OnMessage(caller_, timer_id_); }

 private:
  AndroidPlatformTimer *platform_timer_;
  TimedCallback *caller_;
  int timer_id_;
};

AndroidPlatformTimer::AndroidPlatformTimer(GearsTimer *gears_timer)
    : gears_timer_(gears_timer),
      owner_id_(ThreadMessageQueue::GetInstance()->GetCurrentThreadId()),
      mutex_(),
      timer_map_() {
  LOG(("%p AndroidPlatformTimer constructed\n", this));
}

AndroidPlatformTimer::~AndroidPlatformTimer() {
  LOG(("%p AndroidPlatformTimer deleted\n", this));
  MutexLock lock(&mutex_);
  assert(timer_map_.empty());
}

void AndroidPlatformTimer::AddTimer(int timer_id) {
  LOG(("%p Starting platform timer %d\n", this, timer_id));
  ASSERT_SINGLE_THREAD();
  std::map<int, GearsTimer::TimerInfo>::iterator it =
      gears_timer_->timers_.find(timer_id);
  assert(it != gears_timer_->timers_.end());
  GearsTimer::TimerInfo *timer_info = &it->second;
  assert(timer_map_.find(timer_id) == timer_map_.end());
  int timeout = timer_info->timeout;
  if (timeout <= 0) {
    // Minimum TimedCallback timeout is 1 millisecond.
    timeout = 1;
  }
  // Increment refcount while the timer is running.
  gears_timer_->Ref();
  {
    MutexLock lock(&mutex_);
    TimedCallback *callback = new TimedCallback(
        this,
        timeout,
        reinterpret_cast<void *>(timer_id));
    timer_map_.insert(std::make_pair(timer_id, callback));
  }
}

void AndroidPlatformTimer::RemoveTimer(int timer_id) {
  LOG(("%p Deleting platform timer %d\n", this, timer_id));
  ASSERT_SINGLE_THREAD();
  // Remove the timer from the list.
  TimedCallback *removed;
  {
    MutexLock lock(&mutex_);
    TimerMap::iterator it = timer_map_.find(timer_id);
    if (it == timer_map_.end()) {
      LOG(("%p Already removed %d\n", this, timer_id));
      removed = NULL;
      // Timer may now be in the message queue.
    } else {
      // Remove the timer from the map, but don't delete with the
      // mutex held.
      removed = it->second;
      timer_map_.erase(it);
      // We know the timer will NOT fire now.
    }
  }
  if (removed) {
    // Must delete the thread without holding the mutex or we'll
    // deadlock.
    delete removed;
    // Drop the refcount now that this timer is gone.
    gears_timer_->Unref();
  }
}

void AndroidPlatformTimer::OnTimeout(TimedCallback *caller, void *user_data) {
  assert(caller != NULL);
  int timer_id = reinterpret_cast<int>(user_data);
  // Remove ourselvse from the map.
  {
    MutexLock lock(&mutex_);
    TimerMap::iterator it = timer_map_.find(timer_id);
    if (it == timer_map_.end()) {
      // We fired in the short space of time between removing from the
      // map and deletion. Don't post a message.
      LOG(("%p Removed timer %d just in time!\n", this, timer_id));
      return;
    }
    LOG(("%p OnTimeout removed %d\n", this, timer_id));
    timer_map_.erase(it);
  }
  AndroidTimeoutFunctor *functor =
      new AndroidTimeoutFunctor(this, caller, timer_id);
  AsyncRouter::GetInstance()->CallAsync(owner_id_, functor);
}

void AndroidPlatformTimer::OnMessage(TimedCallback *caller, int timer_id) {
  LOG(("%p OnMessage %d\n", this, timer_id));
  ASSERT_SINGLE_THREAD();
  // Delete the thread - it's done now.
  delete caller;
  // Still holding a refcount on gears_timer_.
  std::map<int, GearsTimer::TimerInfo>::iterator it =
      gears_timer_->timers_.find(timer_id);
  if (it == gears_timer_->timers_.end()) {
    LOG(("%p Stale message %d\n", this, timer_id));
    gears_timer_->Unref();
    return;
  }
  GearsTimer::TimerInfo *timer_info = &it->second;
  // Pass the message along.
  bool repeat = timer_info->repeat;
  gears_timer_->HandleTimer(timer_info);
  if (repeat) {
    // It's on repeat, spawn another timer.
    if (gears_timer_->timers_.find(timer_id) ==
        gears_timer_->timers_.end()) {
      LOG(("%p Repeating timer %d was deleted in callback\n", this, timer_id));
      gears_timer_->Unref();
    } else {
      LOG(("%p Repeating timer %d\n", this, timer_id));
      AddTimer(timer_id);
    }
  } else {
    LOG(("%p Single-shot\n", this));
    gears_timer_->Unref();
  }
}
#endif // defined(OS_ANDROID)

// Disables the timer when the TimerInfo is deleted.
GearsTimer::TimerInfo::~TimerInfo() {
#if BROWSER_FF
  if (platform_timer) {
    platform_timer->Cancel();
  }
#endif
  if (owner) {
#if BROWSER_IE
    owner->platform_timer_->CancelGearsTimer(timer_id);
#elif defined(OS_ANDROID)
    owner->platform_timer_->RemoveTimer(timer_id);
#endif
  }
}



DECLARE_GEARS_WRAPPER(GearsTimer);

template<>
void Dispatcher<GearsTimer>::Init() {
  RegisterMethod("clearInterval", &GearsTimer::ClearInterval);
  RegisterMethod("clearTimeout", &GearsTimer::ClearTimeout);
  RegisterMethod("setInterval", &GearsTimer::SetInterval);
  RegisterMethod("setTimeout", &GearsTimer::SetTimeout);
}

void GearsTimer::SetTimeout(JsCallContext *context) {
  SetTimeoutOrInterval(context, false);
}

void GearsTimer::ClearTimeout(JsCallContext *context) {
  ClearTimeoutOrInterval(context);
}

void GearsTimer::SetInterval(JsCallContext *context) {
  SetTimeoutOrInterval(context, true);
}

void GearsTimer::ClearInterval(JsCallContext *context) {
  ClearTimeoutOrInterval(context);
}

void GearsTimer::SetTimeoutOrInterval(JsCallContext *context, bool repeat) {
  int timeout;

  std::string16 script;
  JsRootedCallback *timer_callback = NULL;

  const int argc = 2;
  JsArgument argv[argc] = {
    { JSPARAM_REQUIRED, JSPARAM_UNKNOWN, NULL },
    { JSPARAM_REQUIRED, JSPARAM_INT, &timeout },
  };

  int timer_code_type = context->GetArgumentType(0);
  if (timer_code_type == JSPARAM_FUNCTION) {
    argv[0].type = JSPARAM_FUNCTION;
    argv[0].value_ptr = &timer_callback;
  } else if (timer_code_type == JSPARAM_STRING16) {
    argv[0].type = JSPARAM_STRING16;
    argv[0].value_ptr = &script;
  } else {
    context->SetException(
        STRING16(L"First parameter must be a function or string."));
    return;
  }

  context->GetArguments(argc, argv);
  scoped_ptr<JsRootedCallback> scoped_callback(timer_callback);
  if (context->is_exception_set()) return;

  TimerInfo timer_info;
  timer_info.repeat = repeat;
  if (timer_callback) {
    timer_info.callback.reset(scoped_callback.release());  // transfer ownership
  } else {
    timer_info.script = script;
  }

  int result = CreateTimer(timer_info, timeout);
  if (result == 0) {
    context->SetException(STRING16(L"Timer creation failed."));
    return;
  }

  context->SetReturnValue(JSPARAM_INT, &result);
}

void GearsTimer::ClearTimeoutOrInterval(JsCallContext *context) {
  int timer_id;

  const int argc = 1;
  JsArgument argv[argc] = {
    { JSPARAM_REQUIRED, JSPARAM_INT, &timer_id },
  };

  context->GetArguments(argc, argv);
  if (context->is_exception_set()) return;

  timers_.erase(timer_id);
}

// Makes sure the object's structures are initialized.  We need to set up the
// unload monitor for the web page.
void GearsTimer::Initialize() {
#if BROWSER_IE
  platform_timer_->Initialize();
#endif

  // Create an event monitor to remove remaining timers when the page
  // unloads.
  if (unload_monitor_ == NULL) {
    unload_monitor_.reset(new JsEventMonitor(GetJsRunner(), JSEVENT_UNLOAD,
                                             this));
  }
}

// Creates the platform's timer object, perform all common initialization of the
// TimerInfo structure, and store the TimerInfo in the map.
int GearsTimer::CreateTimer(const TimerInfo &timer_info, int timeout) {
  Initialize();

  // Store the timer info
  int timer_id = ++next_timer_id_;

  // Add the timer to the map.
  timers_[timer_id] = timer_info;
  TimerInfo *timer = &timers_[timer_id];
  timer->timer_id = timer_id;
  timer->timeout = timeout;
  timer->SetOwner(this);

  // Create the actual timer.
#if BROWSER_WEBKIT
  // Create the actual timer.
  CFRunLoopTimerContext context;
  memset(&context, 0, sizeof(CFRunLoopTimerContext));
  context.info = static_cast<void *>(timer);
  
  CFAbsoluteTime fireDate = CFAbsoluteTimeGetCurrent() + (1.0e-3 * timeout);
  CFTimeInterval interval = 0;
  
  if (timer_info.repeat) {
    interval = 1.0e-3 * timeout;
  }
  
  CFRunLoopTimerRef tmp_timer = CFRunLoopTimerCreate(0, fireDate,
                                                     interval, 0, 0, 
                                                     TimerCallback, 
                                                     &context);
  if (!tmp_timer) {
    timers_.erase(timer_id);
    return 0;
  }
  
  CFRunLoopAddTimer(CFRunLoopGetCurrent(), tmp_timer,
                    kCFRunLoopCommonModes);
  
  timer->platform_timer.reset(new TimerInfo::scoped_timer(tmp_timer));
#elif BROWSER_FF
  nsresult result;
  timer->platform_timer =
      do_CreateInstance("@mozilla.org/timer;1", &result);

  if (NS_FAILED(result)) {
    timers_.erase(timer_id);
    return 0;
  }

#if BROWSER_FF2          // FIXME: kimmo.t.kinnunen@nokia.com: Timers in gecko 1.9 are always in this thread by default?
  // Turning off idle causes the callback to be invoked in this thread,
  // instead of in the Timer idle thread.
  nsCOMPtr<nsITimerInternal> timer_internal(
      do_QueryInterface(timer->platform_timer));
  timer_internal->SetIdle(false);
#endif

  // Cast because the two constants are defined in different anonymous
  // enums, so they aren't literally of the same type, which throws a
  // warning on gcc.
  PRUint32 type = timer->repeat
      ? static_cast<PRUint32>(nsITimer::TYPE_REPEATING_SLACK)
      : static_cast<PRUint32>(nsITimer::TYPE_ONE_SHOT);

  // Start the timer
  timer->platform_timer->InitWithFuncCallback(
      TimerCallback, timer, timeout, type);
#elif BROWSER_IE
  if (0 == platform_timer_->SetTimer(timer_id, timeout, NULL)) {
    timers_.erase(timer_id);
    return 0;
  }
#elif defined(OS_ANDROID)
  platform_timer_->AddTimer(timer_id);
#endif

  return timer_id;
}

// Handles the page being unloaded.  Clean up any remaining active timers.
void GearsTimer::HandleEvent(JsEventType event_type) {
  assert(event_type == JSEVENT_UNLOAD);

  // Hold an extra reference to protect against deletion while clearing the map,
  // which contains references to us.
  scoped_refptr<GearsTimer> hold(this);
  timers_.clear();
}


// Perform the non-platform specific work that occurs when a timer fires.
void GearsTimer::HandleTimer(TimerInfo *timer_info) {
  // Store the information required to clean up the timer, in case it gets
  // deleted in the handler.
  bool repeat = timer_info->repeat;
  int timer_id = timer_info->timer_id;

  // Invoke JavaScript timer handler.  *timer_info can become invalid here, if
  // the timer gets deleted in the handler.
  if (timer_info->callback.get()) {
    GetJsRunner()->InvokeCallback(timer_info->callback.get(), 0, NULL, NULL);
  } else {
    GetJsRunner()->Eval(timer_info->script);
  }

  // If this is a one shot timer, we're done with the timer object.
  if (!repeat) {
    timers_.erase(timer_id);
  }
}

// Perform the platform specific work when a timer fires.
#ifdef BROWSER_WEBKIT
void TimerCallback(CFRunLoopTimerRef ref, void* closure)
{
  GearsTimer::TimerInfo *timer_info = 
                             static_cast<GearsTimer::TimerInfo *>(closure);
  
  // Hold an extra reference to protect against deletion while running the
  // timer handler.
  scoped_refptr<GearsTimer> hold(timer_info->owner);
  timer_info->owner->HandleTimer(timer_info);
}
#elif BROWSER_FF
void GearsTimer::TimerCallback(nsITimer *timer, void *closure) {
  TimerInfo *timer_info = reinterpret_cast<TimerInfo *>(closure);

  // Hold an extra reference to protect against deletion while running the
  // timer handler.
  scoped_refptr<GearsTimer> hold(timer_info->owner);
  timer_info->owner->HandleTimer(timer_info);
}
#endif
