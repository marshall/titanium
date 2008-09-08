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

// This file contains workarounds and documentation for issues with WebKit and
// OSX that affect Gears.

// OS X Bugs affecting Gears:

// -----------------------------------------------------------------------------
// rdar://problem/6116708 - NSURLConnection will sometimes abort with no
//                          delegate callback in OS X 10.5 & 10.4.
// The conditions under which this bug is tickled are unlikely to be
// seen in the wild.  Our testwebserver.py script has been fixed so that
// it no longer causes us to run into this problem.
// Also see gears issue 568.
// -----------------------------------------------------------------------------
// rdar://problem/5830581 - NSURLProtocol not called on redirect.
//  Our NSURLProtocol handler isn't called if a URL redirects to another URL
// in the Gears Cache.
// -----------------------------------------------------------------------------
// rdar://problem/5817126 - NSURLProtocol can't return NSHTTPURLResponse under
//                          Leopard.
//  This is a regression in Leopard (works fine in Tiger) - If a NSURLProtocol
// returns a class other than NSURLResponse, the system casts it down to a plain
// NSURLResponse.  This means that we can't feed custom HTTP data to Safari.
//
// Workaround: Add allHeaderFields & statusCode selectors to all instances of
// NSURLResponse.
// -----------------------------------------------------------------------------
// Attempting to download a URL synchronously from a call that blocks JS wedges
// the URL loading mechanism in OS X 10.4.
//
// Workaround: Use libcurl instead of an NSURLConnection in these cases (see
// desktop/curl_icon_downloader.h.

// WebKit Bugs affecting Gears:

// -----------------------------------------------------------------------------
// Bug 16829 - NPN_SetException() sets exception on the Global ExecState 
//             instead of local.
//   This means that we can't use NPN_SetException() to throw exceptions from
// our plugin.
//
// Workaround: use WebScriptObject's throwException: selector instead - see
// common_sf.mm.
// -----------------------------------------------------------------------------
// Bug 18234: JS exception thrown from NPN_InvokeDefault not shown in 
//            error console.
//   If we use NPN_InvokeDefault() to call a JS callback from Gears, and that
// callback throws an exception, the exception isn't bubbled up into the
// the originating JS code - it effectively gets lost.
// -----------------------------------------------------------------------------
// Bug 8519: WebCore doesn't fire window.onerror event.
//  WebKit doesn't support window.onerror(), this affects our unit tests which
// rely on window.onerror to catch failing unit tests.
//   This effectively means that a unit tests that fails with an exception must
// timeout before we continue.
// -----------------------------------------------------------------------------
// Bug 18333: NPAPI: No way of telling the difference between 'ints' and 
//           'doubles'.
//  NPVARIANT_IS_INT32() never returns True in WebKit, this means that we have
// no way of telling the difference between JS calling us with a numeric
// parameter of 42 or 42.0.
//  This bug affects our ability to stringify numbers.
//
// Workaround: We manually check if a double fits in the bounds of an int, and
// if so we manually report it's type as an int, see JsTokenGetType() in
// js_types.cc.
// -----------------------------------------------------------------------------
// Webkit bug 18941: If you display a Modal Webview from inside a JS timer
//                   callback, then WebKit's native timers don't fire.
//                   Since these are used for things like layout, nothing
//                   is displayed.
//
// Workaround: Call WebCore::TimerBase::fireTimersInNestedEventLoop() which
// enables timers in a nested runloop which makes the WebView display content.
// See proposed patch for this bug:
// https://bugs.webkit.org/attachment.cgi?id=21681&action=prettypatch

#import <dlfcn.h>
#import <Cocoa/Cocoa.h>
#import <mach-o/nlist.h>
#include <WebKit/npfunctions.h>

// Workaround for Webkit bug 18941. 
// Returns address of WebCore::TimerBase::fireTimersInNestedEventLoop() static
// function defined in WebCore library.
// Returns NULL on error.
static void *GetFireTimerFunc() {
  // Find the base address of the WebCore library.
  // The WebScriptObject class is defined in the library.
  Class c =  NSClassFromString(@"WebScriptObject");
  Dl_info addr;
  if (dladdr(c, &addr) == 0) return NULL;

  // Open the library binary and find the address of the symbol.
  // dli_fname contains the full path of the WebCore library.
  const char *module_name = addr.dli_fname;
  struct nlist nl[2];
  memset(&nl, 0, sizeof(nl));
  // Mangled version of WebCore::TimerBase::fireTimersInNestedEventLoop() as
  // reported by nm.
  nl[0].n_un.n_name = "__ZN7WebCore9TimerBase27fireTimersInNestedEventLoopEv";
 
  if (nlist(module_name, nl) != 0) return NULL;
  if (nl[0].n_type == N_UNDF) return NULL;
  vm_offset_t relative_address = (vm_offset_t)nl[0].n_value;
  
  SInt32 os_version;
  if (Gestalt(gestaltSystemVersion, &os_version) != noErr) return NULL;
  
  // It seems that on 10.4 nlist returns an absolute address, while on
  // 10.5 a relative address from the start of the module is returned.
  void *webcore_base_addr = 0;
  if (os_version >= 0x1050) {
    webcore_base_addr = addr.dli_fbase;
  }
  
  return webcore_base_addr + relative_address;
}

// Calls through WebCore::TimerBase::fireTimersInNestedEventLoop() in WebCore.
void EnableWebkitTimersForNestedRunloop() {
  static void *func_ptr = 0;
  
  if (!func_ptr) {
    func_ptr = GetFireTimerFunc();
  }
  
  // Call through to function if it's defined.
  if (func_ptr) {
    void (* fireTimersInNestedEventLoop)() = func_ptr;
    fireTimersInNestedEventLoop();
  }
}

// Stub functions for NPAPI entry points, see plugin_proxy.mm for details.
NPError NP_Initialize(NPNetscapeFuncs *browserFuncs);
NPError NP_GetEntryPoints(NPPluginFuncs *pluginFuncs);
void NP_Shutdown(void);

@interface NPAPIStub : NSObject
+ (NPError)NP_Initialize:(NPNetscapeFuncs *)browserFuncs;
+ (NPError)NP_GetEntryPoints:(NPPluginFuncs *)pluginFuncs;
+ (void)NP_Shutdown;
@end

@implementation NPAPIStub
+ (NPError)NP_Initialize:(NPNetscapeFuncs *)browserFuncs {
  return NP_Initialize(browserFuncs);
}

+ (NPError)NP_GetEntryPoints:(NPPluginFuncs *)pluginFuncs {
  return NP_GetEntryPoints(pluginFuncs);
}

+ (void)NP_Shutdown {
  NP_Shutdown();
}
@end

