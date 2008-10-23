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

#ifndef GEARS_V8_BINDINGS_V8_HELPERS_H__
#define GEARS_V8_BINDINGS_V8_HELPERS_H__

#include <assert.h>

#include "gears/base/common/common.h"
#include "gears/base/common/message_queue.h"
#include "third_party/v8/public/v8.h"
#include "v8_npruntime.h"

#define ASSERT assert

// Type specifiers placed in V8 object internal data to let us know what type
// it holds.  Gears only cares about NPObjects.
class V8ClassIndex {
 public:
  enum V8WrapperType {
    INVALID_CLASS_INDEX = 0,
    NPOBJECT,
    CLASSINDEX_END,
  };
};

class V8Proxy {
 public:
  // The types of javascript errors that can be thrown.
  enum ErrorType {
    RANGE_ERROR,
    REFERENCE_ERROR,
    SYNTAX_ERROR,
    TYPE_ERROR,
    GENERAL_ERROR
  };

  static v8::Handle<v8::Value> WrapCPointer(void *cptr);
  static void *ExtractCPointerImpl(v8::Handle<v8::Value> obj);
  static void *ToNativeObjectImpl(V8ClassIndex::V8WrapperType type,
                                  v8::Handle<v8::Value> obj);

  template<class T>
  static T *ExtractCPointer(v8::Handle<v8::Value> obj) {
    return static_cast<T*>(ExtractCPointerImpl(obj));
  }

  template<class T>
  static T *ToNativeObject(V8ClassIndex::V8WrapperType type,
                           v8::Handle<v8::Value> obj) {
    return static_cast<T*>(ToNativeObjectImpl(type, obj));
  }

  static v8::Handle<v8::Value> ThrowError(ErrorType type, const char* message);
};

class SafeAllocation {
 public:
  static v8::Local<v8::Object> NewInstance(v8::Handle<v8::Function> fun) {
    return fun->NewInstance();
  }
};

// Wrapper classes around v8::Locker and v8::Unlocker that give us more debug
// info about whether V8 is locked or not.  The V8 locks are not reentrant, so
// these classes have asserts to ensure we don't use them incorrectly.
class V8Locker {
 public:
  V8Locker();
  ~V8Locker();
 private:
  v8::Locker locker_;
};

class V8Unlocker {
 public:
  V8Unlocker();
  ~V8Unlocker();
 private:
  // This is a pointer so that we have more control over when we unlock and
  // re-lock V8.
  v8::Unlocker* unlocker_;
};

// This class holds a list of objects to release for a thread.  Since V8 garbage
// collection can happen on any thread, we must take care to delete objects on
// the right thread.  When an object is GC'd, we add it to the list of objects
// for the thread it was created on, and we post a message to Release all
// objects on that thread.
class NPObjectReleaseList {
 public:
  // Init per-thread data.  Calling a second time on a thread is a no-op.
  static void ThreadInit();

  // Note that these next methods should only be accessed while V8 is locked.
  // Failure to do so could result in deadlock.

  // Schedule an NPObject to be released asynchronously on the given thread.
  static void AddForThread(NPObject *object, ThreadId thread_id);

  // Release all the objects scheduled for release on this thread.
  static void ReleaseAllForCurrentThread();

 private:
  static NPObjectReleaseList *GetForCurrentThread();
  static void ThreadDestroy(void *context);

  NPObjectReleaseList(ThreadId thread_id) : creation_thread_id_(thread_id) {}

  typedef std::vector<NPObject*> NPObjectList;
  Mutex lock_;
  ThreadId creation_thread_id_;
  NPObjectList objects_to_release_;
};
// Associates an NPObject with a V8 object.
void WrapNPObject(v8::Handle<v8::Object> obj, NPObject *npobj);

// Retrieves the V8 Context from the NP context.  The second parameter is unused
// and exists to ease porting from Chrome code.
// Implemented in js_runner_np.cc as it relies on JsRunner.
v8::Local<v8::Context> GetV8Context(NPP npp, NPObject *npobj_unused);

// Retrieves the global object.  Does not addref the return value.
// Implemented in js_runner_np.cc as it relies on JsRunner.
NPObject *GetGlobalNPObject(NPP npp);

// Visual studio whines about strdup being deprecated.
NPUTF8* nputf8_strdup(const NPUTF8* str);

#endif
