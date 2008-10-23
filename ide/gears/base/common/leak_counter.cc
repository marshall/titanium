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

#include "gears/base/common/leak_counter.h"

#if ENABLE_LEAK_COUNTING

#include <assert.h>
#include <stdio.h>
#include <string>

#include "gears/base/common/atomic_ops.h"
#include "gears/base/common/basictypes.h"
#include "gears/base/common/string16.h"
#include "gears/base/common/string_utils.h"
#include "genfiles/product_constants.h"

static AtomicWord leak_counter_counters[MAX_LEAK_COUNTER_TYPE];

static const char16 *leak_counter_names[] = {
  STRING16(L"DocumentJsRunner"),
  STRING16(L"FFHttpRequest"),
  STRING16(L"JavaScriptWorkerInfo"),
  STRING16(L"JsArray"),
  STRING16(L"JsCallContext"),
  STRING16(L"JsContextWrapper"),
  STRING16(L"JsDomElement"),
  STRING16(L"JsEventMonitor"),
  STRING16(L"JsObject"),
  STRING16(L"JsRootedToken"),
  STRING16(L"JsRunner"),
  STRING16(L"JsWrapperDataForFunction"),
  STRING16(L"JsWrapperDataForInstance"),
  STRING16(L"JsWrapperDataForProto"),
  STRING16(L"ModuleEnvironment"),
  STRING16(L"ModuleImplBaseClass"),
  STRING16(L"ModuleWrapper"),
  STRING16(L"PoolThreadsManager"),
  STRING16(L"ProgressInputStream"),
  STRING16(L"SafeHttpRequest"),
  STRING16(L"SharedJsClasses"),
  NULL
};

void LeakCounterDumpCounts() {
  int total = 0;
  for (int i = 0; i < MAX_LEAK_COUNTER_TYPE; i++) {
    total += leak_counter_counters[i];
  }
  if (total == 0) {
    return;
  }
#ifdef WINCE
  // TODO(nigeltao): figure out some sort of UI for showing leaks on WinCE.
#else

  std::string16 s(STRING16(PRODUCT_FRIENDLY_NAME));
  s += STRING16(L" is leaking memory. Known leaks include ");
  s += IntegerToString16(total);
  s += STRING16(L" objects:\n");
  for (int i = 0; i < MAX_LEAK_COUNTER_TYPE; ++i) {
    int count = leak_counter_counters[i];
    if (count == 0) {
      continue;
    }
    s += STRING16(L"    ");
    s += IntegerToString16(count);
    s += STRING16(L" ");
    s += leak_counter_names[i];
    s += STRING16(L"\n");
  }
#ifdef WIN32
  ::MessageBox(0, s.c_str(), PRODUCT_FRIENDLY_NAME, MB_OK);
#endif
#ifdef LINUX
  std::string s_as_utf8;
  String16ToUTF8(s, &s_as_utf8);
  printf("%s", s_as_utf8.c_str());
#endif
#endif  // WINCE
}

void LeakCounterIncrement(LeakCounterType type, int delta) {
  AtomicIncrement(&leak_counter_counters[type], delta);
}

void LeakCounterInitialize() {
  // The +1 is for the NULL value at the end of leak_counter_names.
  assert(MAX_LEAK_COUNTER_TYPE + 1 == ARRAYSIZE(leak_counter_names));
  for (int i = 0; i < MAX_LEAK_COUNTER_TYPE; i++) {
    leak_counter_counters[i] = 0;
  }
}

#endif  // ENABLE_LEAK_COUNTING
