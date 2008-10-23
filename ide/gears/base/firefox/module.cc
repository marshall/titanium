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
#if BROWSER_FF3
// This needs to be here because of following: common_ff.h defines
// FORCE_PR_LOG and includes prlog.h, which defines PR_LOGGING however,
// appearently with gecko 1.9 some other header includes prlog.h before
// #includes below include common_ff.h. this causes PR_LOGGING to be not
// defined and gLog to become not defined symbol in opt builds
#include "gears/base/common/common_ff.h"
#endif
#include <gecko_sdk/include/nsXPCOM.h>
#include <gecko_sdk/include/nsMemory.h>
#include <gecko_sdk/include/nsILocalFile.h>
#include <gecko_sdk/include/nsIGenericFactory.h>
#include <gecko_sdk/include/nsICategoryManager.h>
#include <gecko_internal/nsIDOMClassInfo.h>
#include <gecko_internal/nsIScriptNameSpaceManager.h>
#include <gecko_internal/nsIXULAppInfo.h>

#include "gears/base/common/leak_counter.h"
#include "gears/base/common/message_queue.h"
#include "gears/base/common/string_utils.h"
#include "gears/base/common/thread_locals.h"
#include "gears/base/firefox/xpcom_dynamic_load.h"
#include "gears/factory/factory_ff.h"

#include "gears/localserver/firefox/cache_intercept.h"
#include "gears/ui/firefox/ui_utils.h"

#if BROWSER_FF2
#include <gecko_internal/nsIEventQueueService.h> // for event loop
#endif

#if BROWSER_FF2
// From gears/workerpool/firefox/pool_threads_manager.h
void DestroyThreadRecycler();
#endif
//-----------------------------------------------------------------------------

// TODO(cprince): can remove this when switch to google3 logging
#ifdef PR_LOGGING
PRLogModuleInfo *gLog = PR_NewLogModule(PRODUCT_SHORT_NAME_ASCII);
#endif

//-----------------------------------------------------------------------------

#define SINGLETON_CONSTRUCTOR(class_)                                  \
static NS_METHOD class_##Constructor(nsISupports *outer,               \
                                     const nsIID &iid,                 \
                                     void **result) {                  \
  if (outer)                                                           \
    return NS_ERROR_NO_AGGREGATION;                                    \
  static class_ *service = nsnull;                                     \
  if (!service) {                                                      \
    service = new class_();                                            \
    if (!service)                                                      \
      return NS_ERROR_OUT_OF_MEMORY;                                   \
    NS_ADDREF(service);  /* hold reference for lifetime of app */      \
  }                                                                    \
  return service->QueryInterface(iid, result);                         \
}

#define SINGLETON_CONSTRUCTOR_INIT(class_)                             \
static NS_METHOD class_##Constructor(nsISupports *outer,               \
                                     const nsIID &iid,                 \
                                     void **result) {                  \
  if (outer)                                                           \
    return NS_ERROR_NO_AGGREGATION;                                    \
  static class_ *service = nsnull;                                     \
  if (!service) {                                                      \
    service = new class_();                                            \
    if (!service)                                                      \
      return NS_ERROR_OUT_OF_MEMORY;                                   \
    NS_ADDREF(service);  /* hold reference for lifetime of app */      \
    nsresult rv = service->Init();                                     \
    if (NS_FAILED(rv)) {                                               \
      NS_RELEASE(service);                                             \
      return rv;                                                       \
    }                                                                  \
  }                                                                    \
  return service->QueryInterface(iid, result);                         \
}

//-----------------------------------------------------------------------------


const char *kDomciExtensionContractId = "@google.com/" PRODUCT_SHORT_NAME_ASCII
                                        "/domciextension;1";
const char *kDomciExtensionClassName = "DomciExtension";
const nsCID kDomciExtensionClassId = {0x71f2446a, 0x81ed, 0x4345, {0x8d, 0xdb,
                                      0xd6, 0x9b, 0xd5, 0xc3, 0xc7, 0x32}};
                                     // {71F2446A-81ED-4345-8DDB-D69BD5C3C732}

NS_DOMCI_EXTENSION(Scour)
  // "Under The Hood":
  // The entries below form the body of this extension's RegisterDOMCI method.
  // For each block, if the string in line 1 matches the RegisterDOMCI argument,
  // RegisterDOMClassInfo will be called for each of the enclosed IIDs.
  // RegisterDOMClassInfo calls into gNameSpaceManager->RegisterDOMCIData.

  // factory
  NS_DOMCI_EXTENSION_ENTRY_BEGIN(GearsFactory)
    NS_DOMCI_EXTENSION_ENTRY_INTERFACE(GearsFactoryInterface)
  NS_DOMCI_EXTENSION_ENTRY_END_NO_PRIMARY_IF(GearsFactory, PR_TRUE,
                                             &kGearsFactoryClassId)
NS_DOMCI_EXTENSION_END

static NS_METHOD ScourRegisterSelf(nsIComponentManager *compMgr,
                                   nsIFile *path,
                                   const char *loaderStr,
                                   const char *type,
                                   const nsModuleComponentInfo *info) {
  LOG(("RegisterSelf()\n"));

  // Trigger early initialization of our cache interceptor.
  nsCOMPtr<nsICategoryManager> catMgr =
      do_GetService(NS_CATEGORYMANAGER_CONTRACTID);
  if (!catMgr)
    return NS_ERROR_UNEXPECTED;

  // The cache intercept component needs to be initialized at xpcom startup
  // time (so that it can override the default cache implementation).
  catMgr->AddCategoryEntry(NS_XPCOM_STARTUP_OBSERVER_ID,
                           kCacheInterceptClassName,
                           kCacheInterceptContractId,
                           PR_TRUE, PR_TRUE, NULL);

  // The JAVASCRIPT_DOM_CLASS category lets us hook into "new GearsFactory()"
  // calls.
  catMgr->AddCategoryEntry(JAVASCRIPT_DOM_CLASS,
                           kGearsFactoryClassName,
                           kDomciExtensionContractId,
                           PR_TRUE, PR_TRUE, NULL);

#ifdef PROVIDE_GEARS_AS_JAVASCRIPT_GLOBAL_OBJECT
  // Providing a global "gears" object lets web pages call something like:
  //   var ls = gears.create('beta.localserver')
  // rather than the two step process of:
  //   1. include the 'gears_init.js' bootstrap script
  //   2. var ls = google.gears.factory.create('beta.localserver')
  //
  // Further points:
  //   * The "gears" object is the same as "window.gears".
  //   * The XPCOM object that backs "gears" is created lazily, and the
  //     instance is cached so that repeated calls to "gears" will refer to
  //     the same jsval and the same C++ object.
  //   * If JavaScript code overwrites "gears" (or "window.gears"), e.g.:
  //     var gears = true;
  //     then the XPCOM object is no longer accessible (unless another
  //     reference has been made, e.g.:
  //     var originalGears = window.gears;
  //     window.gears = true;
  //
  // As of 2008-07-25, PROVIDE_GEARS_AS_JAVASCRIPT_GLOBAL_OBJECT is not
  // defined by default, since the feature is not implemented uniformly
  // across all platforms.
  catMgr->AddCategoryEntry(JAVASCRIPT_GLOBAL_PROPERTY_CATEGORY,
                           "gears",
                           kGearsFactoryContractId,
                           PR_TRUE, PR_TRUE, NULL);
#endif

  return NS_OK;
}


// We need a NS_DECL_DOM_CLASSINFO for each
// NS_INTERFACE_MAP_ENTRY_EXTERNAL_DOM_CLASSINFO in the codebase.
//
// These macros inform the DOM about every Gears class it might see
// (indicating, for example, what methods and properties exist).
//
// "Under The Hood":
// NS_DECL_DOM_CLASSINFO instances a global variable.  Its argument must match
// the argument to NS_INTERFACE_MAP_ENTRY_EXTERNAL_DOM_CLASSINFO.  The value
// also gets string-ified and affects the class name exposed to JavaScript.

// factory
NS_DECL_DOM_CLASSINFO(GearsFactory)

nsresult PR_CALLBACK ScourModuleConstructor(nsIModule *self) {
  if (NS_FAILED(ThreadLocals::HandleModuleConstructed())) {
    return NS_ERROR_FAILURE;
  }
  ThreadMessageQueue::GetInstance()->InitThreadMessageQueue();
  LEAK_COUNTER_INITIALIZE();
  return NS_OK;
}


void PR_CALLBACK ScourModuleDestructor(nsIModule *self) {
  // We need a NS_IF_RELEASE for each
  // NS_INTERFACE_MAP_ENTRY_EXTERNAL_DOM_CLASSINFO in the codebase.

  // factory
  NS_IF_RELEASE(NS_CLASSINFO_NAME(GearsFactory));

#if BROWSER_FF2
  DestroyThreadRecycler();
#endif
  LEAK_COUNTER_DUMP_COUNTS();
}


// Define nsFactory constructors for certain classes.
// These constructors are referenced in components[] below.
//
// We do not need to define a factory constructor for Gears objects that
// should only be created via GearsFactory (rather than instanced directly).
//
// IMPORTANT: objects that derive from ModuleImplBaseClass should not use
// singleton init because their state (like security origin) will not get
// updated when the page changes!

SINGLETON_CONSTRUCTOR(CacheIntercept)
SINGLETON_CONSTRUCTOR(GearsUiUtils)
NS_GENERIC_FACTORY_CONSTRUCTOR_INIT(GearsFactory, InitFactoryFromDOM)
// On Firefox this C++ Factory name affects the script-visible name.
// It may be possible to use modified Mozilla macros to break the dependency,
// but we don't have that ability today.


static const nsModuleComponentInfo components[] = {
  // internal plumbing
  { kDomciExtensionClassName, // [0] fields could be any description string
    kDomciExtensionClassId,
    kDomciExtensionContractId,
    NS_DOMCI_EXTENSION_CONSTRUCTOR(Scour) },

  { kCacheInterceptClassName,
    kCacheInterceptClassId,
    kCacheInterceptContractId,
    CacheInterceptConstructor,
    ScourRegisterSelf }, // register all components in one go

  // ui
  { kGearsUiUtilsClassName,
    kGearsUiUtilsClassId,
    kGearsUiUtilsContractId,
    GearsUiUtilsConstructor },

  // factory
  { kGearsFactoryClassName,
    kGearsFactoryClassId,
    kGearsFactoryContractId,
    GearsFactoryConstructor }
};


static nsModuleInfo const kModuleInfo = {
  NS_MODULEINFO_VERSION,
  ("gears_module"),
  (components),
  (sizeof(components) / sizeof(components[0])),
  (ScourModuleConstructor),
  (ScourModuleDestructor)
};
NSGETMODULE_ENTRY_POINT(gears_module) (nsIComponentManager *servMgr,
                                       nsIFile* location,
                                       nsIModule** result) {
  // This module is compiled once against Gecko 1.8 (Firefox 1.5 and 2) and
  // once against Gecko 1.9 (Firefox 3). We need to make sure that we are
  // only loaded into the environment we were compiled against.
  nsresult nr;
  nsCOMPtr<nsIXULAppInfo> app_info =
      do_GetService("@mozilla.org/xre/app-info;1", &nr);
  if (NS_FAILED(nr) || !app_info) {
    return NS_ERROR_FAILURE;
  }

  nsCString gecko_version;
  app_info->GetPlatformVersion(gecko_version);

#if defined(BROWSER_FF2)
  if (strncmp(gecko_version.BeginReading(), "1.8", 3) != 0) {
    return NS_ERROR_FAILURE;
  }
#elif defined(BROWSER_FF3)
  if (strncmp(gecko_version.BeginReading(), "1.9", 3) != 0) {
    return NS_ERROR_FAILURE;
  }

  // We don't support the early betas of Firefox 3 that many people have
  // installed, so we explicitly disallow those versions here.
  nsCString build_id_string;
  app_info->GetPlatformBuildID(build_id_string);

  // The first FF3 version that we support is RC1. I got its build ID here:
  // http://wiki.mozilla.org/QA/Firefox3/TestResults/RC1
  // There are several listed for the various platforms. I use the earliest.
  int build_id = ParseLeadingInteger(build_id_string.BeginReading(), NULL);
  if (build_id < 2008051202) {
    return NS_ERROR_FAILURE;
  }
#endif

  return NS_NewGenericModule2(&kModuleInfo, result);
}
