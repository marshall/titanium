// Copyright 2007 Igor Tandetnik
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef GEARS_THIRD_PARTY_PASSTHRU_APP_PROTOCOLCF_H__
#define GEARS_THIRD_PARTY_PASSTHRU_APP_PROTOCOLCF_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

namespace PassthroughAPP
{

namespace Detail
{
// A bit of template metaprogramming allows to avoid requiring
// the protocol class to either derive from CComCoClass or explicitly
// specify DECLARE_*AGGREGATABLE macro. If the class does it, the declared
// behavior is used, otherwise the default behavior is as if
// DECLARE_AGGREGATABLE is specified

// If T has a typedef _CreatorClass, Derived would pick it from its base.
// If T does not define _CreatorClass, the definition is picked from the next
// enclosing scope, which is ChooseCreatorClass::_CreatorClass, or Default
template <typename T, typename Default>
struct ChooseCreatorClass
{
  typedef Default _CreatorClass;
  struct Derived : private T
  {
    typedef _CreatorClass CreatorClass;
  };
  typedef typename Derived::CreatorClass CreatorClass;
};

} // end namespace PassthroughAPP::Detail

class ATL_NO_VTABLE CComClassFactoryProtocol :
  public CComClassFactory
{
  typedef CComClassFactory BaseClass;
public:
  STDMETHODIMP CreateInstance(IUnknown* punkOuter, REFIID riid,
    void** ppvObj);

  HRESULT CreateInstanceTarget(IUnknown** ppTargetProtocol);

  HRESULT GetTargetClassFactory(IClassFactory** ppCF);
  HRESULT SetTargetClassFactory(IClassFactory* pCF);
  HRESULT SetTargetCLSID(REFCLSID clsid, DWORD clsContext = CLSCTX_ALL);

  void FinalRelease();
private:
  CComPtr<IClassFactory> m_spTargetCF;
};

template <class Factory, class Protocol,
  class FactoryComObject = CComObjectNoLock<Factory> >
struct CMetaFactory
{
  typedef
    CComCreator2<CComCreator<CComObject<Protocol> >,
      CComCreator<CComAggObject<Protocol> > >
  DefaultCreatorClass;

  typedef typename
    Detail::ChooseCreatorClass<Protocol, DefaultCreatorClass>::CreatorClass
      CreatorClass;

  // returns a non-AddRef'ed pointer to FactoryComObject, already initialized
  // with Protocol's creator function, via ppObj [out] parameter
  static HRESULT CreateInstance(Factory** ppObj);

  static HRESULT CreateInstance(IClassFactory* pTargetCF,
    IClassFactory** ppCF);
  static HRESULT CreateInstance(REFCLSID clsidTarget, IClassFactory** ppCF);
};

} // end namespace PassthroughAPP

#include "ProtocolCF.inl"

#endif  // GEARS_THIRD_PARTY_PASSTHRU_APP_PROTOCOLCF_H__
