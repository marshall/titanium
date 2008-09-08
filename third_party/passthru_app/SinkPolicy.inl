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

#ifndef GEARS_THIRD_PARTY_PASSTHRU_APP_SINKPOLICY_INL__
#define GEARS_THIRD_PARTY_PASSTHRU_APP_SINKPOLICY_INL__

#if _MSC_VER > 1000
  #pragma once
#endif // _MSC_VER > 1000

#ifndef GEARS_THIRD_PARTY_PASSTHRU_APP_SINKPOLICY_H__
  #error SinkPolicy.inl requires SinkPolicy.h to be included first
#endif

namespace PassthroughAPP
{

// ===== NoSinkStartPolicy =====

inline HRESULT NoSinkStartPolicy::OnStart(LPCWSTR szUrl,
  IInternetProtocolSink *pOIProtSink, IInternetBindInfo *pOIBindInfo,
  DWORD grfPI, HANDLE_PTR dwReserved,
  IInternetProtocol* pTargetProtocol) const
{
  ATLASSERT(pTargetProtocol != 0);
  return pTargetProtocol->Start(szUrl, pOIProtSink, pOIBindInfo,
    grfPI, dwReserved);
}

// gears - added by jabdelmalek
inline HRESULT NoSinkStartPolicy::OnStartEx(IUri *pUri,
    IInternetProtocolSink *pOIProtSink, IInternetBindInfo *pOIBindInfo,
    DWORD grfPI, HANDLE_PTR dwReserved,
    IInternetProtocolEx* pTargetProtocol) const
{
  ATLASSERT(pTargetProtocol != 0);
  return pTargetProtocol->StartEx(pUri, pOIProtSink, pOIBindInfo,
      grfPI, dwReserved);
}

// ===== CComObjectSharedRef =====

template<class Base>
inline CComObjectSharedRef<Base>::
  CComObjectSharedRef(IUnknown* punkRefCount, IUnknown*) :
    m_punkRefCount(punkRefCount)
{
  ATLASSERT(punkRefCount != 0);
}

#ifdef _ATL_DEBUG_INTERFACES
template<class Base>
inline CComObjectSharedRef<Base>::~CComObjectSharedRef()
{
#if _ATL_VER < 0x700
  _Module.DeleteNonAddRefThunk(_GetRawUnknown());
#else
  _AtlDebugInterfacesModule.DeleteNonAddRefThunk(_GetRawUnknown());
#endif
}
#endif // _ATL_DEBUG_INTERFACES

template<class Base>
inline STDMETHODIMP CComObjectSharedRef<Base>::QueryInterface(REFIID iid,
  void** ppvObject)
{
  return _InternalQueryInterface(iid, ppvObject);
}

template<class Base>
inline STDMETHODIMP_(ULONG) CComObjectSharedRef<Base>::AddRef()
{
  if (m_punkRefCount)
  {
    m_punkRefCount->AddRef();
  }
  return InternalAddRef();
}

template<class Base>
inline STDMETHODIMP_(ULONG) CComObjectSharedRef<Base>::Release()
{
  ULONG l = InternalRelease();
  if (!l)
  {
    ReleaseAll();
  }
  if (m_punkRefCount)
  {
    m_punkRefCount->Release();
  }
  return l;
}

template<class Base>
inline Base* CComObjectSharedRef<Base>::GetContainedObject()
{
  return this;
}

template<class Base>
inline CComObjectSharedRef<Base>* CComObjectSharedRef<Base>::
  GetThisObject(const Base* pBase)
{
  ATLASSERT(pBase != 0);
  return const_cast<CComObjectSharedRef<Base>*>(
    static_cast<const CComObjectSharedRef<Base>*>(pBase));
}

// ===== CComPolyObjectSharedRef =====

template <class Contained>
inline CComPolyObjectSharedRef<Contained>::
  CComPolyObjectSharedRef(IUnknown* punkRefCount, IUnknown* punkOuter) :
    m_contained(punkOuter ? punkOuter : this),
    m_punkRefCount(punkRefCount)
{
  ATLASSERT(punkRefCount != 0);
}

#ifdef _ATL_DEBUG_INTERFACES
template <class Contained>
inline  CComPolyObjectSharedRef<Contained>::~CComPolyObjectSharedRef()
{
#if _ATL_VER < 0x700
  _Module.DeleteNonAddRefThunk(this);
#else
  _AtlDebugInterfacesModule.DeleteNonAddRefThunk(this);
#endif
}
#endif // _ATL_DEBUG_INTERFACES

template <class Contained>
inline HRESULT CComPolyObjectSharedRef<Contained>::FinalConstruct()
{
  InternalAddRef();
  CComObjectRootEx<Contained::_ThreadModel::ThreadModelNoCS>::
    FinalConstruct();
  HRESULT hr;
#if _ATL_VER >= 0x800
  hr = m_contained._AtlInitialConstruct();  // gears - added by andreip
  if (SUCCEEDED(hr))
#endif
    hr = m_contained.FinalConstruct();
#if _ATL_VER >= 0x800
  if (SUCCEEDED(hr))
    hr = m_contained._AtlFinalConstruct();  // gears - added by andreip
#endif
  InternalRelease();
  return hr;
}

template <class Contained>
inline void CComPolyObjectSharedRef<Contained>::FinalRelease()
{
  CComObjectRootEx<Contained::_ThreadModel::ThreadModelNoCS>::
    FinalRelease();
  m_contained.FinalRelease();
}

template <class Contained>
inline STDMETHODIMP CComPolyObjectSharedRef<Contained>::QueryInterface(
  REFIID iid, void** ppvObject)
{
  ATLASSERT(ppvObject != 0);
  if (!ppvObject)
  {
    return E_POINTER;
  }
  *ppvObject = NULL;

  HRESULT hr = S_OK;
  if (InlineIsEqualUnknown(iid))
  {
    if (ppvObject == NULL)
    {
      return E_POINTER;
    }
    *ppvObject = static_cast<IUnknown*>(this);
    AddRef();
#ifdef _ATL_DEBUG_INTERFACES
#if _ATL_VER < 0x700
    _Module.AddThunk((IUnknown**)ppvObject, (LPCTSTR)Contained::_GetEntries()[-1].dw, iid);
#else
    _AtlDebugInterfacesModule.AddThunk((IUnknown**)ppvObject, (LPCTSTR)Contained::_GetEntries()[-1].dw, iid);
#endif
#endif // _ATL_DEBUG_INTERFACES
  }
  else
  {
    hr = m_contained._InternalQueryInterface(iid, ppvObject);
  }
  return hr;
}

template <class Contained>
inline STDMETHODIMP_(ULONG) CComPolyObjectSharedRef<Contained>::AddRef()
{
  if (m_punkRefCount)
  {
    m_punkRefCount->AddRef();
  }
  return InternalAddRef();
}

template <class Contained>
inline STDMETHODIMP_(ULONG) CComPolyObjectSharedRef<Contained>::Release()
{
  ULONG l = InternalRelease();
  if (!l)
  {
    m_contained.ReleaseAll();
  }
  if (m_punkRefCount)
  {
    m_punkRefCount->Release();
  }
  return l;
}

template <class Contained>
inline IUnknown* CComPolyObjectSharedRef<Contained>::GetControllingUnknown()
{
  return m_contained.GetControllingUnknown();
}

template <class Contained>
Contained* CComPolyObjectSharedRef<Contained>::GetContainedObject()
{
  return &m_contained;
}

template<class Contained>
inline CComPolyObjectSharedRef<Contained>* CComPolyObjectSharedRef<Contained>::
  GetThisObject(const Contained* pContained)
{
  ATLASSERT(pContained != 0);
  const CComContainedObject<Contained>* pContainedObject =
    static_cast<const CComContainedObject<Contained>*>(pContained);
  return reinterpret_cast<CComPolyObjectSharedRef<Contained>*>(
    reinterpret_cast<DWORD_PTR>(pContainedObject) -
      offsetof(CComPolyObjectSharedRef<Contained>, m_contained));
}

// ===== CComObjectRefCount =====

template <class T, class ThreadModel>
inline STDMETHODIMP CComObjectRefCount<T, ThreadModel>::
  QueryInterface(REFIID iid, void** ppvObject)
{
  ATLASSERT(false && _T("CComObjectRefCount: QueryInterface should never be called"));
  return E_NOTIMPL;
}

template <class T, class ThreadModel>
inline STDMETHODIMP_(ULONG) CComObjectRefCount<T, ThreadModel>::AddRef()
{
  return InternalAddRef();
}

template <class T, class ThreadModel>
inline STDMETHODIMP_(ULONG) CComObjectRefCount<T, ThreadModel>::Release()
{
  ULONG l = InternalRelease();
  if (l == 0)
  {
    T* pT = reinterpret_cast<T*>(
      reinterpret_cast<DWORD_PTR>(this) - offsetof(T, m_refCount));
    delete pT;
  }
  return l;
}

// ===== CComObjectProtSink =====

template <class ProtocolObject, class SinkObject>
inline CComObjectProtSink<ProtocolObject, SinkObject>::
  CComObjectProtSink(void* pv) :
    ProtocolObject(&m_refCount, static_cast<IUnknown*>(pv)),
    m_sink(&m_refCount)
{
#if _ATL_VER < 0x700
  _Module.Lock();
#else
  _pAtlModule->Lock();
#endif
}

template <class ProtocolObject, class SinkObject>
inline CComObjectProtSink<ProtocolObject, SinkObject>::
  ~CComObjectProtSink()
{
  m_refCount.m_dwRef = 1;
  FinalRelease();
#if _ATL_VER < 0x700
#ifdef _ATL_DEBUG_INTERFACES
  _Module.DeleteNonAddRefThunk(this);
#endif
  _Module.Unlock();
#else // _ATL_VER >= 0x700
#ifdef _ATL_DEBUG_INTERFACES
  _AtlDebugInterfacesModule.DeleteNonAddRefThunk(this);
#endif
  _pAtlModule->Unlock();
#endif
}

template <class ProtocolObject, class SinkObject>
inline HRESULT CComObjectProtSink<ProtocolObject, SinkObject>::
  FinalConstruct()
{
  m_refCount.InternalAddRef();
  m_refCount.FinalConstruct();
  HRESULT hr = ProtocolObject::FinalConstruct();
  HRESULT hrSink;
#if _ATL_VER >= 0x800
  hrSink = m_sink._AtlInitialConstruct();  // gears - added by zork
  if (SUCCEEDED(hrSink))
#endif
    hrSink = m_sink.FinalConstruct();
#if _ATL_VER >= 0x800
  if (SUCCEEDED(hrSink))
    hrSink = m_sink._AtlFinalConstruct();  // gears - added by zork
#endif
  if (SUCCEEDED(hr) && FAILED(hrSink))
  {
    hr = hrSink;
  }
  m_refCount.InternalRelease();
  return hr;
}

template <class ProtocolObject, class SinkObject>
inline void CComObjectProtSink<ProtocolObject, SinkObject>::
  FinalRelease()
{
  m_sink.FinalRelease();
  ProtocolObject::FinalRelease();
  m_refCount.FinalRelease();
}

template <class ProtocolObject, class SinkObject>
inline typename CComObjectProtSink<ProtocolObject, SinkObject>::Protocol*
CComObjectProtSink<ProtocolObject, SinkObject>::
  GetProtocol(const typename CComObjectProtSink<ProtocolObject, SinkObject>::Sink* pSink)
{
  ATLASSERT(pSink != 0);
  const SinkObject* pSinkObject = SinkObject::GetThisObject(pSink);
  CComObjectProtSink* pThis = reinterpret_cast<CComObjectProtSink*>(
    reinterpret_cast<DWORD_PTR>(pSinkObject) -
      offsetof(CComObjectProtSink, m_sink));
  return pThis->ProtocolObject::GetContainedObject();
}

template <class ProtocolObject, class SinkObject>
inline typename CComObjectProtSink<ProtocolObject, SinkObject>::Sink*
CComObjectProtSink<ProtocolObject, SinkObject>::
  GetSink(const typename CComObjectProtSink<ProtocolObject, SinkObject>::Protocol* pProtocol)
{
  ATLASSERT(pProtocol != 0);
  const ProtocolObject* pProtocolObject =
    ProtocolObject::GetThisObject(pProtocol);
  CComObjectProtSink* pThis = const_cast<CComObjectProtSink*>(
    static_cast<const CComObjectProtSink*>(pProtocolObject));
  return pThis->m_sink.GetContainedObject();
}

// ===== CustomSinkStartPolicy =====

template <class Protocol, class Sink>
inline HRESULT CustomSinkStartPolicy<Protocol, Sink>::OnStart(LPCWSTR szUrl,
  IInternetProtocolSink *pOIProtSink, IInternetBindInfo *pOIBindInfo,
  DWORD grfPI, HANDLE_PTR dwReserved,
  IInternetProtocol* pTargetProtocol) const
{
  ATLASSERT(pTargetProtocol != 0);

  Sink* pSink = GetSink(static_cast<const Protocol*>(this));
  HRESULT hr = pSink->OnStart(szUrl, pOIProtSink, pOIBindInfo, grfPI,
    dwReserved, pTargetProtocol);

  CComPtr<IInternetProtocolSink> spSink;
  CComPtr<IInternetBindInfo> spBindInfo;
  if (SUCCEEDED(hr))
  {
    hr = pSink->QueryInterface(IID_IInternetProtocolSink,
      reinterpret_cast<void**>(&spSink));
    ATLASSERT(SUCCEEDED(hr) && spSink != 0);
  }
  if (SUCCEEDED(hr))
  {
    hr = pSink->QueryInterface(IID_IInternetBindInfo,
      reinterpret_cast<void**>(&spBindInfo));
    ATLASSERT(SUCCEEDED(hr) && spBindInfo != 0);
  }
  if (SUCCEEDED(hr))
  {
    hr = pTargetProtocol->Start(szUrl, spSink, spBindInfo, grfPI,
      dwReserved);
  }
  return hr;
}

// gears - added by jabdelmalek
template <class Protocol, class Sink>
inline HRESULT CustomSinkStartPolicy<Protocol, Sink>::OnStartEx(IUri *pUri,
    IInternetProtocolSink *pOIProtSink, IInternetBindInfo *pOIBindInfo,
    DWORD grfPI, HANDLE_PTR dwReserved,
    IInternetProtocolEx* pTargetProtocol) const
{
  ATLASSERT(pTargetProtocol);

  Sink* pSink = GetSink(static_cast<const Protocol*>(this));
  HRESULT hr = pSink->OnStartEx(pUri, pOIProtSink, pOIBindInfo,
                                grfPI, dwReserved, pTargetProtocol);

  CComPtr<IInternetProtocolSink> spSink;
  CComPtr<IInternetBindInfo> spBindInfo;
  if (SUCCEEDED(hr)) {
    hr = pSink->QueryInterface(IID_IInternetProtocolSink,
      reinterpret_cast<void**>(&spSink));
    ATLASSERT(SUCCEEDED(hr) && spSink != 0);
  }

  if (SUCCEEDED(hr)) {
    hr = pSink->QueryInterface(IID_IInternetBindInfo,
      reinterpret_cast<void**>(&spBindInfo));
    ATLASSERT(SUCCEEDED(hr) && spBindInfo != 0);
  }

  if (SUCCEEDED(hr)) {
    hr = pTargetProtocol->StartEx(pUri, spSink, spBindInfo, grfPI, dwReserved);
  }

  return hr;
}

template <class Protocol, class Sink>
inline Sink* CustomSinkStartPolicy<Protocol, Sink>::GetSink(
  const Protocol* pProtocol)
{
  return Protocol::ComObjectClass::GetSink(pProtocol);
}

template <class Protocol, class Sink>
inline Sink* CustomSinkStartPolicy<Protocol, Sink>::GetSink() const
{
  return GetSink(static_cast<Protocol*>(this));
}

template <class Protocol, class Sink>
inline Protocol* CustomSinkStartPolicy<Protocol, Sink>::GetProtocol(
  const Sink* pSink)
{
  return Protocol::ComObjectClass::GetProtocol(pSink);
}

} // end namespace PassthroughAPP

#endif // GEARS_THIRD_PARTY_PASSTHRU_APP_SINKPOLICY_INL__
