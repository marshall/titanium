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

#ifndef GEARS_FACTORY_FACTORY_IE_H__
#define GEARS_FACTORY_FACTORY_IE_H__

#include <objsafe.h>
#include "gears/base/common/base_class.h"
#include "gears/base/common/common.h"
#include "gears/base/ie/resource.h"  // for .rgs resource ids (IDR_*)
#include "gears/factory/factory_impl.h"
#include "genfiles/interfaces.h"


// This is just a thin COM wrapper around a Dispatcher-backed
// GearsFactoryImpl instance.
class ATL_NO_VTABLE GearsFactory
    : public CComObjectRootEx<CComMultiThreadModel>,
      public CComCoClass<GearsFactory, &CLSID_GearsFactory>,
      public IObjectWithSiteImpl<GearsFactory>,
      public IObjectSafetyImpl<GearsFactory,
                               INTERFACESAFE_FOR_UNTRUSTED_CALLER +
                               INTERFACESAFE_FOR_UNTRUSTED_DATA>,
      public IDispatch {
 public:
  BEGIN_COM_MAP(GearsFactory)
    COM_INTERFACE_ENTRY(IObjectWithSite)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY(IDispatch)
  END_COM_MAP()

  DECLARE_PROTECT_FINAL_CONSTRUCT()

  DECLARE_REGISTRY_RESOURCEID(IDR_GEARSFACTORY)
  // End boilerplate code.

  GearsFactory() {}

  // IDispatch: GetTypeInfoCount, GetTypeInfo, GetIDsOfNames, Invoke.
  STDMETHOD(GetTypeInfoCount)(unsigned int FAR* retval);
  STDMETHOD(GetTypeInfo)(unsigned int index, LCID lcid,
                         ITypeInfo FAR* FAR* retval);
  STDMETHOD(GetIDsOfNames)(REFIID iid, OLECHAR FAR* FAR* names,
                           unsigned int num_names, LCID lcid, 
                           DISPID FAR* retval);
  STDMETHOD(Invoke)(DISPID member_id, REFIID iid, LCID lcid, WORD flags,
                    DISPPARAMS FAR* params, VARIANT FAR* retval,
                    EXCEPINFO FAR* exception,
                    unsigned int FAR* arg_error_index);

  // IObjectWithSite: SetSite.
  STDMETHOD(SetSite)(IUnknown *site);

 private:
#ifdef WINCE
  static const std::string16 kGetBuildInfo;
  static const std::string16 kPrivateSetGlobalObject;
  static const std::string16 kUninitializedGearsFactoryImpl;
#endif

  scoped_refptr<GearsFactoryImpl> factory_impl_;

  DISALLOW_EVIL_CONSTRUCTORS(GearsFactory);
};

OBJECT_ENTRY_AUTO(__uuidof(GearsFactory), GearsFactory)

#endif  // GEARS_FACTORY_FACTORY_IE_H__
