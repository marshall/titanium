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

#ifndef GEARS_BASE_COMMON_LEAK_COUNTER_H__
#define GEARS_BASE_COMMON_LEAK_COUNTER_H__

#ifdef DEBUG
#if BROWSER_FF || BROWSER_IE
#ifdef WINCE
// TODO(nigeltao): figure out some sort of UI for showing leaks on WinCE.
#else
#define ENABLE_LEAK_COUNTING 1
#endif
#endif
#endif


#if ENABLE_LEAK_COUNTING

// When adding new LeakCounterTypes, please keep them in alphabetical order,
// and also update the leak_counter_names array in leak_counter.cc.
enum LeakCounterType {
  LEAK_COUNTER_TYPE_DocumentJsRunner,
  LEAK_COUNTER_TYPE_FFHttpRequest,
  LEAK_COUNTER_TYPE_JavaScriptWorkerInfo,
  LEAK_COUNTER_TYPE_JsArray,
  LEAK_COUNTER_TYPE_JsCallContext,
  LEAK_COUNTER_TYPE_JsContextWrapper,
  LEAK_COUNTER_TYPE_JsDomElement,
  LEAK_COUNTER_TYPE_JsEventMonitor,
  LEAK_COUNTER_TYPE_JsObject,
  LEAK_COUNTER_TYPE_JsRootedToken,
  LEAK_COUNTER_TYPE_JsRunner,
  LEAK_COUNTER_TYPE_JsWrapperDataForFunction,
  LEAK_COUNTER_TYPE_JsWrapperDataForInstance,
  LEAK_COUNTER_TYPE_JsWrapperDataForProto,
  LEAK_COUNTER_TYPE_ModuleEnvironment,
  LEAK_COUNTER_TYPE_ModuleImplBaseClass,
  LEAK_COUNTER_TYPE_ModuleWrapper,
  LEAK_COUNTER_TYPE_PoolThreadsManager,
  LEAK_COUNTER_TYPE_ProgressInputStream,
  LEAK_COUNTER_TYPE_SafeHttpRequest,
  LEAK_COUNTER_TYPE_SharedJsClasses,
  MAX_LEAK_COUNTER_TYPE
};

void LeakCounterDumpCounts();
void LeakCounterIncrement(LeakCounterType type, int delta);
void LeakCounterInitialize();

#define LEAK_COUNTER_DUMP_COUNTS() LeakCounterDumpCounts()
#define LEAK_COUNTER_DECREMENT(name) \
    LeakCounterIncrement(LEAK_COUNTER_TYPE_##name, -1)
#define LEAK_COUNTER_INCREMENT(name) \
    LeakCounterIncrement(LEAK_COUNTER_TYPE_##name, +1)
#define LEAK_COUNTER_INITIALIZE() LeakCounterInitialize()

#else   // ENABLE_LEAK_COUNTING

#define LEAK_COUNTER_DUMP_COUNTS()    do {} while (false)
#define LEAK_COUNTER_DECREMENT(name)  do {} while (false)
#define LEAK_COUNTER_INCREMENT(name)  do {} while (false)
#define LEAK_COUNTER_INITIALIZE()     do {} while (false)

#endif  // ENABLE_LEAK_COUNTING
#endif  // GEARS_BASE_COMMON_LEAK_COUNTER_H__
