/* ***** BEGIN LICENSE BLOCK *****
 * Version: NPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the NPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the NPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

//////////////////////////////////////////////////////////////
//
// Main plugin entry point implementation
//
#include "gears/base/common/base_class.h"
#ifdef BROWSER_WEBKIT
#include "gears/base/common/common_sf.h"
#endif
#include "gears/base/common/thread_locals.h"
#ifdef BROWSER_WEBKIT
#include "gears/base/safari/npapi_patches.h" 
#endif
#include "gears/base/npapi/module.h"

#ifndef HIBYTE
#define HIBYTE(x) ((((uint32)(x)) & 0xff00) >> 8)
#endif

#ifdef BROWSER_WEBKIT
static bool g_allow_npinit = true;
#else
static bool g_allow_npinit = false;
#endif

void AllowNPInit(bool allow) {
  g_allow_npinit = allow;
}

// Export NPAPI entry points on OS X.
#ifdef BROWSER_WEBKIT

// TODO(mpcomplete): remove this if BROWSER_WEBKIT starts using the standard
// headers.

#define STDCALL

extern "C" {
  // Mach-o entry points
#define EXPORT __attribute__((visibility("default")))
  EXPORT NPError NP_Initialize(NPNetscapeFuncs *browserFuncs);
  EXPORT NPError NP_GetEntryPoints(NPPluginFuncs *pluginFuncs);
  EXPORT void NP_Shutdown(void);
#undef EXPORT
  // For compatibility with NPAPI in Opera & FF on the Mac, we need to implement
  // this.
  // int main(NPNetscapeFuncs *browserFuncs, NPPluginFuncs *pluginFuncs,
  //         void *shutdown);
}
#endif  // BROWSER_WEBKIT


// Store the browser functions in thread local storage to avoid calling the
// functions on a different thread.
#ifdef BROWSER_WEBKIT
// Work around a bug in the WebKit NPAPI headers, see 
// gears/base/safari/npapi_patches.h for details.
static GearsNPNetscapeFuncs g_browser_funcs;
#else
static NPNetscapeFuncs g_browser_funcs;
#endif
const ThreadLocals::Slot kNPNFuncsKey = ThreadLocals::Alloc();

NPError STDCALL NP_GetEntryPoints(NPPluginFuncs* funcs)
{
  if (funcs == NULL)
    return NPERR_INVALID_FUNCTABLE_ERROR;

  // On FF & Safari on win32, funcs->size is a parameter we need to check on
  // input in order to make sure we're being passed a structure of the right
  // size.
  //
  // Webkit under OSX on the other hand, passes 0 in funcs->size.
  // Apple's sample code (NetscapeMoviePlugIn) treats this as an output
  // parameter.
  //
  // We play it safe by being consistent with Apple's example code under OSX &
  // keeping with the standard behavior otherwise.
#ifdef BROWSER_WEBKIT
  funcs->size          = sizeof(NPPluginFuncs);
#else
  if (funcs->size < sizeof(NPPluginFuncs))
    return NPERR_INVALID_FUNCTABLE_ERROR;
#endif
  funcs->version       = (NP_VERSION_MAJOR << 8) | NP_VERSION_MINOR;
  funcs->newp          = NPP_New;
  funcs->destroy       = NPP_Destroy;
  funcs->setwindow     = NPP_SetWindow;
  funcs->newstream     = NPP_NewStream;
  funcs->destroystream = NPP_DestroyStream;
  funcs->asfile        = NPP_StreamAsFile;
  funcs->writeready    = NPP_WriteReady;
#ifdef BROWSER_WEBKIT
  funcs->write         = reinterpret_cast<NPP_WriteProcPtr>(NPP_Write);
#else
  funcs->write         = NPP_Write;
#endif
  funcs->print         = NPP_Print;
  funcs->event         = NPP_HandleEvent;
  funcs->urlnotify     = NPP_URLNotify;
  funcs->getvalue      = NPP_GetValue;
  funcs->setvalue      = NPP_SetValue;
  funcs->javaClass     = NULL;

  return NPERR_NO_ERROR;
}

NPError STDCALL NP_Initialize(NPNetscapeFuncs* funcs)
{
  if (!g_allow_npinit)
    return NPERR_INCOMPATIBLE_VERSION_ERROR;

  if (funcs == NULL)
    return NPERR_INVALID_FUNCTABLE_ERROR;

  if (HIBYTE(funcs->version) > NP_VERSION_MAJOR)
    return NPERR_INCOMPATIBLE_VERSION_ERROR;

  if (funcs->size < sizeof(NPNetscapeFuncs))
    return NPERR_INVALID_FUNCTABLE_ERROR;

#ifdef BROWSER_WEBKIT
  assert(funcs->size >= sizeof(g_browser_funcs));
  g_browser_funcs = *(GearsNPNetscapeFuncs *)funcs;
#else
  g_browser_funcs = *funcs;
#endif

// NPN_SetException is buggy in WebKit, see 
// http://bugs.webkit.org/show_bug.cgi?id=16829
#ifdef BROWSER_WEBKIT
  g_browser_funcs.setexception = WebKitNPN_SetException;
#endif
  
  ThreadLocals::SetValue(kNPNFuncsKey, &g_browser_funcs, NULL);

  return NPERR_NO_ERROR;
}


// Apple's NetscapeMoviePlugin Example defines NP_Shutdown this as returning a
// void. Gecko defines this differently.
#ifdef BROWSER_WEBKIT
void STDCALL NP_Shutdown() {
  return;
}
#else
NPError STDCALL NP_Shutdown() {
  return NPERR_NO_ERROR;
}
#endif


#ifdef WIN32
BOOL MyDllMain(HANDLE instance, DWORD reason, LPVOID reserved) {
  switch (reason) {
    case DLL_THREAD_DETACH:
      ThreadLocals::HandleThreadDetached();
      break;
    case DLL_PROCESS_DETACH:
      ThreadLocals::HandleProcessDetached();
      break;
    case DLL_PROCESS_ATTACH:
      ThreadLocals::HandleProcessAttached();
      break;
  }

  return TRUE;
}

extern "C"
BOOL WINAPI DllMain(HANDLE instance, DWORD reason, LPVOID reserved) {
 return MyDllMain(instance, reason, reserved);
}
#endif
