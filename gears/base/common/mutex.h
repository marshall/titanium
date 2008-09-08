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

#ifndef GEARS_BASE_COMMON_MUTEX_H__
#define GEARS_BASE_COMMON_MUTEX_H__

#include "gears/base/common/basictypes.h" // for DISALLOW_EVIL_CONSTRUCTORS
#include "gears/base/common/common.h"
#if defined(WIN32) || defined(WINCE)
#include "gears/base/common/scoped_refptr.h"
#endif  // defined(WIN32) || defined(WINCE)

class Condition;

// TODO(bgarcia): Replace this function with something more appropriate.
//                Hopefully Thread::Yield() from gears/base/common/thread.h,
//                once it is created.
extern void ThreadYield();

// A Mutex is a non-reentrant (aka non-recursive) mutex.  At most
// one thread T may hold a mutex at a given time.
// See also MutexLock, below, for scoped Mutex acquisition.

class Mutex {
 public:
  Mutex();
  ~Mutex();

  void Lock();
  void Unlock();

  // Until the Condition is true, releases and reacquires this Mutex.
  // The thread must already hold this Mutex when calling Await().
  // Performance matches that of native Events (Win32) or CondVars (Firefox).
  //
  // A Condition "cond" used with this Mutex may be invoked an arbitrary
  // number of times, by any thread using the Mutex.  "cond" will always
  // be invoked with the Mutex held by the thread that calls it, so it
  // should not block for long periods or sleep on a timer.  "cond" must return
  // a boolean that is a function of state that is protected by the Mutex. Thus,
  // if "cond" returns true just before this Mutex is released, it must also
  // return true just after this Mutex is reacquired.  "cond" MUST NOT BE A
  // FUNCTION OF THE TIME, or of any other state not protected by the Mutex!
  void Await(const Condition &cond);

  // As Await, but will unblock once the specified time interval has expired, if
  // the condition has not become true by that time. Note that there is no
  // guarantee that the thread will be unblocked immediately once the specified
  // interval has elapsed. Return value indicates whether the condition became
  // true.
  bool AwaitWithTimeout(const Condition &cond, int timeout_milliseconds);

 private:
  // Implementation for Await and AwaitWithTimeout.
  bool AwaitImpl(const Condition &cond, int64 end_time);
  friend class CondVar;
#ifdef DEBUG
#if defined(WIN32) || defined(WINCE)
  // Track whether the mutex is locked to detect recursive usage.
  bool is_locked_;
#elif defined(LINUX) || defined(OS_MACOSX) || defined(OS_ANDROID)
  // Track the owner of the mutex to detect recursive usage.
  // TODO(jripley): Some patterns of pthread_t may not work if it is
  // 64 bit and the system does not perform atomic 64 bit operations.
  pthread_t owner_;
#endif
#endif // DEBUG

#if defined(WIN32) || defined(WINCE)
  CRITICAL_SECTION crit_sec_;
#elif defined(LINUX) || defined(OS_MACOSX) || defined(OS_ANDROID)
  pthread_mutex_t mutex_;
#endif
};


// ----------------------------------------------------------------------------


// MutexLock(mu) acquires mu when constructed and releases it when destroyed.

class MutexLock {
 public:
  explicit MutexLock(Mutex *mu) : mu_(mu) { this->mu_->Lock(); }
  ~MutexLock() { this->mu_->Unlock(); }
 private:
  Mutex *const mu_;
  DISALLOW_EVIL_CONSTRUCTORS(MutexLock);
};
// Catch bug where variable name is omitted, e.g. MutexLock (&mu);
#define MutexLock(x) COMPILE_ASSERT(0, mutex_lock_decl_missing_var_name)


// ----------------------------------------------------------------------------


// Implements a subset of the Google 'Condition' class.
//
// Any variables referenced to evaluate this Condition should be protected by
// the Mutex receiving this Condition.
//
// Various calls in Mutex take a Condition as argument;
// clients can wait for conditions to become true.
// Functions passed to the constructors should be pure functions;
// their results should depend only on the arguments and
// they should have no side-effects.

class Condition {
 public:
  // A Condition that returns the value of *cond.
  explicit Condition(bool *cond)
      : function_(reinterpret_cast<InternalFunctionType>(Dereference)),
        method_(NULL),
        arg_(cond) {}

  // Templated versions for invoking a method that returns a bool.
  // Usage:   Condition(object, &Class::Method)
  // Condition::Eval() returns the result of object->Method()
  template<typename T>
  Condition(T *object, bool (T::*method)());
  // Same as above, for const members
  template<typename T>
  Condition(const T *object, bool (T::*method)() const);

  // Evaluates the condition
  bool Eval() const {
    if (method_) {
      InternalMethodCallerType caller =
          reinterpret_cast<InternalMethodCallerType>(function_);
      return (*caller)(arg_, method_);
    }
    return function_ == 0 || (*function_)(arg_);
  }

 private:
  typedef bool (*InternalFunctionType)(void *);
  typedef bool (Condition::*InternalMethodType)();
  typedef bool (*InternalMethodCallerType)(void *, InternalMethodType);

  static bool Dereference(bool *arg) { return *arg; }

  template<typename T>
  static bool ConditionMethodCaller(void *v, bool (Condition::*m)());

  InternalFunctionType function_; // function taking void * returning bool
  InternalMethodType method_;     // method returning bool
  void *arg_;
};

// static
template<typename T>
bool Condition::ConditionMethodCaller(void *v, bool (Condition::*m)()) {
  typedef bool (T::*RealMethodType)();
  T *x = static_cast<T *>(v);
  RealMethodType rm = reinterpret_cast<RealMethodType>(m);
  return (x->*rm)();
}

template<typename T>
inline Condition::Condition(T *object, bool (T::*method)()) :
    method_(reinterpret_cast<InternalMethodType>(method)), arg_(object) {
  InternalMethodCallerType caller = &ConditionMethodCaller<T>;
  this->function_ = reinterpret_cast<InternalFunctionType>(caller);
}

template<typename T>
inline Condition::Condition(const T *object, bool (T::*method)() const) :
    method_((InternalMethodType) method), arg_((void *) object) {
  InternalMethodCallerType caller = &ConditionMethodCaller<T>;
  this->function_ = reinterpret_cast<InternalFunctionType>(caller);
}

// Implements a subset of the Google 'CondVar' class.
//
// The Mutex::Await() implementation does a busy-wait, consuming CPU resources
// until the condition is true.  This is fine for conditions that aren't
// expected to take long to be satisfied, but for more generic cases, use
// CondVar instead.
//
// Usage for a thread waiting for some condition C protected by mutex mu:
//       mu.Lock();
//       while (!C) { cv->Wait(&mu); }        // releases and reacquires mu
//       //  C holds; process data
//       mu.Unlock();
//
// Usage to wake T is:
//       mu.Lock();
//      // process data, possibly establishing C
//      if (C) { cv->Signal(); }
//      mu.Unlock();
// If C may be useful to more than one waiter, use SignalAll()
// instead of Signal().
//
class CondVar {
 public:
  CondVar();
  ~CondVar();

  // Atomically release *mutex and block on this condition variable.
  // The thread unblocks, reacquires the *mutex and returns if after blocking,
  // any of:
  //  - this condition variable is signalled with SignalAll(), or
  //  - this condition variable is signalled in any manner and this thread
  //    was chosen to be awakened.
  //  - this call was interrupted by a signal.
  void Wait(Mutex *mutex);

  // Atomically release *mutex and block on this condition variable, with
  // a timeout.
  // The thread unblocks, reacquires the *mutex and returns if after blocking,
  // any of:
  //  - this condition variable is signalled with SignalAll(), or
  //  - "milliseconds" milliseconds have elapsed, or
  //  - this condition variable is signalled in any manner and this thread
  //    was the most recently blocked thread that has not yet woken.
  //  - this call was interrupted by a signal.
  // Returns false if signalled, true if timed out.
  // If both conditions are true, it can return either true or false.
  bool WaitWithTimeout(Mutex *mutex, int milliseconds);

  // Signal this CondVar to awaken all waiters.
  // It is more efficient (but not necessary) to call SignalAll after
  // unlocking the mutex.
  void SignalAll();

 private:
#if defined(WIN32) || defined(WINCE)
  class Event;  // This is a win32 event with reference counting added.
  Mutex current_event_mutex_;
  scoped_refptr<Event> current_event_;
#elif defined(LINUX) || defined(OS_MACOSX) || defined(OS_ANDROID)
  pthread_cond_t cond_;
#endif
};

#endif // GEARS_BASE_COMMON_MUTEX_H__
