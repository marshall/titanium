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

#ifndef GEARS_BASE_NPAPI_MODULE_WRAPPER_H__
#define GEARS_BASE_NPAPI_MODULE_WRAPPER_H__

#include "gears/base/common/base_class.h"
#include "gears/base/common/js_runner.h"
#include "gears/base/npapi/browser_utils.h"
#include "gears/base/npapi/plugin.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

// Represents the bridge between the JavaScript engine and a Gears module. A
// ModuleWrapper wraps each Gears module instance and exposes its methods to
// JavaScript. The wrapper has ownership of the module, so that when the
// JavaScript engine decides to destroy the wrapper, the module itself is
// destroyed.
class ModuleWrapper
    : public PluginBase,
      public ModuleWrapperBaseClass {
 public:
  static NPClass *GetNPClass() {
    static NPClass np_class = GetNPClassTemplate<ModuleWrapper>();
    return &np_class;
  }

  ModuleWrapper(NPP instance) : PluginBase(instance) {
  }

  void Init(ModuleImplBaseClass *impl, DispatcherInterface *dispatcher) {
    PluginBase::Init(dispatcher);
    impl_.reset(impl);
    impl_->SetJsWrapper(this);
    dispatcher_.reset(dispatcher);
  }

  virtual JsToken GetWrapperToken() const {
    NPVariant token;
    OBJECT_TO_NPVARIANT(const_cast<ModuleWrapper *>(this), token);
    return token;
  }

  virtual DispatcherInterface *GetDispatcher() const {
    assert(dispatcher_.get());
    return dispatcher_.get();
  }
  
  virtual ModuleImplBaseClass *GetModuleImplBaseClass() const {
    assert(impl_.get());
    return impl_.get();
  }

  virtual void Ref() { NPN_RetainObject(this); }
  virtual void Unref() { NPN_ReleaseObject(this); }

 protected:
  scoped_ptr<ModuleImplBaseClass> impl_;
  scoped_ptr<DispatcherInterface> dispatcher_;

 private:
  DISALLOW_EVIL_CONSTRUCTORS(ModuleWrapper);
};


// Creates an instance of the class and its wrapper.
template<class GearsClass, class OutType>
bool CreateModule(ModuleEnvironment *module_environment,
                  JsCallContext *context,
                  scoped_refptr<OutType>* module) {
  ModuleWrapper *wrapper = static_cast<ModuleWrapper *>(
      NPN_CreateObject(module_environment->js_runner_->GetContext(),
                       ModuleWrapper::GetNPClass()));

  if (!wrapper) {
    if (context) {
      context->SetException(STRING16(L"Module creation failed."));
    }
    return false;
  }

  GearsClass *impl = new GearsClass;
  impl->InitModuleEnvironment(module_environment);
  wrapper->Init(impl, new Dispatcher<GearsClass>(impl));
  module->reset(impl);

  // In NPAPI, objects are created with refcount 1.  We want the scoped_refptr
  // to have the only reference, so we unref here after giving a ref to the
  // scoped_refptr.
  wrapper->Unref();
  return true;
}

#endif // GEARS_BASE_NPAPI_MODULE_WRAPPER_H__
