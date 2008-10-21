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

// Create a SpiderMonkey instance and return an NPAPI wrapper for it.

#include "gears/base/common/js_standalone_engine.h"
#include "gears/base/common/js_runner.h"
#include "gears/base/common/mutex.h"
#include "gears/base/common/scoped_token.h"
#ifdef BROWSER_WEBKIT
#include "gears/base/safari/npapi_patches.h" 
#endif
#include "third_party/spidermonkey/gears_include/mozjs_api.h"
#include "third_party/spidermonkey/gears_npapi_bindings/mozjs_npapi_storage.h"
#include "third_party/spidermonkey/gears_npapi_bindings/mozjs_npruntime.h"

// Mutex to guard JSCreateContext calls.
static Mutex engine_creation_mutex;

// Defined in js_runner_np.cc
void HandleJSError(const JsRunner *js_runner, JsErrorInfo &error_info);

typedef DECLARE_SCOPED_TRAITS(JSContext*, JS_DestroyContext, NULL)
    JSContextTraits;
typedef scoped_token<JSContext*, JSContextTraits> scoped_jscontext_ptr;

// Spidermonkey callback for error handling.
static void JS_DLL_CALLBACK JsErrorHandler(JSContext *cx, const char *message,
                                           JSErrorReport *report) {
  // Ugly hack, without this, if we call a JS error handler from here
  // then Spidermonkey will bork.
  JSBool old_val = cx->throwing;
  cx->throwing = JS_FALSE;
  
  JsRunner *js_runner = static_cast<JsRunner*>(JS_GetContextPrivate(cx));
  if (js_runner && report) {
    JsErrorInfo error_info;
    error_info.line = report->lineno + 1; // Reported lines start at zero.

    // The error message can either be in the separate *message param or in
    // *report->ucmessage. For example, running the following JS in a worker
    // causes the separate message param to get used:
    //   throw new Error("foo")
    // Other errors cause the report->ucmessage property to get used.
    //
    // Mozilla also does this, see:
    // http://lxr.mozilla.org/mozilla1.8.0/source/dom/src/base/nsJSEnvironment.cpp#163
    if (report->ucmessage) {
      error_info.message = reinterpret_cast<const char16 *>(report->ucmessage);
    } else if (message) {
      std::string16 message_str;
      if (UTF8ToString16(message, &message_str)) {
        error_info.message = message_str;
      }
    }

    HandleJSError(js_runner, error_info);
    
    // See comment above.
    cx->throwing = old_val;
  }
}

#ifdef DEBUG
static JSBool js_gc(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, 
                    jsval *rval) {
  JS_GC(cx);
  return JS_TRUE;
}
    
static JSBool js_print(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, 
                       jsval *rval) {
  JSString *str = JS_ValueToString(cx, argv[0]);
  printf("script result: %s\n", JS_GetStringBytes(str));
  return JS_TRUE;
}

static JSBool js_break(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, 
                       jsval *rval) {
  return JS_TRUE;
}
#endif


// Create a JS context & Engine and return them.
static bool CreateJSEngine(JsRunner *js_runner, JSRuntime **runtime, 
                           JSContext **context) {
  // These are taken from js_runner_ff.cc, if changes are made you'll
  // probably want to change the values there too.
#ifdef OS_ANDROID
  const int kRuntimeMaxBytes = 1 * 1024 * 1024; // Somewhat more sensible.
#else
  const int kRuntimeMaxBytes = 64 * 1024 * 1024; // mozilla/.../js.c uses 64 MB
#endif
  const int kContextStackChunkSize = 1024; // Firefox often uses 1024;
                                          // also see js/src/readme.html
  // Create a new JS Runtime.
  JSRuntime *rt;
  {
      MutexLock lock(&engine_creation_mutex);
      rt = JS_NewRuntime(kRuntimeMaxBytes);
  }
  
  if (!rt) { return false; }
  
  // Initialize a context.
  scoped_jscontext_ptr cx(JS_NewContext(rt, kContextStackChunkSize));
  if (!cx.get()) {
    return false;
  }
  
  // Setup context
  // VAROBJFIX is recommended in /mozilla/js/src/jsapi.h
  JS_SetOptions(cx.get(), JS_GetOptions(cx.get()) | JSOPTION_VAROBJFIX);
  JS_SetErrorReporter(cx.get(), JsErrorHandler); 
  JS_SetContextPrivate(cx.get(), reinterpret_cast<void*>(js_runner));
  
  // These structs are static because they must live for duration of JS engine.
  // SpiderMonkey README also suggests using static for one-off objects.
  static JSClass global_class = {
      "Global", 0, // name, flags
      JS_PropertyStub, JS_PropertyStub,  // defineProperty, deleteProperty
      JS_PropertyStub, JS_PropertyStub, // getProperty, setProperty
      JS_EnumerateStub, JS_ResolveStub, // enum, resolve
      JS_ConvertStub, JS_FinalizeStub // convert, finalize
  };
  
  JSAutoRequest context_lock(cx.get());
  JSObject *global_obj = JS_NewObject(cx.get(), &global_class, 0, 0);
  if (!global_obj) { return false; }

  JSBool js_ok = JS_InitStandardClasses(cx.get(), global_obj);
  if (!js_ok) { return false; }
  
#ifdef DEBUG
  JS_DefineFunction(cx.get(), global_obj, "print", js_print, 1, 0);
  JS_DefineFunction(cx.get(), global_obj, "GearsInternalCollectGarbage", js_gc, 
                    1, 0);
  JS_DefineFunction(cx.get(), global_obj, "brk", js_break, 
                    1, 0);
#endif
  
  // Everything is setup, so pass out the JSContext.
  *runtime = rt;
  *context = cx.release();
  return true;
}

bool JSStandaloneEngine::InitEngine(JsRunner *js_runner,
                                    NPP_t *np_instance) {
  assert(js_runner);
  assert(np_instance);
  
  // Bootstrap the JS Engine instance.
  JSRuntime *runtime;
  JSContext *context;
  if (!CreateJSEngine(js_runner, &runtime, &context)) {
    return false; 
  }
    
  // Setup the global object
  // JsRunner is the browser side.
  (np_instance)->ndata = static_cast<void *>(js_runner);
  
  // for use by the plugin side.
  (np_instance)->pdata = NULL;
  
  SpiderMonkeyNPAPIBindings::NPAPI_Storage::CreateThreadLocals(runtime, 
                                                               context, 
                                                               np_instance);
  
  return true;
}

void JSStandaloneEngine::GetNPNEntryPoints(NPNetscapeFuncs *browser_funcs) {
  assert(browser_funcs);
#ifdef BROWSER_WEBKIT
  memset(browser_funcs, 0, sizeof(GearsNPNetscapeFuncs));
  browser_funcs->size = sizeof(GearsNPNetscapeFuncs);
#else
  memset(browser_funcs, 0, sizeof(NPNetscapeFuncs));
  browser_funcs->size = sizeof(browser_funcs);
#endif
  browser_funcs->version = 1;
  browser_funcs->memalloc = SpiderMonkeyNPAPIBindings::NPN_MemAlloc;
  browser_funcs->memfree = SpiderMonkeyNPAPIBindings::NPN_MemFree;
  browser_funcs->getvalue = SpiderMonkeyNPAPIBindings::NPN_GetValue;
  browser_funcs->releasevariantvalue = 
      SpiderMonkeyNPAPIBindings::NPN_ReleaseVariantValue;
  browser_funcs->getstringidentifier = 
      SpiderMonkeyNPAPIBindings::NPN_GetStringIdentifier;
  browser_funcs->getstringidentifiers = 
      SpiderMonkeyNPAPIBindings::NPN_GetStringIdentifiers;
  browser_funcs->getintidentifier = 
      SpiderMonkeyNPAPIBindings::NPN_GetIntIdentifier;
  browser_funcs->identifierisstring = 
      SpiderMonkeyNPAPIBindings::NPN_IdentifierIsString;
  browser_funcs->utf8fromidentifier = 
      SpiderMonkeyNPAPIBindings::NPN_UTF8FromIdentifier;
#ifdef BROWSER_WEBKIT
  browser_funcs->intfromidentifier =
      reinterpret_cast<void * (*)(void *)>(
          SpiderMonkeyNPAPIBindings::NPN_IntFromIdentifier);
#else
  browser_funcs->intfromidentifier =
      SpiderMonkeyNPAPIBindings::NPN_IntFromIdentifier;
#endif
  browser_funcs->createobject = SpiderMonkeyNPAPIBindings::NPN_CreateObject;
  browser_funcs->retainobject = SpiderMonkeyNPAPIBindings::NPN_RetainObject;
  browser_funcs->releaseobject = SpiderMonkeyNPAPIBindings::NPN_ReleaseObject;
  browser_funcs->invoke = SpiderMonkeyNPAPIBindings::NPN_Invoke;
  browser_funcs->invokeDefault = SpiderMonkeyNPAPIBindings::NPN_InvokeDefault;
  browser_funcs->evaluate = SpiderMonkeyNPAPIBindings::NPN_Evaluate;
  browser_funcs->getproperty = SpiderMonkeyNPAPIBindings::NPN_GetProperty;
  browser_funcs->setproperty = SpiderMonkeyNPAPIBindings::NPN_SetProperty;
  browser_funcs->removeproperty = SpiderMonkeyNPAPIBindings::NPN_RemoveProperty;
  browser_funcs->hasproperty = SpiderMonkeyNPAPIBindings::NPN_HasProperty;
  browser_funcs->hasmethod = SpiderMonkeyNPAPIBindings::NPN_HasMethod;
  browser_funcs->setexception = SpiderMonkeyNPAPIBindings::NPN_SetException;
#ifdef BROWSER_WEBKIT
  ((GearsNPNetscapeFuncs *)browser_funcs)->enumerate = 
                              SpiderMonkeyNPAPIBindings::NPN_Enumerate;
#else
  browser_funcs->enumerate = SpiderMonkeyNPAPIBindings::NPN_Enumerate;
#endif
}

bool JSStandaloneEngine::TerminateEngine() {
  JSContext * context = 
      SpiderMonkeyNPAPIBindings::NPAPI_Storage::GetCurrentJSContext();
  JSRuntime *runtime =
      SpiderMonkeyNPAPIBindings::NPAPI_Storage::GetsJSRuntime();
  JS_DestroyContext(context);
  JS_DestroyRuntime(runtime);
  SpiderMonkeyNPAPIBindings::NPAPI_Storage::ClearJSContext();
  return true;
}
