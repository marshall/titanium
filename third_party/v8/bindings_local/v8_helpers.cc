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

#include <hash_map>

#include "v8_helpers.h"

#include "np_v8object.h"

#include "gears/base/common/async_router.h"
#include "gears/base/common/mutex.h"
#include "gears/base/common/thread_locals.h"

// V8 lock counters.  These should only be accessed when the current thread has
// the V8 lock.
static int g_v8_counter = 0;
static int g_v8_locker_count = 0;
static int g_v8_unlocker_count = 0;

V8Locker::V8Locker() {
  ASSERT(g_v8_counter == 0);
  ++g_v8_counter;
  ++g_v8_locker_count;
}

V8Locker::~V8Locker() {
  --g_v8_counter;
  --g_v8_locker_count;
  ASSERT(g_v8_counter == 0);
}

V8Unlocker::V8Unlocker() {
  --g_v8_counter;
  ++g_v8_unlocker_count;
  // Try to test if we're unlocking without owning the lock.  This really only
  // tests that *some thread* owns the lock, but it should be good enough for
  // testing purposes.
  ASSERT(g_v8_counter == 0);
  unlocker_ = new v8::Unlocker;  // we don't own the lock after this point
}

V8Unlocker::~V8Unlocker() {
  delete unlocker_;  // we now own the v8 lock again
  ASSERT(g_v8_counter == 0);
  ++g_v8_counter;
  --g_v8_unlocker_count;
}

// static
v8::Handle<v8::Value> V8Proxy::WrapCPointer(void *cptr) {
  // Represent void* as int
  int addr = reinterpret_cast<int>(cptr);
  if ((addr & 0x01) == 0) {
    return v8::Number::New(addr >> 1);
  } else {
    return v8::External::New(cptr);
  }
}

// static
void *V8Proxy::ExtractCPointerImpl(v8::Handle<v8::Value> obj) {
  if (obj->IsNumber()) {
    int addr = obj->Int32Value();
    return reinterpret_cast<void*>(addr << 1);
  } else if (obj->IsExternal()) {
    return v8::Handle<v8::External>::Cast(obj)->Value();
  }
  ASSERT(false);
  return 0;
}

void *V8Proxy::ToNativeObjectImpl(V8ClassIndex::V8WrapperType type,
                                  v8::Handle<v8::Value> value) {
  ASSERT(type == V8ClassIndex::NPOBJECT);
  ASSERT(value->IsObject());

  v8::Handle<v8::Object> obj = v8::Handle<v8::Object>::Cast(value);
  ASSERT(obj->InternalFieldCount() == 3 &&
         obj->GetInternalField(1)->IsNumber() &&
         obj->GetInternalField(1)->Uint32Value() == type);

  return ExtractCPointer<NPObject>(obj->GetInternalField(0));
}

v8::Handle<v8::Value> V8Proxy::ThrowError(ErrorType type, const char* message) {
  switch (type) {
    case RANGE_ERROR:
      return v8::ThrowException(v8::Exception::RangeError(
          v8::String::New(message)));
    case REFERENCE_ERROR:
      return v8::ThrowException(v8::Exception::ReferenceError(
          v8::String::New(message)));
    case SYNTAX_ERROR:
      return v8::ThrowException(v8::Exception::SyntaxError(
          v8::String::New(message)));
    case TYPE_ERROR:
      return v8::ThrowException(v8::Exception::TypeError(
          v8::String::New(message)));
    case GENERAL_ERROR:
      return v8::ThrowException(v8::Exception::Error(
          v8::String::New(message)));
    default:
      ASSERT(false);
      return v8::Handle<v8::Value>();
  }
}

// A simple AsyncFunctor that calls ReleaseAll() on the thread's
// NPObjectReleaseList.
class AsyncReleaseObjects : public AsyncFunctor {
public:
  virtual void Run() {
    V8Locker locker;
    NPObjectReleaseList::ReleaseAllForCurrentThread();
  }
};

static const ThreadLocals::Slot kReleaseListKey = ThreadLocals::Alloc();
static Mutex g_release_list_map_lock;
static stdext::hash_map<ThreadId, NPObjectReleaseList*> g_release_list_map;

NPObjectReleaseList *NPObjectReleaseList::GetForCurrentThread() {
  NPObjectReleaseList *list =  reinterpret_cast<NPObjectReleaseList*>(
      ThreadLocals::GetValue(kReleaseListKey));
  ASSERT(list);
  return list;
}

// static
void NPObjectReleaseList::ThreadInit() {
  if (ThreadLocals::HasValue(kReleaseListKey))
    return;  // Already created for this thread.

  ThreadId thread_id = ThreadMessageQueue::GetInstance()->GetCurrentThreadId();
  NPObjectReleaseList *list = new NPObjectReleaseList(thread_id);

  MutexLock locker(&g_release_list_map_lock);
  g_release_list_map[thread_id] = list;

  // Add the list to ThreadLocals so that it gets deleted when the thread dies.
  ThreadLocals::SetValue(kReleaseListKey, list,
                         &NPObjectReleaseList::ThreadDestroy);
}

// static
void NPObjectReleaseList::ThreadDestroy(void *context) {
  NPObjectReleaseList *list = static_cast<NPObjectReleaseList*>(context);
  MutexLock locker(&g_release_list_map_lock);
  g_release_list_map.erase(list->creation_thread_id_);
  ASSERT(list->objects_to_release_.empty());
  delete list;
}

// static
void NPObjectReleaseList::AddForThread(NPObject *object, ThreadId thread_id) {
  v8::Locker::AssertIsLocked();
  MutexLock map_locker(&g_release_list_map_lock);
  NPObjectReleaseList *list = g_release_list_map[thread_id];

  // If this is NULL, then we're trying to release objects for a thread that
  // has already been destroyed, which should never happen.
  ASSERT(list);

  MutexLock locker(&list->lock_);
  if (list->objects_to_release_.empty()) {
    AsyncRouter::GetInstance()->CallAsync(
        list->creation_thread_id_, new AsyncReleaseObjects);
  }
  list->objects_to_release_.push_back(object);
}

// static
void NPObjectReleaseList::ReleaseAllForCurrentThread() {
  v8::Locker::AssertIsLocked();
  NPObjectReleaseList *list =  reinterpret_cast<NPObjectReleaseList*>(
      ThreadLocals::GetValue(kReleaseListKey));
  ASSERT(list);

  MutexLock locker(&list->lock_);
  for (NPObjectList::iterator it = list->objects_to_release_.begin();
       it != list->objects_to_release_.end(); ++it) {
    V8_NPN_ReleaseObject(*it);
  }
  list->objects_to_release_.clear();
}

void WrapNPObject(v8::Handle<v8::Object> obj, NPObject* npobj) {
  ASSERT(obj->InternalFieldCount() >= 3);
  obj->SetInternalField(0, V8Proxy::WrapCPointer(npobj));
  obj->SetInternalField(1, v8::Number::New(V8ClassIndex::NPOBJECT));

  // Create a JS object as a hash map for functions
  obj->SetInternalField(2, v8::Object::New());
}

NPUTF8* nputf8_strdup(const NPUTF8* str) {
  int len = strlen(str);
  char *copy = reinterpret_cast<char*>(malloc(sizeof(NPUTF8) * (len + 1)));
  strncpy(copy, str, len);
  copy[len] = 0;
  return reinterpret_cast<NPUTF8*>(copy);
}
