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

#ifndef GEARS_BASE_COMMON_REFCOUNT_H__
#define GEARS_BASE_COMMON_REFCOUNT_H__

#include <assert.h>

#include "gears/base/common/atomic_ops.h"
#include "gears/base/common/common.h"

// RefCountedInterface illustrates the method signatures for a class which
// is compatible with scoped_refptr.
//
// class RefCountedInterface {
//  public:
//   // Adds a reference to the object.
//   void Ref();
//
//   // Removes a reference to the object, and destroys it if there are no more
//   // references.
//   void Unref();
// };

// scoped_refptr is a smart pointer similar to scoped_ptr, but which holds a
// reference to its payload.

template <typename T>
class scoped_refptr;

template<typename T>
T** as_out_parameter(scoped_refptr<T>&);

template <typename T>
class scoped_refptr {
 public:
  typedef T element_type;

  // The 'explicit' keyword is omitted on these constructors to allow intuitive
  // raw pointer interoperability such as the following:
  //   Foo* f;
  //   void Func(const scoped_refptr<Foo>& foo);
  //   std::vector< scoped_refptr<Foo> > vec;
  //   Func(f);
  //   vec.push_back(f);
  // Instead of:
  //   Func(scoped_refptr<Foo>(f));
  //   vec.push_back(scoped_refptr<Foo>(f));

  scoped_refptr(T* p = NULL) : ptr_(p) {
    DoRef(ptr_);
  }

  scoped_refptr(const scoped_refptr& b) : ptr_(b.get()) {
    DoRef(ptr_);
  }

  template <typename U>
  scoped_refptr(const scoped_refptr<U>& b) : ptr_(b.get()) {
    DoRef(ptr_);
  }

  ~scoped_refptr() {
    typedef char type_must_be_complete[sizeof(T)];
    DoUnref(ptr_);
  }

  T* get() const {
    return ptr_;
  }

  void reset(T* p = NULL) {
    typedef char type_must_be_complete[sizeof(T)];
    T* tmp = ptr_;
    ptr_ = p;
    DoRef(ptr_);
    DoUnref(tmp);
  }

  // Note: release() is explicitly unsupported, because:
  // 1) The concept of transferring ownership is less relevant for a
  //    shared-ownership pointer.
  // 2) scoped_refptr::release has different semantics from scoped_ptr::release,
  //    which may lead to inadvertent memory leaks.
  //  Example:
  //   scoped_ptr<Foo> p(new Foo);          // p owns the object.
  //   Foo* p1 = p.release();               // p1 owns the object.
  //   scoped_ptr<Foo> p2(p.release());     // p2 owns the object.
  // 
  //   scoped_refptr<Foo> p(new Foo);       // p owns the object, ref == 1.
  //   Foo* p1 = p.release();               // p1 owns the object, ref == 1.
  //   scoped_refptr<Foo> p2(p.release());  // p2 owns the object, ref == 2.
  //
  // Releases ownership of p without decreasing its reference count.
  //T** release() {
  //  T* tmp = ptr_;
  //  ptr_ = NULL;
  //  return tmp;
  //}

  // Allows assignment from raw pointers and scoped_refptrs.
  scoped_refptr& operator=(T* p) {
    reset(p);
    return *this;
  }

  scoped_refptr& operator=(const scoped_refptr& b) {
    return this->operator=(b.get());
  }

  template <typename U>
  scoped_refptr& operator=(const scoped_refptr<U>& b) {
    return this->operator=(b.get());
  }

  // Allows dereferencing (* and ->).
  T& operator*() const {
    assert(ptr_ != NULL);
    return *ptr_;
  }

  T* operator->() const  {
    assert(ptr_ != NULL);
    return ptr_;
  }

  // Allows comparison to raw pointers and scoped_refptrs.
  bool operator==(T* p) const {
    return ptr_ == p;
  }

  bool operator!=(T* p) const {
    return ptr_ != p;
  }

  // Allows evaluation in a boolean context, without the dangers of
  // operator bool().  AsBoolean is undefined and unrelated to any other type,
  // so implicit conversions or comparisons will not occur.
  // Example:
  //   scoped_refptr<Foo> f;
  //   if (f) {}
  //   if (!f) {}
  struct AsBoolean;
  operator AsBoolean*() const { return reinterpret_cast<AsBoolean*>(ptr_); }

  template <typename U>
  bool operator==(const scoped_refptr<U>& b) const {
    return this->operator==(b.get());
  }

  template <typename U>
  bool operator!=(const scoped_refptr<U>& b) const {
    return this->operator!=(b.get());
  }

 private:
  friend T** as_out_parameter<>(scoped_refptr<T>&);

  // DoRef and DoUnref provide convenient points for instrumentation or debug
  // breakpoints.
  inline void DoRef(T* p) {
    if (p) {
      p->Ref();
    }
  }

  inline void DoUnref(T* p) {
    if (p) {
      p->Unref();
    }
  }

  T* ptr_;
};

template<typename T, typename U> inline
bool operator==(T* p, const scoped_refptr<U>& b) {
  return p == b.get();
}

template<typename T, typename U> inline
bool operator!=(T* p, const scoped_refptr<U>& b) {
  return p != b.get();
}

// Returns the internal pointer, suitable for passing to functions that
// return an object as an out parameter.
// Example:
//   int CreateObject(Foo** created);
//   scoped_refptr<Foo> foo;
//   CreateObject(as_out_parameter(foo));
template<typename T>
T** as_out_parameter(scoped_refptr<T>& p) {
  assert(p.ptr_ == NULL);
  return &p.ptr_;
}

// RefCount is a convenience object for implementing a thread-safe integer with
// atomic increment and decrement.

class RefCount {
 public:
  RefCount() : count_(0) { }

  // Increments the count atomically, and returns true on a 0 -> 1 transition.
  inline bool Ref() { return (1 == AtomicIncrement(&count_, 1)); }

  // Decrements the count atomically, and returns true on a 1 -> 0 transition.
  inline bool Unref() { 
    AtomicWord count = AtomicIncrement(&count_, -1);
    assert(count >= 0);
    return (0 == count);
  }

  // Returns the current value of the count.  The value is intended for
  // diagnostic code and is meaningless in multi-threaded scenarios.
  inline AtomicWord Value() { return AtomicIncrement(&count_, 0); };

 private:
  AtomicWord count_;
  DISALLOW_EVIL_CONSTRUCTORS(RefCount);
};

// RefCounted is a convenience base class for reference-counted classes.
// A subclass that participates in multiple inheritance should virtually
// inherit from RefCounted to avoid multiple internal counts.
//  Example:
//   class A : virtual public RefCounted { };
//   class B : virtual public RefCounted { };
//   class C : public A, public B { };

class RefCounted {
 public:
  void Ref() {
    count_.Ref();
  }
  void Unref() {
    if (count_.Unref())
      delete this;
  }
  AtomicWord GetRef() {
    return count_.Value();
  }

 protected:
  RefCounted() { }
  virtual ~RefCounted() { }

 private:
  RefCount count_;
  DISALLOW_EVIL_CONSTRUCTORS(RefCounted);
};

#endif // GEARS_BASE_COMMON_REFCOUNT_H__
