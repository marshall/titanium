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

#ifndef GEARS_THIRD_PARTY_PASSTHRU_APP_SINKPOLICY_H__
#define GEARS_THIRD_PARTY_PASSTHRU_APP_SINKPOLICY_H__

#if _MSC_VER > 1000
  #pragma once
#endif // _MSC_VER > 1000

namespace PassthroughAPP
{

// A sink policy class should implement OnStart with the prototype shown
// below, presumably by eventually forwarding to pTargetProtocol->Start,
// possibly with different parameters

class NoSinkStartPolicy
{
public:
  HRESULT OnStart(LPCWSTR szUrl, IInternetProtocolSink *pOIProtSink,
    IInternetBindInfo *pOIBindInfo, DWORD grfPI, HANDLE_PTR dwReserved,
    IInternetProtocol* pTargetProtocol) const;

  // gears - added by jabdelmalek
  HRESULT OnStartEx(IUri *pUri, IInternetProtocolSink *pOIProtSink,
    IInternetBindInfo *pOIBindInfo, DWORD grfPI, HANDLE_PTR dwReserved,
    IInternetProtocolEx* pTargetProtocol) const;
};

template <class Base>
class CComObjectSharedRef : public Base
{
public:
  typedef Base ContainedObject;

  CComObjectSharedRef(IUnknown* punkRefCount, IUnknown* = 0);
#ifdef _ATL_DEBUG_INTERFACES
  ~CComObjectSharedRef();
#endif

  STDMETHODIMP QueryInterface(REFIID iid, void** ppvObject);

  template <class Q>
  HRESULT STDMETHODCALLTYPE QueryInterface(Q** pp)
  {
    return QueryInterface(__uuidof(Q), (void**)pp);
  }

  STDMETHODIMP_(ULONG) AddRef();
  STDMETHODIMP_(ULONG) Release();

  Base* GetContainedObject();
  static CComObjectSharedRef* GetThisObject(const Base* pBase);
private:
  IUnknown* m_punkRefCount;
};

template <class Contained>
class CComPolyObjectSharedRef :
  public IUnknown,
  public CComObjectRootEx<typename Contained::_ThreadModel::ThreadModelNoCS>
{
public:
  typedef Contained ContainedObject;

  typedef typename Contained::_ThreadModel _ThreadModel;

  CComPolyObjectSharedRef(IUnknown* punkRefCount, IUnknown* punkOuter);
#ifdef _ATL_DEBUG_INTERFACES
  ~CComPolyObjectSharedRef();
#endif

  HRESULT FinalConstruct();
  void FinalRelease();

  STDMETHODIMP QueryInterface(REFIID iid, void** ppvObject);

  template <class Q>
  HRESULT STDMETHODCALLTYPE QueryInterface(Q** pp)
  {
    return QueryInterface(__uuidof(Q), (void**)pp);
  }

  STDMETHODIMP_(ULONG) AddRef();
  STDMETHODIMP_(ULONG) Release();

  IUnknown* GetControllingUnknown();

  Contained* GetContainedObject();
  static CComPolyObjectSharedRef* GetThisObject(const Contained* pContained);

  CComContainedObject<Contained> m_contained;
private:
  IUnknown* m_punkRefCount;
};

template <class T, class ThreadModel>
class CComObjectRefCount :
  public IUnknown,
  public CComObjectRootEx<typename ThreadModel::ThreadModelNoCS>
{
  STDMETHODIMP QueryInterface(REFIID iid, void** ppvObject);
  STDMETHODIMP_(ULONG) AddRef();
  STDMETHODIMP_(ULONG) Release();
};

template <class ProtocolObject, class SinkObject>
class CComObjectProtSink :
  public ProtocolObject
{
public:
  typedef typename ProtocolObject::ContainedObject Protocol;
  typedef typename SinkObject::ContainedObject Sink;

  CComObjectProtSink(void* pv);
  ~CComObjectProtSink();

  HRESULT FinalConstruct();
  void FinalRelease();

  static Protocol* GetProtocol(const Sink* pSink);
  static Sink* GetSink(const Protocol* pProtocol);

  SinkObject m_sink;

  typedef CComObjectRefCount<CComObjectProtSink,
    typename ProtocolObject::_ThreadModel> RefCountObject;
  RefCountObject m_refCount;
};

#define DECLARE_NOT_AGGREGATABLE_PROTSINK(Protocol, Sink) public: \
  typedef ::PassthroughAPP::CComObjectProtSink< \
    ::PassthroughAPP::CComObjectSharedRef<Protocol>, \
    ::PassthroughAPP::CComObjectSharedRef<Sink> > \
  ComObjectClass; \
  typedef CComCreator2<CComCreator<ComObjectClass>, \
    CComFailCreator<CLASS_E_NOAGGREGATION> > _CreatorClass;

#define DECLARE_ONLY_AGGREGATABLE_PROTSINK(Protocol, Sink) public: \
  typedef ::PassthroughAPP::CComObjectProtSink< \
    ::PassthroughAPP::CComPolyObjectSharedRef<Protocol>, \
    ::PassthroughAPP::CComObjectSharedRef<Sink> > \
  ComObjectClass; \
  typedef CComCreator2<CComFailCreator<E_FAIL>, \
    CComCreator<ComObjectClass> > _CreatorClass;

#define DECLARE_AGGREGATABLE_PROTSINK(Protocol, Sink) public: \
  typedef ::PassthroughAPP::CComObjectProtSink< \
    ::PassthroughAPP::CComPolyObjectSharedRef<Protocol>, \
    ::PassthroughAPP::CComObjectSharedRef<Sink> > \
  ComObjectClass; \
  typedef CComCreator<ComObjectClass> _CreatorClass;


template <class Protocol, class Sink>
class CustomSinkStartPolicy
{
public:
  DECLARE_AGGREGATABLE_PROTSINK(Protocol, Sink)

  HRESULT OnStart(LPCWSTR szUrl,
    IInternetProtocolSink *pOIProtSink, IInternetBindInfo *pOIBindInfo,
    DWORD grfPI, HANDLE_PTR dwReserved,
    IInternetProtocol* pTargetProtocol) const;

  // gears - added by jabdelmalek
  HRESULT OnStartEx(IUri *pUri,
    IInternetProtocolSink *pOIProtSink, IInternetBindInfo *pOIBindInfo,
    DWORD grfPI, HANDLE_PTR dwReserved,
    IInternetProtocolEx* pTargetProtocol) const;

  static Sink* GetSink(const Protocol* pProtocol);
  Sink* GetSink() const;
  static Protocol* GetProtocol(const Sink* pSink);
};

} // end namespace PassthroughAPP

#include "SinkPolicy.inl"

#endif  // GEARS_THIRD_PARTY_PASSTHRU_APP_SINKPOLICY_H__
