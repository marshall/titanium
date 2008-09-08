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

// Safari will unload plugins when it exits, which unmaps the memory
// associated with that plugin.
// This can cause a crash when using Gears since worker threads can
// keep running after the memory has been unmapped.
// To solve this, we use a proxy library that then loads the real Gears
// code and calls through to it, this way only the proxy is unloaded
// and the actual Gears code remains in memory so that threads can
// shutdown gracefully.

#include <Cocoa/Cocoa.h>
#include <WebKit/npfunctions.h>

// NPAPI entry points called by Safari.
extern "C" {
#define EXPORT __attribute__((visibility("default")))
  EXPORT NPError NP_Initialize(NPNetscapeFuncs *browserFuncs);
  EXPORT NPError NP_GetEntryPoints(NPPluginFuncs *pluginFuncs);
  EXPORT void NP_Shutdown(void);
#undef EXPORT
}

// A stub obj-c object implmeneted in safari_workarounds.m, simply calls
// through to Gears' NPAPI entry points.
@interface NPAPIStub : NSObject
+ (NPError)NP_Initialize:(NPNetscapeFuncs *)browserFuncs;
+ (NPError)NP_GetEntryPoints:(NPPluginFuncs *)pluginFuncs;
+ (void)NP_Shutdown;
@end

Class gNPAPIStub = nil;

NPError NP_Initialize(NPNetscapeFuncs *browserFuncs) {
  assert(gNPAPIStub);
  if (gNPAPIStub) {
    return [gNPAPIStub NP_Initialize:browserFuncs];
  } else {
    return NPERR_MODULE_LOAD_FAILED_ERROR;
  }
}

NPError NP_GetEntryPoints(NPPluginFuncs *pluginFuncs) {
  assert(gNPAPIStub);
  if (gNPAPIStub) {
    return [gNPAPIStub NP_GetEntryPoints:pluginFuncs];
  } else {
    return NPERR_MODULE_LOAD_FAILED_ERROR;
  }

}

void NP_Shutdown(void) {
  assert(gNPAPIStub);
  if (gNPAPIStub) {
    [gNPAPIStub NP_Shutdown];
  }
}

// Empty class, so we can use obj-c to get the path to the current bundle.
@interface GearsProxyClass : NSObject
@end

@implementation GearsProxyClass
@end

// This function is called by the dynamic loader when the DLL is loaded.
static void __attribute__ ((__constructor__)) on_load(void) {
  // Load Gears.bndl in our resource directory.
  NSBundle *main_bundle = [NSBundle bundleForClass:[GearsProxyClass class]];
  NSString *gears_bndl_path = [main_bundle pathForResource:@"Gears" 
                                                    ofType:@"bundle"];
  if (!gears_bndl_path) return;
  
  NSBundle *bndl = [[NSBundle alloc] initWithPath:gears_bndl_path];
  if ([bndl load] == YES) {
    gNPAPIStub = NSClassFromString(@"NPAPIStub");
  }
  [bndl release];
}
