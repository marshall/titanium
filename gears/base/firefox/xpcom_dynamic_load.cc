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

#include <assert.h>
#include <gecko_sdk/include/prlink.h>

#include "gears/base/firefox/xpcom_dynamic_load.h"


// Shared routine that handles dynamic loading
//
// We do not call PR_FreeLibraryName. This is fine; our module stays loaded
// for the lifetime of the browser.
PRFuncPtr DynamicLoad(const char *module, const char *function) {
  PRFuncPtr function_ptr = NULL;

  char *lib_name = PR_GetLibraryName(NULL, module); // get OS-specific name
  if (lib_name) {
    PRLibrary *lib = PR_LoadLibrary(lib_name);
    if (lib) {
      function_ptr = PR_FindFunctionSymbol(lib, function);
    }
  }

  return function_ptr;
}


// Initialization of global variables
XPTC_InvokeByIndex_Type XPTC_InvokeByIndex_DynLoad =
#if BROWSER_FF3
  NS_InvokeByIndex;
#elif defined(OS_MACOSX)
// For OSX, the libxpcom_core is already linked, so we can access the symbol
// directly
  XPTC_InvokeByIndex;
#else
  reinterpret_cast<XPTC_InvokeByIndex_Type>(DynamicLoad("xpcom_core",
                                                        "XPTC_InvokeByIndex"));
#endif
