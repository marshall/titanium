//
// Copyright 2006-2008 Appcelerator, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include "base/scoped_ptr.h"
#include "webkit/glue/webpreferences.h"

#ifdef TIDEBUG
#define ti_debug(s) ti_debug_internal(s, __FILE__, __LINE__);
#else
#define ti_debug(s)
#endif


void ti_debug_internal(char *message, char *filename, int line);
WebPreferences ti_initWebPrefs();

void systemError(const wchar_t *message);

