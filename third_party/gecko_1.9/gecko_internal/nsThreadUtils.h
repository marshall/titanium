/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Mozilla code.
 *
 * The Initial Developer of the Original Code is Google Inc.
 * Portions created by the Initial Developer are Copyright (C) 2006
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *  Darin Fisher <darin@meer.net>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef nsThreadUtils_h__
#define nsThreadUtils_h__

#include "prthread.h"
#include "prinrval.h"
#include "nsIThreadManager.h"
#include "nsIThread.h"
#include "nsIRunnable.h"
#include "nsStringGlue.h"
#include "nsCOMPtr.h"

// This is needed on some systems to prevent collisions between the symbols
// appearing in xpcom_core and xpcomglue.  It may be unnecessary in the future
// with better toolchain support.
#ifdef MOZILLA_INTERNAL_API
# define NS_NewThread NS_NewThread_P
# define NS_GetCurrentThread NS_GetCurrentThread_P
# define NS_GetMainThread NS_GetMainThread_P
# define NS_IsMainThread NS_IsMainThread_P
# define NS_DispatchToCurrentThread NS_DispatchToCurrentThread_P
# define NS_DispatchToMainThread NS_DispatchToMainThread_P
# define NS_ProcessPendingEvents NS_ProcessPendingEvents_P
# define NS_HasPendingEvents NS_HasPendingEvents_P
# define NS_ProcessNextEvent NS_ProcessNextEvent_P
#endif

//-----------------------------------------------------------------------------
// These methods are alternatives to the methods on nsIThreadManager, provided
// for convenience.

/**
 * Create a new thread, and optionally provide an initial event for the thread.
 *
 * @param result
 *   The resulting nsIThread object.
 * @param initialEvent
 *   The initial event to run on this thread.  This parameter may be null.
 *
 * @returns NS_ERROR_INVALID_ARG
 *   Indicates that the given name is not unique.
 */
extern NS_COM_GLUE NS_METHOD
NS_NewThread(nsIThread **result, nsIRunnable *initialEvent = nsnull);

/**
 * Get a reference to the current thread.
 *
 * @param result
 *   The resulting nsIThread object.
 */
extern NS_COM_GLUE NS_METHOD
NS_GetCurrentThread(nsIThread **result);

/**
 * Get a reference to the main thread.
 *
 * @param result
 *   The resulting nsIThread object.
 */
extern NS_COM_GLUE NS_METHOD
NS_GetMainThread(nsIThread **result);

/**
 * Test to see if the current thread is the main thread.
 *
 * @returns PR_TRUE if the current thread is the main thread, and PR_FALSE
 * otherwise.
 */
extern NS_COM_GLUE NS_METHOD_(PRBool)
NS_IsMainThread();

/**
 * Dispatch the given event to the current thread.
 *
 * @param event
 *   The event to dispatch.
 *
 * @returns NS_ERROR_INVALID_ARG
 *   If event is null.
 */
extern NS_COM_GLUE NS_METHOD
NS_DispatchToCurrentThread(nsIRunnable *event);

/**
 * Dispatch the given event to the main thread.
 *
 * @param event
 *   The event to dispatch.
 * @param dispatchFlags
 *   The flags to pass to the main thread's dispatch method.
 *
 * @returns NS_ERROR_INVALID_ARG
 *   If event is null.
 */
extern NS_COM_GLUE NS_METHOD
NS_DispatchToMainThread(nsIRunnable *event,
                        PRUint32 dispatchFlags = NS_DISPATCH_NORMAL);

#ifndef XPCOM_GLUE_AVOID_NSPR
/**
 * Process all pending events for the given thread before returning.  This
 * method simply calls ProcessNextEvent on the thread while HasPendingEvents
 * continues to return true and the time spent in NS_ProcessPendingEvents
 * does not exceed the given timeout value.
 *
 * @param thread
 *   The thread object for which to process pending events.  If null, then
 *   events will be processed for the current thread.
 * @param timeout
 *   The maximum number of milliseconds to spend processing pending events.
 *   Events are not pre-empted to honor this timeout.  Rather, the timeout
 *   value is simply used to determine whether or not to process another event.
 *   Pass PR_INTERVAL_NO_TIMEOUT to specify no timeout.
 */
extern NS_COM_GLUE NS_METHOD
NS_ProcessPendingEvents(nsIThread *thread,
                        PRIntervalTime timeout = PR_INTERVAL_NO_TIMEOUT);
#endif

/**
 * Shortcut for nsIThread::HasPendingEvents.
 *
 * It is an error to call this function when the given thread is not the
 * current thread.  This function will return PR_FALSE if called from some
 * other thread.
 *
 * @param thread
 *   The current thread or null.
 *
 * @returns
 *   A boolean value that if "true" indicates that there are pending events
 *   in the current thread's event queue.
 */
extern NS_COM_GLUE PRBool
NS_HasPendingEvents(nsIThread *thread = nsnull);

/**
 * Shortcut for nsIThread::ProcessNextEvent.
 *   
 * It is an error to call this function when the given thread is not the
 * current thread.  This function will simply return PR_FALSE if called
 * from some other thread.
 *
 * @param thread
 *   The current thread or null.
 * @param mayWait
 *   A boolean parameter that if "true" indicates that the method may block
 *   the calling thread to wait for a pending event.
 *
 * @returns
 *   A boolean value that if "true" indicates that an event from the current
 *   thread's event queue was processed.
 */
extern NS_COM_GLUE PRBool
NS_ProcessNextEvent(nsIThread *thread = nsnull, PRBool mayWait = PR_TRUE);

//-----------------------------------------------------------------------------
// Helpers that work with nsCOMPtr:

inline already_AddRefed<nsIThread>
do_GetCurrentThread() {
  nsIThread *thread = nsnull;
  NS_GetCurrentThread(&thread);
  return already_AddRefed<nsIThread>(thread);
}

inline already_AddRefed<nsIThread>
do_GetMainThread() {
  nsIThread *thread = nsnull;
  NS_GetMainThread(&thread);
  return already_AddRefed<nsIThread>(thread);
}

//-----------------------------------------------------------------------------

#ifdef MOZILLA_INTERNAL_API
// Fast access to the current thread.  Do not release the returned pointer!  If
// you want to use this pointer from some other thread, then you will need to
// AddRef it.  Otherwise, you should only consider this pointer valid from code
// running on the current thread.
extern NS_COM_GLUE nsIThread *NS_GetCurrentThread();
#endif

//-----------------------------------------------------------------------------

#ifndef XPCOM_GLUE_AVOID_NSPR

#undef  IMETHOD_VISIBILITY
#define IMETHOD_VISIBILITY NS_COM_GLUE

// This class is designed to be subclassed.
class NS_COM_GLUE nsRunnable : public nsIRunnable
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIRUNNABLE

  nsRunnable() {
  }

protected:
  virtual ~nsRunnable() {
  }
};

#undef  IMETHOD_VISIBILITY
#define IMETHOD_VISIBILITY NS_VISIBILITY_HIDDEN

// An event that can be used to call a method on a class.  The class type must
// support reference counting.
template <class T>
class nsRunnableMethod : public nsRunnable
{
public:
  typedef void (T::*Method)();

  nsRunnableMethod(T *obj, Method method)
    : mObj(obj), mMethod(method) {
    NS_ADDREF(mObj);
  }

  NS_IMETHOD Run() {
    (mObj->*mMethod)();
    return NS_OK;
  }

private:
  virtual ~nsRunnableMethod() {
    NS_RELEASE(mObj);
  }

  T      *mObj;
  Method  mMethod;
};

// Use this helper macro like so:
//
//   nsCOMPtr<nsIRunnable> event =
//       NS_NEW_RUNNABLE_METHOD(MyClass, myObject, HandleEvent);
//   NS_DispatchToCurrentThread(event);
//
// Constraints:
//  - myObject must be of type MyClass
//  - MyClass must defined AddRef and Release methods
//
// NOTE: Attempts to make this a template function caused VC6 to barf :-(
//
#define NS_NEW_RUNNABLE_METHOD(class_, obj_, method_) \
    new nsRunnableMethod<class_>(obj_, &class_::method_)

#endif  // XPCOM_GLUE_AVOID_NSPR

// This class is designed to be used when you have an event class E that has a
// pointer back to resource class R.  If R goes away while E is still pending,
// then it is important to "revoke" E so that it does not try use R after R has
// been destroyed.  nsRevocableEventPtr makes it easy for R to manage such
// situations:
//
//   class R;
//
//   class E : public nsRunnable {
//   public:
//     void Revoke() {
//       mResource = nsnull;
//     }
//   private:
//     R *mResource;
//   };
//
//   class R {
//   public:
//     void EventHandled() {
//       mEvent.Forget();
//     }
//   private:
//     nsRevocableEventPtr<E> mEvent;
//   };
//
//   void R::PostEvent() {
//     // Make sure any pending event is revoked.
//     mEvent->Revoke();
//
//     nsCOMPtr<nsIRunnable> event = new E();
//     if (NS_SUCCEEDED(NS_DispatchToCurrentThread(event))) {
//       // Keep pointer to event so we can revoke it.
//       mEvent = event;
//     }
//   }
//
//   NS_IMETHODIMP E::Run() {
//     if (!mResource)
//       return NS_OK;
//     ...
//     mResource->EventHandled();
//     return NS_OK;
//   }
//
template <class T>
class nsRevocableEventPtr {
public:
  nsRevocableEventPtr()
    : mEvent(nsnull) {
  }

  ~nsRevocableEventPtr() {
    Revoke();
  }

  const nsRevocableEventPtr& operator=(T *event) {
    Revoke();
    mEvent = event;
    return *this;
  }

  void Revoke() {
    if (mEvent) {
      mEvent->Revoke();
      mEvent = nsnull;
    }
  }

  void Forget() {
    mEvent = nsnull;
  }

  PRBool IsPending() {
    return mEvent != nsnull;
  }

private:
  // Not implemented
  nsRevocableEventPtr(const nsRevocableEventPtr&);
  nsRevocableEventPtr& operator=(const nsRevocableEventPtr&);

  T *mEvent;
};

#endif  // nsThreadUtils_h__
