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

// Utility functions for SpiderMonkey NPAPI bindings.
#ifndef GEARS_WORKERPOOL_SAFARI_NPAPI_STORAGE_H__
#define GEARS_WORKERPOOL_SAFARI_NPAPI_STORAGE_H__

#include "third_party/spidermonkey/gears_include/mozjs_api.h"
#if BROWSER_WEBKIT
// NPAPI header not needed for WebKit.
#else
#include "third_party/npapi/npapi.h"
#endif

struct JSContext;
struct JSRuntime;

namespace SpiderMonkeyNPAPIBindings {

// Mozilla's NPAPI bindings make use of static storage, this class retreives
// instances of these variables from TLS, so we get a new instance of each
// on a per-thread basis.
class NPAPI_Storage {
 public:
   // Methods to create and destroy NPAPI data.
   static void CreateThreadLocals(JSRuntime *js_runtime, 
                                  JSContext *js_context,
                                  NPP global_object);
   
   // Called when thread is destroyed.
   static void ReleaseThreadLocals(void *data);
   
   // Sets the JS context & runtime values to NULL in TLS.
   // use after destroying these.
   static void ClearJSContext();
 
  static JSContext * GetCurrentJSContext();
  static NPP GetGlobalObject();
  
  // Hash of JSObject wrappers that wraps JSObjects as NPObjects. There
  // will be one wrapper per JSObject per plugin instance, i.e. if two
  // plugins access the JSObject x, two wrappers for x will be
  // created. This is needed to be able to properly drop the wrappers
  // when a plugin is torn down in case there's a leak in the plugin (we
  // don't want to leak the world just because a plugin leaks an
  // NPObject).
  static PLDHashTable &GetsJSObjWrappers();
  
  // Hash of NPObject wrappers that wrap NPObjects as JSObjects.
  static PLDHashTable &GetsNPObjWrappers();
  
  // Global wrapper count. This includes JSObject wrappers *and*
  // NPObject wrappers. When this count goes to zero, there are no more
  // wrappers and we can kill off hash tables etc.
  static int32 * GetsWrapperCount();

  // The JSRuntime. Used to unroot JSObjects when no JSContext is
  // reachable.
  static JSRuntime * GetsJSRuntime();
  
  static char ** GetgNPPException();
};

} // namespace SpiderMonkeyNPAPIBindings

#endif  // GEARS_WORKERPOOL_SAFARI_NPAPI_STORAGE_H__
