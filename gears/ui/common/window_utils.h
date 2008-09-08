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

#ifndef GEARS_UI_COMMON_WINDOW_UTILS_H__
#define GEARS_UI_COMMON_WINDOW_UTILS_H__

#include "gears/base/common/common.h"

#if defined(WIN32)
typedef HWND NativeWindowPtr;
#elif defined(OS_MACOSX)
typedef WindowPtr NativeWindowPtr;
#elif defined(LINUX)
#include <gtk/gtkwindow.h>
typedef GtkWindow* NativeWindowPtr;
#else
typedef void* NativeWindowPtr;
#endif

class ModuleImplBaseClass;

// Obtains a native window handle for the browser.  For reasons detailed in the
// implementation, on Mac Safari, this method has a side-effect of giving
// keyboard focus to the browser window.
bool GetBrowserWindow(const ModuleImplBaseClass* module,
                      NativeWindowPtr* window);

#endif  // GEARS_UI_COMMON_WINDOW_UTILS_H__
