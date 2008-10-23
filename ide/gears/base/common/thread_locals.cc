// Copyright 2006, Google Inc.
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

#include <assert.h>
#include "gears/base/common/thread_locals.h"

#include "gears/base/common/atomic_ops.h"

// TODO(mpcomplete): implement these.
#if BROWSER_NPAPI && defined(WIN32)
#define BROWSER_IE 1
#endif

// TODO(michaeln): figure out if it's safe to initialize this tls_index_ here or
// not. It's set in when DllMain(processAttached) is called, will initializing
// it via the CRT squash that value set via DllMain?
#if BROWSER_IE
#ifdef WINCE
// On Windows Mobile 5 TLS_OUT_OF_INDEXES is undefined.
// On Windows Mobile 6 TLS_OUT_OF_INDEXES is defined to 0xffffffff
// (see http://msdn2.microsoft.com/en-us/library/aa908741.aspx).
const DWORD kNoIndex = 0xffffffff;
#else
const DWORD kNoIndex = TLS_OUT_OF_INDEXES;
#endif
DWORD ThreadLocals::tls_index_;
#elif BROWSER_FF
const PRUintn kNoIndex = 0xffffffff;
PRUintn ThreadLocals::tls_index_;
#elif BROWSER_SAFARI || defined(ANDROID)
pthread_once_t ThreadLocals::tls_index_init_ = PTHREAD_ONCE_INIT;
pthread_key_t ThreadLocals::tls_index_;
#endif

static const int kMaxSlots = 64;
static ThreadLocals::Slot next_slot_;

ThreadLocals::Slot ThreadLocals::Alloc() {
  // Grab a new slot.
  Slot slot = AtomicIncrement(&next_slot_, 1) - 1;
  if (next_slot_ >= kMaxSlots) {
    assert(!"Ran out of thread-local slots! Increase kMaxSlots.");
    return -1;
  }

  return slot;
}

//------------------------------------------------------------------------------
// GetValue
//------------------------------------------------------------------------------
void *ThreadLocals::GetValue(Slot key) {
  if (key < 0 || key >= kMaxSlots) return NULL; 
  Entry *entries = GetEntries(false);
  if (!entries) return NULL;
  if (entries[key].valid_)
    return entries[key].value_;
  else
    return NULL;
}


//------------------------------------------------------------------------------
// HasValue
//------------------------------------------------------------------------------
bool ThreadLocals::HasValue(Slot key) {
  if (key < 0 || key >= kMaxSlots) return false; 
  Entry *entries = GetEntries(false);
  if (!entries) return false;
  return entries[key].valid_;
}


//------------------------------------------------------------------------------
// SetValue
//------------------------------------------------------------------------------
void ThreadLocals::SetValue(Slot key,
                            void *value,
                            DestructorCallback destructor) {
  if (key < 0 || key >= kMaxSlots) return; 
  DestroyValue(key);  // get rid of any existing value for this key
  Entry *entries = GetEntries(true);
  assert(entries);
  if (entries) {
    entries[key] = Entry(value, destructor);
  }
}


//------------------------------------------------------------------------------
// DestroyValue
//------------------------------------------------------------------------------
void ThreadLocals::DestroyValue(Slot key) {
  if (key < 0 || key >= kMaxSlots) return; 
  Entry *entries = GetEntries(false);
  if (entries && entries[key].valid_) {
    if (entries[key].destructor_) {
      entries[key].destructor_(entries[key].value_);
    }
    entries[key].valid_ = false;
  }
}


//------------------------------------------------------------------------------
// GetEntries
//------------------------------------------------------------------------------
ThreadLocals::Entry *ThreadLocals::GetEntries(bool createIfNeeded) {
#if !BROWSER_SAFARI && !defined(ANDROID)
  assert(tls_index_ != kNoIndex);
#endif
  Entry *entries = GetTlsEntries();
  if (createIfNeeded && !entries) {
    entries = new Entry[kMaxSlots];
    SetTlsEntries(entries);
  }
  return entries;
}


//------------------------------------------------------------------------------
// ClearEntries
//------------------------------------------------------------------------------
void ThreadLocals::ClearEntries() {
  Entry *entries = GetTlsEntries();
  if (!entries) return;
  for (int i = 0; i < kMaxSlots; ++i) {
    if (entries[i].valid_ && entries[i].destructor_) {
      entries[i].destructor_(entries[i].value_);
    }
    entries[i].valid_ = false;
  }
}

//------------------------------------------------------------------------------
// DestroyEntries
//------------------------------------------------------------------------------
void ThreadLocals::DestroyEntries(Entry* entries) {
  for (int i = 0; i < kMaxSlots; ++i) {
    if (entries[i].valid_ && entries[i].destructor_) {
      entries[i].destructor_(entries[i].value_);
    }
    entries[i].valid_ = false;
  }
  delete entries;
}


//------------------------------------------------------------------------------
// SetTlsEntries
//------------------------------------------------------------------------------
void ThreadLocals::SetTlsEntries(Entry* entries) {
#if BROWSER_IE
  ::TlsSetValue(tls_index_, entries);
#elif BROWSER_FF
  PR_SetThreadPrivate(tls_index_, entries);
#elif BROWSER_SAFARI || defined(ANDROID)
  pthread_once(&tls_index_init_, ThreadLocals::InitializeKey);
  pthread_setspecific(tls_index_, entries);
#endif
}


//------------------------------------------------------------------------------
// GetTlsEntries
//------------------------------------------------------------------------------
ThreadLocals::Entry* ThreadLocals::GetTlsEntries() {
#if BROWSER_IE
  return reinterpret_cast<Entry*>(TlsGetValue(tls_index_));
#elif BROWSER_FF
  return reinterpret_cast<Entry*>(PR_GetThreadPrivate(tls_index_));
#elif BROWSER_SAFARI || defined(ANDROID)
  pthread_once(&tls_index_init_, ThreadLocals::InitializeKey);
  return reinterpret_cast<Entry*>(pthread_getspecific(tls_index_));
#endif
}


#if BROWSER_IE

//------------------------------------------------------------------------------
// HandleProcessAttached
// Gets a TLS slot from the OS. If we don't get a valid slot, return FALSE to
// indicate to our caller that our DLL should fail to load.
//------------------------------------------------------------------------------
BOOL ThreadLocals::HandleProcessAttached() {
  tls_index_ = TlsAlloc();
  return (tls_index_ != kNoIndex) ? TRUE : FALSE;
}


//------------------------------------------------------------------------------
// HandleProcessDetached
// Frees our TLS slot.
//------------------------------------------------------------------------------
void ThreadLocals::HandleProcessDetached() {
  if (tls_index_ != kNoIndex) {
    HandleThreadDetached();
    TlsFree(tls_index_);
    tls_index_ = kNoIndex;
  }
}


//------------------------------------------------------------------------------
// HandleThreadDetached
// Destroys any values stored for the currently executing thread and deletes the
// per thread entries we've been using for this thread.
//------------------------------------------------------------------------------
void ThreadLocals::HandleThreadDetached() {
  if (tls_index_ != kNoIndex) {
    Entry* entries = GetTlsEntries();
    if (entries) {
      DestroyEntries(entries);
      SetTlsEntries(NULL);
    }
  }
}

#elif BROWSER_FF

//------------------------------------------------------------------------------
// HandleModuleConstructed
// Gets a TLS slot from the OS. If we don't get a valid slot, return an error to
// indicate to our caller that our module should fail to load.
//------------------------------------------------------------------------------
nsresult ThreadLocals::HandleModuleConstructed() {
  PRStatus status = PR_NewThreadPrivateIndex(&tls_index_, TlsDestructor);
  if (status == PR_SUCCESS)
    return NS_OK;
  else
    return NS_ERROR_FAILURE;
}

//------------------------------------------------------------------------------
// TlsDestructor
//------------------------------------------------------------------------------
void PR_CALLBACK ThreadLocals::TlsDestructor(void *priv) {
  if (priv)
    DestroyEntries(reinterpret_cast<Entry*>(priv));
}

#elif BROWSER_SAFARI || defined(ANDROID)

void ThreadLocals::InitializeKey() {
  pthread_key_create(&tls_index_, FinalizeKey);
}

// This destructor will be called for each key when the thread is terminating.
// For this class, it should be called only once.
void ThreadLocals::FinalizeKey(void *entries) {
  DestroyEntries(reinterpret_cast<Entry*>(entries));
}

#endif
