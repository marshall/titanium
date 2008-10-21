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

#ifndef GEARS_BASE_COMMON_DISPATCHER_H__
#define GEARS_BASE_COMMON_DISPATCHER_H__

#include <map>

#include "gears/base/common/base_class.h"
#include "gears/base/common/thread_locals.h"

class JsCallContext;

// An opaque type used to uniquely identify a method or property.
typedef void* DispatchId;

typedef std::map<std::string, DispatchId> DispatcherNameList;

// Interface to a dynamic dispatch class.  This class is responsible for
// taking a method or property identifier (from the JavaScript bindings), and
// calling the corresponding native method on a C++ class.
//
// Each class that wishes to be a target of dynamic dispatch must implement
// Dispatcher<T>::Init(), where the class can register the mappings between
// a method/property name and the native C++ method.
class DispatcherInterface {
 public:
  virtual ~DispatcherInterface() {}
  virtual bool HasMethod(DispatchId method_id) = 0;
  virtual bool HasPropertyGetter(DispatchId property_id) = 0;
  virtual bool HasPropertySetter(DispatchId property_id) = 0;
  virtual bool CallMethod(DispatchId method_id, JsCallContext *context) = 0;
  virtual bool GetProperty(DispatchId property_id, JsCallContext *context) = 0;
  virtual bool SetProperty(DispatchId property_id, JsCallContext *context) = 0;
  virtual const DispatcherNameList &GetMemberNames() = 0;
  virtual DispatchId GetDispatchId(const std::string &member_name) = 0;
};

// The implementation of the DispatcherInterface.  Each dispatch target class
// will have its own templated version, so that it has its own method and
// property lookup tables.  There should be one Dispatcher<T> instance for each
// instance of class T.
template<class T>
class Dispatcher : public DispatcherInterface {
 public:
  // The class that we dispatch property and method invocations to.
  typedef T ImplClass;

  // Callback function used for property and method invocations.
  typedef void (ImplClass::*ImplCallback)(JsCallContext *);

  Dispatcher(ImplClass *impl);
  virtual ~Dispatcher() {}

  // Called once per (thread, class-type) to initialize properties and methods.
  // Note: This must be specially implemented for each class type T.
  static void Init();

  // DispatcherInterface:
  virtual bool HasMethod(DispatchId method_id);
  virtual bool HasPropertyGetter(DispatchId property_id);
  virtual bool HasPropertySetter(DispatchId property_id);
  virtual bool CallMethod(DispatchId method_id, JsCallContext *context);
  virtual bool GetProperty(DispatchId property_id, JsCallContext *context);
  virtual bool SetProperty(DispatchId property_id, JsCallContext *context);
  virtual const DispatcherNameList &GetMemberNames();
  virtual DispatchId GetDispatchId(const std::string &member_name);

 protected:
  // Register JavaScript property/methods.
  // Note: setter may be NULL, but getter may not.
  static void RegisterProperty(const char *name,
                               ImplCallback getter, ImplCallback setter);
  static void RegisterMethod(const char *name, ImplCallback callback);

 private:
  typedef std::map<DispatchId, ImplCallback> IDList;

  struct ThreadLocalVariables {
    bool did_init_class;
    IDList property_getters;
    IDList property_setters;
    IDList methods;
    DispatcherNameList members;
    ThreadLocalVariables() : did_init_class(false) {}
  };

  static void DeleteThreadLocals(void *context);
  static ThreadLocalVariables &GetThreadLocals();

  static IDList& GetPropertyGetterList() {
    return GetThreadLocals().property_getters;
  }
  static IDList& GetPropertySetterList() {
    return GetThreadLocals().property_setters;
  }
  static IDList& GetMethodList() {
    return GetThreadLocals().methods;
  }

  static const ThreadLocals::Slot kThreadLocalsKey;

  ImplClass *impl_;

  DISALLOW_EVIL_CONSTRUCTORS(Dispatcher<T>);
};

// Used to set up the Dispatcher for the given class.
#define DECLARE_DISPATCHER(ImplClass) \
class ImplClass; \
template <> \
const ThreadLocals::Slot Dispatcher<ImplClass>::kThreadLocalsKey = \
    ThreadLocals::Alloc()

// Boilerplate code for constants

// Use DEFINE_CONSTANT for generating the implementation of the getter API.
// @JsName - name of constant as exposed in javascript.
// @CType - the type of constant in C (eg., int).
// @JsType - the Js type of the constant (eg., JSPARAM_INT).
// @Val - the value the constant is expected to return.
// This method uses a local variable to set the return value. Setting the
// value directly does not work.
#define DEFINE_CONSTANT(JsName, CType, JsType, Val)        \
  void Get##JsName(JsCallContext *context) {               \
    static const CType local_var = Val;                    \
    context->SetReturnValue(JsType, &local_var);           \
  }

// Use REGISTER_CONSTANT in Dispatcher<>::Init to make the above generated
// getter API available to Javascript.
// @JsName - name of constant as exposed in javascript.
// @GearsClass - class in which the constant is defined.
#define REGISTER_CONSTANT(JsName, GearsClass)              \
  RegisterProperty(#JsName, &GearsClass::Get##JsName, NULL);

// Returns the DispatchId associated with the given string.  Used for looking
// up methods and properties.
inline DispatchId GetStringIdentifier(const char *str) {
#if BROWSER_NPAPI
  return reinterpret_cast<DispatchId>(NPN_GetStringIdentifier(str));
#else
  // TODO(mpcomplete): Figure out what we need for other ports.
  // This only works if str is a static string.
  return reinterpret_cast<DispatchId>(const_cast<char *>(str));
#endif
}

template<class T>
Dispatcher<T>::Dispatcher(ImplClass *impl) : impl_(impl) {
  // Ensure that property and method mappings are initialized.
  ThreadLocalVariables &locals = GetThreadLocals();
  if (!locals.did_init_class) {
    locals.did_init_class = true;
    Init();
  }
}

template<class T>
bool Dispatcher<T>::HasMethod(DispatchId method_id) {
  const IDList &methods = GetMethodList();
  return methods.find(method_id) != methods.end();
}

template<class T>
bool Dispatcher<T>::HasPropertyGetter(DispatchId property_id) {
  const IDList &properties = GetPropertyGetterList();
  return properties.find(property_id) != properties.end();
}

template<class T>
bool Dispatcher<T>::HasPropertySetter(DispatchId property_id) {
  const IDList &properties = GetPropertySetterList();
  return properties.find(property_id) != properties.end();
}

template<class T>
bool Dispatcher<T>::CallMethod(DispatchId method_id, JsCallContext *context) {
  const IDList &methods = GetMethodList();
  typename IDList::const_iterator method = methods.find(method_id);
  if (method == methods.end())
    return false;
  ImplCallback callback = method->second;

  (impl_->*callback)(context);
  return true;
}

template<class T>
bool Dispatcher<T>::GetProperty(DispatchId property_id,
                                JsCallContext *context) {
  const IDList &properties = GetPropertyGetterList();
  typename IDList::const_iterator property = properties.find(property_id);
  if (property == properties.end())
    return false;
  ImplCallback callback = property->second;

  (impl_->*callback)(context);
  return true;
}

template<class T>
bool Dispatcher<T>::SetProperty(DispatchId property_id,
                                JsCallContext *context) {
  const IDList &properties = GetPropertySetterList();
  typename IDList::const_iterator property = properties.find(property_id);
  if (property == properties.end()) {
    return false;
  }
  ImplCallback callback = property->second;
  if (callback == NULL) { // Read only property.
    context->SetException(
                 STRING16(L"Cannot assign value to a read only property."));
    return true;
  }

  (impl_->*callback)(context);
  return true;
}

template<class T>
const DispatcherNameList &Dispatcher<T>::GetMemberNames() {
  return GetThreadLocals().members;
}

template<class T>
DispatchId Dispatcher<T>::GetDispatchId(const std::string &member_name) {
  const DispatcherNameList &member_names = GetMemberNames();
  DispatcherNameList::const_iterator result = member_names.find(member_name);
  if (result != member_names.end()) {
    return result->second;
  } else {
    return NULL;
  }
}

// static
template<class T>
void Dispatcher<T>::RegisterProperty(const char *name,
                                     ImplCallback getter, ImplCallback setter) {
  assert(getter);
  DispatchId id = GetStringIdentifier(name);
  GetPropertyGetterList()[id] = getter;
  GetPropertySetterList()[id] = setter;
  GetThreadLocals().members[name] = id;
}

// static
template<class T>
void Dispatcher<T>::RegisterMethod(const char *name, ImplCallback callback) {
  DispatchId id = GetStringIdentifier(name);
  GetMethodList()[id] = callback;
  GetThreadLocals().members[name] = id;
}

// static
template<class T>
void Dispatcher<T>::DeleteThreadLocals(void *context) {
  ThreadLocalVariables *locals =
      reinterpret_cast<ThreadLocalVariables*>(context);
  delete locals;
}

// static
template<class T>
typename Dispatcher<T>::ThreadLocalVariables &Dispatcher<T>::GetThreadLocals() {
  const ThreadLocals::Slot &key = kThreadLocalsKey;
  ThreadLocalVariables *locals =
      reinterpret_cast<ThreadLocalVariables*>(ThreadLocals::GetValue(key));
  if (!locals) {
    locals = new ThreadLocalVariables;
    ThreadLocals::SetValue(key, locals, &DeleteThreadLocals);
  }
  return *locals;
}

#endif // GEARS_BASE_COMMON_DISPATCHER_H__
