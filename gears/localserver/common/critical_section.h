// Copyright 2005, Google Inc.
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
// *** DEPRECATED ***
//
// Use /gears/base/common/mutex.h, which implements the Google interface.
//
// This file remains only for WebCache AsyncTask. The synchronization semantics
// of that class were unclear, making conversion to Mutex too difficult for now.
//
// *** DEPRECATED ***

#ifndef GEARS_LOCALSERVER_COMMON_CRITICAL_SECTION_H__
#define GEARS_LOCALSERVER_COMMON_CRITICAL_SECTION_H__

// TODO(mpcomplete): implement these.
#if BROWSER_NPAPI && defined(WIN32)
#define BROWSER_IE 1
#endif

#if BROWSER_IE
//------------------------------------------------------------------------------
// BROWSER_IE
//------------------------------------------------------------------------------
#include <atlsync.h>

class CriticalSection : protected CCriticalSection {
 public:
  CriticalSection() {}
  void Enter() { CCriticalSection::Enter(); }
  void Leave() { CCriticalSection::Leave(); }
  friend class CritSecLock;
};

class CritSecLock : protected CCritSecLock {
 public:
  CritSecLock(CriticalSection &cs, bool initially_locked = true)
    : CCritSecLock(cs, initially_locked) {}
  void Lock() { CCritSecLock::Lock(); }
  void Unlock() { CCritSecLock::Unlock(); }
};


#elif BROWSER_FF
//------------------------------------------------------------------------------
// BROWSER_FF
//------------------------------------------------------------------------------
#include <assert.h>
#include <gecko_sdk/include/prmon.h>

class CriticalSection {
 public:
  CriticalSection() { monitor_ = PR_NewMonitor(); }
  ~CriticalSection() { if (monitor_) PR_DestroyMonitor(monitor_); }
  void Enter() { PR_EnterMonitor(monitor_); }
  void Leave() { PR_ExitMonitor(monitor_); }
  operator PRMonitor*() { return monitor_; }
 private:
  PRMonitor *monitor_;
  friend class CritSecLock;
};

class CritSecLock {
 public:
  CritSecLock(PRMonitor *monitor, bool initially_locked = true)
    : monitor_(monitor), locked_(false) { if (initially_locked) Lock(); }
  ~CritSecLock() { if (locked_) Unlock(); }
  void Lock() { assert(!locked_); PR_EnterMonitor(monitor_); locked_ = true; }
  void Unlock() { assert(locked_); PR_ExitMonitor(monitor_); locked_ = false; }
 private:
  PRMonitor *monitor_;
  bool locked_;
};


#elif BROWSER_SAFARI || defined(OS_ANDROID)
//------------------------------------------------------------------------------
// BROWSER_SAFARI, OS_ANDROID
//------------------------------------------------------------------------------
#include <pthread.h>

class CriticalSection {
 public:
  CriticalSection() { 
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&monitor_, &attr);
    pthread_mutexattr_destroy(&attr);
  }
  ~CriticalSection() { pthread_mutex_destroy(&monitor_); }
  void Enter() { pthread_mutex_lock(&monitor_); }
  void Leave() { pthread_mutex_unlock(&monitor_); }
 private:
  pthread_mutex_t monitor_;
  friend class CritSecLock;
};

class CritSecLock {
 public:
  CritSecLock(CriticalSection &cs, bool initially_locked = true)
    : cs_(cs), locked_(false) { if (initially_locked) Lock(); }
  ~CritSecLock() { if (locked_) Unlock(); }
  void Lock() { assert(!locked_); cs_.Enter(); locked_ = true; }
  void Unlock() { assert(locked_); cs_.Leave(); locked_ = false; }
 private:
  CriticalSection &cs_;
  bool locked_;
};
#endif

// TODO(mpcomplete): remove
#if BROWSER_NPAPI
#undef BROWSER_IE
#endif

#endif // GEARS_LOCALSERVER_COMMON_CRITICAL_SECTION_H__
