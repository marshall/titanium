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
//
// The C++ base class that all Gears objects should derive from.

#ifndef GEARS_BASE_COMMON_BASE_CLASS_H__
#define GEARS_BASE_COMMON_BASE_CLASS_H__

#include <map>

#include "gears/base/common/basictypes.h"  // for DISALLOW_EVIL_CONSTRUCTORS
#include "gears/base/common/browsing_context.h"
#include "gears/base/common/js_runner.h"
#include "gears/base/common/js_types.h"
#include "gears/base/common/permissions_manager.h"
#include "gears/base/common/scoped_refptr.h"
#include "gears/base/common/security_model.h"
#include "gears/base/common/string16.h"  // for string16
#include "third_party/scoped_ptr/scoped_ptr.h"

class ModuleWrapperBaseClass;

#ifdef WINCE
class GearsFactory;
#endif


// ModuleImplBaseClass objects created in the same environment (i.e. ones from
// the same page and thread) share a reference to the same ModuleEnvironment.
//
// This struct has public members, but is meant to be read-only once
// constructed.  Essentially, it should be const, apart from the inherited
// Ref/Unref methods.
struct ModuleEnvironment : public RefCounted {
 public:
  // CreateFromDOM can only be used from the main thread, and should only be
  // used for the first module created in the main thread - i.e. the first
  // GearsFactory instance. Other ModuleImplBaseClass instances are initialized
  // (i.e. they have InitModuleEnvironment called on them) by CreateModule.
  // This function returns NULL on failure. On success, the resultant
  // ModuleEnvironment should be immediately held within a scoped_refptr.
#if BROWSER_FF
  static ModuleEnvironment *CreateFromDOM();
#elif BROWSER_IE
  static ModuleEnvironment *CreateFromDOM(IUnknown *site);
#elif BROWSER_NPAPI
  static ModuleEnvironment *CreateFromDOM(JsContextPtr instance);
#endif

  ModuleEnvironment(SecurityOrigin security_origin,
#if BROWSER_IE
                    IUnknown *iunknown_site,
#endif
                    bool is_worker,
                    JsRunnerInterface *js_runner,
                    BrowsingContext *browsing_context);

  // Note that the SecurityOrigin may not necessarily be the same as the
  // originating page, e.g. in the case of a cross-origin worker, it would be
  // the origin of the foreign script.  Nonetheless, every module in the same
  // thread should share the same SecurityOrigin.
  SecurityOrigin security_origin_;

#if BROWSER_FF || BROWSER_NPAPI
  // The JavaScript context in which this module was created.
  JsContextPtr js_context_;
#elif BROWSER_IE
  // Pointer to the object that hosts this object. On Win32, this is the pointer
  // passed to SetSite. On WinCE this is the JS IDispatch pointer.
  CComPtr<IUnknown> iunknown_site_;
#endif

  bool is_worker_;
  JsRunnerInterface *js_runner_;
  scoped_refptr<BrowsingContext> browsing_context_;

  PermissionsManager permissions_manager_;

 private:
  // This struct is ref-counted and hence has a private destructor (which
  // should only be called on the final Unref).
  virtual ~ModuleEnvironment();

  DISALLOW_EVIL_CONSTRUCTORS(ModuleEnvironment);
};


class MarshaledModule {
 public:
  MarshaledModule() {}
  virtual ~MarshaledModule() {}
  virtual bool Unmarshal(ModuleEnvironment *module_environment,
                         JsScopedToken *out) = 0;
 private:
  DISALLOW_EVIL_CONSTRUCTORS(MarshaledModule);
};


// Exposes the minimal set of information that Gears objects need to work
// consistently across the main-thread and worker-thread JavaScript engines.
class ModuleImplBaseClass {
 public:
  explicit ModuleImplBaseClass(const std::string &name);
  virtual ~ModuleImplBaseClass();

  const std::string &get_module_name() const {
    return module_name_;
  }

  virtual MarshaledModule *AsMarshaledModule() { return NULL; }

  void InitModuleEnvironment(ModuleEnvironment *source_module_environment);

  // Host environment information
  void GetModuleEnvironment(scoped_refptr<ModuleEnvironment> *out) const;
  bool EnvIsWorker() const;
  const std::string16& EnvPageLocationUrl() const;
#if BROWSER_FF || BROWSER_NPAPI
  JsContextPtr EnvPageJsContext() const;
#elif BROWSER_IE
  IUnknown* EnvPageIUnknownSite() const;
#endif
  const SecurityOrigin& EnvPageSecurityOrigin() const;

  BrowsingContext *EnvPageBrowsingContext() const;

  JsRunnerInterface *GetJsRunner() const;

  PermissionsManager *GetPermissionsManager() const;

  // Methods for dealing with the JavaScript wrapper interface.
  void SetJsWrapper(ModuleWrapperBaseClass *wrapper) { js_wrapper_ = wrapper; }
  ModuleWrapperBaseClass *GetWrapper() const { 
    assert(js_wrapper_);
    return js_wrapper_;
  }

  void Ref();
  void Unref();

  // TODO(aa): Remove and replace call sites with GetWrapper()->GetToken().
  JsToken GetWrapperToken() const;

 protected:
  scoped_refptr<ModuleEnvironment> module_environment_;

 private:
  std::string module_name_;

  // Weak pointer to our JavaScript wrapper.
  ModuleWrapperBaseClass *js_wrapper_;

#ifdef WINCE
  // This method is defined in desktop/ie/factory.cc. It lets us verify that
  // privateSetGlobalObject() has been called from JavaScript on WinCE.
  friend bool IsFactoryInitialized(GearsFactory *factory);
#endif

  DISALLOW_EVIL_CONSTRUCTORS(ModuleImplBaseClass);
};


class DispatcherInterface;

// Interface for the wrapper class that binds the Gears object to the
// JavaScript engine.
class ModuleWrapperBaseClass {
 public:
  // Returns a token for this wrapper class that can be returned via the
  // JsRunnerInterface.
  virtual JsToken GetWrapperToken() const = 0;

  // Gets the Dispatcher for this module.
  virtual DispatcherInterface *GetDispatcher() const = 0;

  // Gets the ModuleImplBaseClass for this module.
  virtual ModuleImplBaseClass *GetModuleImplBaseClass() const = 0;

  // Adds a reference to the wrapper class.
  virtual void Ref() = 0;

  // Removes a reference to the wrapper class.
  virtual void Unref() = 0;

 protected:
  // Don't allow direct deletion via this interface.
  virtual ~ModuleWrapperBaseClass() { }
};

// Creates a new Module of the given type.  Returns false on failure.
// On failure, and if context is not NULL, then context's exception string
// will be set.
// Usually, OutType will be GearsClass or ModuleImplBaseClass.
template<class GearsClass, class OutType>
bool CreateModule(ModuleEnvironment *module_environment,
                  JsCallContext *context,
                  scoped_refptr<OutType> *module);

#endif  // GEARS_BASE_COMMON_BASE_CLASS_H__
