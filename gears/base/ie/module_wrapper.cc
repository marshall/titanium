// Copyright 2007, Google Inc.
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

#include "gears/base/ie/module_wrapper.h"

#include "gears/base/ie/dispatcher_to_idispatch.h"

STDMETHODIMP ModuleWrapper::GetTypeInfoCount(unsigned int FAR* retval) {
  return DispatcherGetTypeInfoCount(dispatcher_.get(), retval);
}

STDMETHODIMP ModuleWrapper::GetTypeInfo(unsigned int index, LCID lcid,
                                        ITypeInfo FAR* FAR* retval) {
  return DispatcherGetTypeInfo(dispatcher_.get(), index, lcid, retval);
}

STDMETHODIMP ModuleWrapper::GetIDsOfNames(REFIID iid, OLECHAR FAR* FAR* names,
                                          unsigned int num_names, LCID lcid, 
                                          DISPID FAR* retval) {
  return DispatcherGetIDsOfNames(dispatcher_.get(), iid, names, num_names,
                                 lcid, retval);
}

STDMETHODIMP ModuleWrapper::Invoke(DISPID member_id, REFIID iid, LCID lcid,
                                   WORD flags, DISPPARAMS FAR* params,
                                   VARIANT FAR* retval,
                                   EXCEPINFO FAR* exception,
                                   unsigned int FAR* arg_error_index) {
  return DispatcherInvoke(dispatcher_.get(), member_id, iid, lcid, flags,
                          params, retval, exception, arg_error_index);
}

STDMETHODIMP ModuleWrapper::get_moduleWrapper(VARIANT *retval) {
  // We pack the pointer into the byref field of a VARIANT.
  retval->vt = VT_BYREF;
  retval->byref = this;
  return S_OK;
}
