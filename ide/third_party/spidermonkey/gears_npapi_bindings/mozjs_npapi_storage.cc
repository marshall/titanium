// Copyright 2008, Google Inc.
//
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
#include "gears/base/common/common.h"
#include "gears/base/common/thread_locals.h"
#include "third_party/spidermonkey/gears_npapi_bindings/mozjs_npapi_storage.h"

namespace SpiderMonkeyNPAPIBindings {

// Storage for per-thread data.
typedef struct {
  JSContext *ctx;
  JSRuntime *runtime;
  NPP global_object;
  PLDHashTable js_obj_wrappers;
  PLDHashTable np_obj_wrappers;
  int32 wrapper_count;
  char *npp_exception;
} NPAPI_Data;

static const ThreadLocals::Slot kNPAPIThreadLocalKey = ThreadLocals::Alloc();

static NPAPI_Data *GetNPAPIData() {
  NPAPI_Data *ret =static_cast<NPAPI_Data *>(ThreadLocals::GetValue(
                                                 kNPAPIThreadLocalKey));
  assert(ret);
  return ret;
}

void NPAPI_Storage::CreateThreadLocals(JSRuntime *js_runtime, 
                                       JSContext *js_context,
                                       NPP global_object) {
  NPAPI_Data *data = static_cast<NPAPI_Data *>(calloc(1, sizeof(NPAPI_Data)));
  if (!data) {
    return;
  }
  
  data->runtime = js_runtime;
  data->ctx = js_context;
  data->global_object = global_object;
  
  ThreadLocals::SetValue(kNPAPIThreadLocalKey,
                         data,
                         NPAPI_Storage::ReleaseThreadLocals);
}

void NPAPI_Storage::ClearJSContext() {
  NPAPI_Data *npapi_data = GetNPAPIData();
  npapi_data->ctx = NULL;
  npapi_data->runtime = NULL;
}

void NPAPI_Storage::ReleaseThreadLocals(void *data) {
  NPAPI_Data *npapi_data = static_cast<NPAPI_Data *>(data);
  
  if (npapi_data->npp_exception) {
    free(npapi_data->npp_exception);
  }
  
  free(data);
}

JSContext * NPAPI_Storage::GetCurrentJSContext()  {
  return GetNPAPIData()->ctx;
}

NPP NPAPI_Storage::GetGlobalObject()  {
  // Mozilla's NPAPI bindings store the global object seperately from the
  // context, we extract this data from the js_context so it'll be easier to
  // separate if we need to change to Mozilla's way of doing things in the
  // future.
  return GetNPAPIData()->global_object;
}

PLDHashTable & NPAPI_Storage::GetsJSObjWrappers() {
  return GetNPAPIData()->js_obj_wrappers;
}

PLDHashTable & NPAPI_Storage::GetsNPObjWrappers() {
  return GetNPAPIData()->np_obj_wrappers;
}

int32 * NPAPI_Storage::GetsWrapperCount() {
  return &(GetNPAPIData()->wrapper_count);
}

JSRuntime * NPAPI_Storage::GetsJSRuntime() {
  NPAPI_Data *data = GetNPAPIData();
  return data->runtime;
}

char ** NPAPI_Storage::GetgNPPException() {
  return &(GetNPAPIData()->npp_exception);
}

} // namespace SpiderMonkeyNPAPIBindings
