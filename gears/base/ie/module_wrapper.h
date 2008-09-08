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

#ifndef GEARS_BASE_IE_MODULE_WRAPPER_H__
#define GEARS_BASE_IE_MODULE_WRAPPER_H__

#include <assert.h>
#include "gears/base/common/base_class.h"
#include "gears/base/common/common.h"
#include "gears/base/common/dispatcher.h"
#include "gears/base/common/js_types.h"
#include "gears/base/ie/atl_headers.h"
#include "genfiles/interfaces.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

// Represents the bridge between the JavaScript engine and a Gears module. A
// ModuleWrapper wraps each Gears module instance and exposes its methods to
// JavaScript. The wrapper is ref-counted and has ownership of the module so
// that when all references are released, the module is destroyed.
class ATL_NO_VTABLE ModuleWrapper
    : public ModuleWrapperBaseClass,
      public IDispatch,
      public GearsModuleProviderInterface,
      public CComObjectRootEx<CComMultiThreadModel>,
      public CComCoClass<ModuleWrapper> {      
 public:
  BEGIN_COM_MAP(ModuleWrapper)
    COM_INTERFACE_ENTRY(GearsModuleProviderInterface)
    COM_INTERFACE_ENTRY(IDispatch)
  END_COM_MAP()

  DECLARE_NOT_AGGREGATABLE(ModuleWrapper)
  DECLARE_PROTECT_FINAL_CONSTRUCT()

  ModuleWrapper() {
    VariantInit(&token_);
    token_.vt = VT_DISPATCH;
    token_.pdispVal = const_cast<ModuleWrapper *>(this);
  }

  ~ModuleWrapper() {}

  // IDispatch: GetTypeInfoCount, GetTypeInfo, GetIDsOfNames, Invoke.
  STDMETHOD(GetTypeInfoCount)(unsigned int FAR* retval);
  STDMETHOD(GetTypeInfo)(unsigned int index, LCID lcid,
                         ITypeInfo FAR* FAR* retval);
  STDMETHOD(GetIDsOfNames)(REFIID iid, OLECHAR FAR* FAR* names,
                           unsigned int num_names, LCID lcid, 
                           DISPID FAR* retval);
  STDMETHOD(Invoke)(DISPID member_id, REFIID iid, LCID lcid, WORD flags,
                    DISPPARAMS FAR* params, VARIANT FAR* retval,
                    EXCEPINFO FAR* exception,
                    unsigned int FAR* arg_error_index);

  // GearsModuleProviderInterface: get_moduleWrapper.
  STDMETHOD(get_moduleWrapper)(VARIANT *retval);

  // ModuleWrapperBaseClass
  // Returns a token for this wrapper class that can be returned via the
  // JsRunnerInterface.
  virtual JsToken GetWrapperToken() const {
    return token_;
  }

  // Gets the Dispatcher for this module.
  virtual DispatcherInterface *GetDispatcher() const {
    assert(dispatcher_.get());
    return dispatcher_.get();
  }

  virtual ModuleImplBaseClass *GetModuleImplBaseClass() const {
    assert(impl_.get());
    return impl_.get();
  }

  virtual void Ref() {
    AddRef();
  }

  virtual void Unref() {
    Release();
  }

  void Init(ModuleImplBaseClass *impl, DispatcherInterface *dispatcher) {
    assert(!impl_.get());
    assert(impl);
    impl_.reset(impl);

    assert(!dispatcher_.get());
    assert(dispatcher);
    dispatcher_.reset(dispatcher);
  }

 private:
  scoped_ptr<ModuleImplBaseClass> impl_;
  scoped_ptr<DispatcherInterface> dispatcher_;
  VARIANT token_;

  DISALLOW_EVIL_CONSTRUCTORS(ModuleWrapper);
};



// Creates an instance of the class and its wrapper.
template<class GearsClass, class OutType>
bool CreateModule(ModuleEnvironment *module_environment,
                  JsCallContext *context,
                  scoped_refptr<OutType>* module) {
  CComObject<ModuleWrapper> *module_wrapper;
  HRESULT hr = CComObject<ModuleWrapper>::CreateInstance(&module_wrapper);
  if (FAILED(hr)) {
    if (context) {
      context->SetException(STRING16(L"Module creation failed."));
    }
    return false;
  }

  GearsClass *impl = new GearsClass();
  impl->InitModuleEnvironment(module_environment);
  Dispatcher<GearsClass> *dispatcher = new Dispatcher<GearsClass>(impl);

  module_wrapper->Init(impl, dispatcher);
  impl->SetJsWrapper(module_wrapper);
  module->reset(impl);
  return true;
}

#endif  //  GEARS_BASE_IE_MODULE_WRAPPER_H__
