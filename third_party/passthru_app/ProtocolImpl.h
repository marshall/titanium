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

#ifndef GEARS_THIRD_PARTY_PASSTHRU_APP_PROTOCOLIMPL_H__
#define GEARS_THIRD_PARTY_PASSTHRU_APP_PROTOCOLIMPL_H__

#if _MSC_VER > 1000
  #pragma once
#endif // _MSC_VER > 1000

#include <urlmon.h>
#include "urlmon_ie7_extras.h"  // gears - added by michaeln
#pragma comment(lib, "urlmon.lib")

#include "PassthroughObject.h"

namespace PassthroughAPP
{

namespace Detail
{

struct PassthroughItfData
{
  DWORD_PTR offsetItf;
  DWORD_PTR offsetUnk;
  const IID* piidBase;
};

template <class itf, class impl, DWORD_PTR offsetUnk, const IID* piidBase>
struct PassthroughItfHelper
{
  static PassthroughItfData data;
};

template <class itf, class impl, DWORD_PTR offsetUnk, const IID* piidBase>
PassthroughItfData
  PassthroughItfHelper<itf, impl, offsetUnk, piidBase>::
      data = {offsetofclass(itf, impl), offsetUnk, piidBase};

#define COM_INTERFACE_ENTRY_PASSTHROUGH(itf, punk)\
  {&_ATL_IIDOF(itf),\
  (DWORD_PTR)&::PassthroughAPP::Detail::PassthroughItfHelper<\
    itf, _ComMapClass,\
    (DWORD_PTR)offsetof(_ComMapClass, punk),\
    0\
  >::data,\
  ::PassthroughAPP::Detail::QIPassthrough<_ComMapClass>::\
    QueryInterfacePassthroughT\
  },

#define COM_INTERFACE_ENTRY_PASSTHROUGH2(itf, punk, itfBase)\
  {&_ATL_IIDOF(itf),\
  (DWORD_PTR)&::PassthroughAPP::Detail::PassthroughItfHelper<\
    itf, _ComMapClass,\
    (DWORD_PTR)offsetof(_ComMapClass, punk),\
    &_ATL_IIDOF(itfBase)\
  >::data,\
  ::PassthroughAPP::Detail::QIPassthrough<_ComMapClass>::\
    QueryInterfacePassthroughT\
  },


#ifdef DEBUG

  #define COM_INTERFACE_ENTRY_PASSTHROUGH_DEBUG()\
    {0, 0,\
    ::PassthroughAPP::Detail::QIPassthrough<_ComMapClass>::\
      QueryInterfaceDebugT\
    },

#else
  #define COM_INTERFACE_ENTRY_PASSTHROUGH_DEBUG()
#endif

#define DECLARE_GET_TARGET_UNKNOWN(x) \
  inline IUnknown* GetTargetUnknown() {return x;}

// Workaround for VC6's deficiencies in dealing with function templates.
// We'd use non-member template functions, but VC6 does not handle those well.
// Static members of class templates work much better, and we don't need
// parameter deduction here
template <class T>
struct QIPassthrough
{
  static HRESULT WINAPI QueryInterfacePassthroughT(void* pv, REFIID riid,
    LPVOID* ppv, DWORD_PTR dw);
  static HRESULT WINAPI QueryInterfaceDebugT(void* pv, REFIID riid,
    LPVOID* ppv, DWORD_PTR dw);
};

HRESULT WINAPI QueryInterfacePassthrough(void* pv, REFIID riid,
  LPVOID* ppv, DWORD_PTR dw, IUnknown* punkTarget, IUnknown* punkWrapper);

HRESULT WINAPI QueryInterfaceDebug(void* pv, REFIID riid,
  LPVOID* ppv, DWORD_PTR dw, IUnknown* punkTarget);

HRESULT QueryServicePassthrough(REFGUID guidService,
  IUnknown* punkThis, REFIID riid, void** ppv,
  IServiceProvider* pClientProvider);

} // end namespace PassthroughAPP::Detail

class ATL_NO_VTABLE IInternetProtocolImpl :
  public IPassthroughObject,
  // public IInternetProtocol,  // gears -removed by jabdelmalek
  public IInternetProtocolEx,  // gears - added by jabdelmalek
  public IInternetProtocolInfo,
  public IInternetPriority,
  public IInternetThreadSwitch,
  public IWinInetHttpInfo,
  public IWinInetCacheHints2  // gears - added by michaeln
{
public:
  void ReleaseAll();

  DECLARE_GET_TARGET_UNKNOWN(m_spInternetProtocolUnk)
public:
  // IPassthroughObject
  STDMETHODIMP SetTargetUnknown(IUnknown* punkTarget);

  // gears - added by jabdelmalek
  // IInternetProtocolEx
  STDMETHODIMP StartEx(IUri *pUri,
    IInternetProtocolSink *pOIProtSink,
    IInternetBindInfo *pOIBindInfo,
    DWORD grfPI,
    HANDLE_PTR dwReserved);

  // IInternetProtocolRoot
  STDMETHODIMP Start(
    /* [in] */ LPCWSTR szUrl,
    /* [in] */ IInternetProtocolSink *pOIProtSink,
    /* [in] */ IInternetBindInfo *pOIBindInfo,
    /* [in] */ DWORD grfPI,
    /* [in] */ HANDLE_PTR dwReserved);

  STDMETHODIMP Continue(
    /* [in] */ PROTOCOLDATA *pProtocolData);

  STDMETHODIMP Abort(
    /* [in] */ HRESULT hrReason,
    /* [in] */ DWORD dwOptions);

  STDMETHODIMP Terminate(
    /* [in] */ DWORD dwOptions);

  STDMETHODIMP Suspend();

  STDMETHODIMP Resume();

  // IInternetProtocol
  STDMETHODIMP Read(
    /* [in, out] */ void *pv,
    /* [in] */ ULONG cb,
    /* [out] */ ULONG *pcbRead);

  STDMETHODIMP Seek(
    /* [in] */ LARGE_INTEGER dlibMove,
    /* [in] */ DWORD dwOrigin,
    /* [out] */ ULARGE_INTEGER *plibNewPosition);

  STDMETHODIMP LockRequest(
    /* [in] */ DWORD dwOptions);

  STDMETHODIMP UnlockRequest();

  // IInternetProtocolInfo
  STDMETHODIMP ParseUrl(
    /* [in] */ LPCWSTR pwzUrl,
    /* [in] */ PARSEACTION ParseAction,
    /* [in] */ DWORD dwParseFlags,
    /* [out] */ LPWSTR pwzResult,
    /* [in] */ DWORD cchResult,
    /* [out] */ DWORD *pcchResult,
    /* [in] */ DWORD dwReserved);

  STDMETHODIMP CombineUrl(
    /* [in] */ LPCWSTR pwzBaseUrl,
    /* [in] */ LPCWSTR pwzRelativeUrl,
    /* [in] */ DWORD dwCombineFlags,
    /* [out] */ LPWSTR pwzResult,
    /* [in] */ DWORD cchResult,
    /* [out] */ DWORD *pcchResult,
    /* [in] */ DWORD dwReserved);

  STDMETHODIMP CompareUrl(
    /* [in] */ LPCWSTR pwzUrl1,
    /* [in] */ LPCWSTR pwzUrl2,
    /* [in] */ DWORD dwCompareFlags);

  STDMETHODIMP QueryInfo(
    /* [in] */ LPCWSTR pwzUrl,
    /* [in] */ QUERYOPTION QueryOption,
    /* [in] */ DWORD dwQueryFlags,
    /* [in, out] */ LPVOID pBuffer,
    /* [in] */ DWORD cbBuffer,
    /* [in, out] */ DWORD *pcbBuf,
    /* [in] */ DWORD dwReserved);

  // IInternetPriority
  STDMETHODIMP SetPriority(
    /* [in] */ LONG nPriority);

  STDMETHODIMP GetPriority(
    /* [out] */ LONG *pnPriority);

  // IInternetThreadSwitch
  STDMETHODIMP Prepare();

  STDMETHODIMP Continue();

  // IWinInetInfo
  STDMETHODIMP QueryOption(
    /* [in] */ DWORD dwOption,
    /* [in, out] */ LPVOID pBuffer,
    /* [in, out] */ DWORD *pcbBuf);

  // IWinInetHttpInfo
  STDMETHODIMP QueryInfo(
    /* [in] */ DWORD dwOption,
    /* [in, out] */ LPVOID pBuffer,
    /* [in, out] */ DWORD *pcbBuf,
    /* [in, out] */ DWORD *pdwFlags,
    /* [in, out] */ DWORD *pdwReserved);

    // IWinInetCacheHints
    // gears - added by michaeln
    virtual HRESULT STDMETHODCALLTYPE SetCacheExtension(
        /* [in] */ LPCWSTR pwzExt,
        /* [size_is][out][in] */ LPVOID pszCacheFile,
        /* [out][in] */ DWORD *pcbCacheFile,
        /* [out][in] */ DWORD *pdwWinInetError,
        /* [out][in] */ DWORD *pdwReserved);

    // IWinInetCacheHints2
    // gears - added by michaeln
    virtual HRESULT STDMETHODCALLTYPE SetCacheExtension2(
        /* [in] */ LPCWSTR pwzExt,
        /* [size_is][out] */ WCHAR *pwzCacheFile,
        /* [out][in] */ DWORD *pcchCacheFile,
        /* [out] */ DWORD *pdwWinInetError,
        /* [out] */ DWORD *pdwReserved);

public:
  CComPtr<IUnknown> m_spInternetProtocolUnk;
  CComPtr<IInternetProtocol> m_spInternetProtocol;
  CComPtr<IInternetProtocolEx> m_spInternetProtocolEx;  // gears - added by jabdelmalek
  CComPtr<IInternetProtocolInfo> m_spInternetProtocolInfo;
  CComPtr<IInternetPriority> m_spInternetPriority;
  CComPtr<IInternetThreadSwitch> m_spInternetThreadSwitch;
  CComPtr<IWinInetInfo> m_spWinInetInfo;
  CComPtr<IWinInetHttpInfo> m_spWinInetHttpInfo;
  CComPtr<IWinInetCacheHints> m_spWinInetCacheHints;  // gears - added by michaeln
  CComPtr<IWinInetCacheHints2> m_spWinInetCacheHints2;  // gears - added by michaeln
};

class ATL_NO_VTABLE IInternetProtocolSinkImpl :
  public IInternetProtocolSink,
  public IServiceProvider,
  public IInternetBindInfo
{
public:
  HRESULT OnStart(LPCWSTR szUrl, IInternetProtocolSink *pOIProtSink,
    IInternetBindInfo *pOIBindInfo, DWORD grfPI, HANDLE_PTR dwReserved,
    IInternetProtocol* pTargetProtocol);

  // gears - added by jabdelmalek
  HRESULT OnStartEx(IUri *pUri, IInternetProtocolSink *pOIProtSink,
    IInternetBindInfo *pOIBindInfo, DWORD grfPI, HANDLE_PTR dwReserved,
    IInternetProtocolEx* pTargetProtocol);

  void ReleaseAll();

  DECLARE_GET_TARGET_UNKNOWN(m_spInternetProtocolSink)

  IServiceProvider* GetClientServiceProvider();

  HRESULT QueryServiceFromClient(REFGUID guidService, REFIID riid,
    void** ppvObject);
  template <class Q>
  HRESULT QueryServiceFromClient(REFGUID guidService, Q** pp)
  {
    return QueryServiceFromClient(guidService, _ATL_IIDOF(Q),
      reinterpret_cast<void**>(pp));
  }
  template <class Q>
  HRESULT QueryServiceFromClient(Q** pp)
  {
    return QueryServiceFromClient(_ATL_IIDOF(Q), _ATL_IIDOF(Q),
      reinterpret_cast<void**>(pp));
  }
public:
  // IInternetProtocolSink
  STDMETHODIMP Switch(
    /* [in] */ PROTOCOLDATA *pProtocolData);

  STDMETHODIMP ReportProgress(
    /* [in] */ ULONG ulStatusCode,
    /* [in] */ LPCWSTR szStatusText);

  STDMETHODIMP ReportData(
    /* [in] */ DWORD grfBSCF,
    /* [in] */ ULONG ulProgress,
    /* [in] */ ULONG ulProgressMax);

  STDMETHODIMP ReportResult(
    /* [in] */ HRESULT hrResult,
    /* [in] */ DWORD dwError,
    /* [in] */ LPCWSTR szResult);

  // IServiceProvider
  STDMETHODIMP QueryService(
    /* [in] */ REFGUID guidService,
    /* [in] */ REFIID riid,
    /* [out] */ void** ppvObject);

  // IInternetBindInfo
  STDMETHODIMP GetBindInfo(
    /* [out] */ DWORD *grfBINDF,
    /* [in, out] */ BINDINFO *pbindinfo);

  STDMETHODIMP GetBindString(
    /* [in] */ ULONG ulStringType,
    /* [in, out] */ LPOLESTR *ppwzStr,
    /* [in] */ ULONG cEl,
    /* [in, out] */ ULONG *pcElFetched);
public:
  CComPtr<IInternetProtocolSink> m_spInternetProtocolSink;
  CComPtr<IServiceProvider> m_spServiceProvider;
  CComPtr<IInternetBindInfo> m_spInternetBindInfo;

  CComPtr<IInternetProtocol> m_spTargetProtocol;
};

template <class ThreadModel = CComSingleThreadModel>
class CInternetProtocolSinkTM :
  public CComObjectRootEx<ThreadModel>,
  public IInternetProtocolSinkImpl
{
public:
  BEGIN_COM_MAP(CInternetProtocolSinkTM)
    COM_INTERFACE_ENTRY(IInternetProtocolSink)
    COM_INTERFACE_ENTRY_PASSTHROUGH(IServiceProvider,
      m_spServiceProvider.p)
    COM_INTERFACE_ENTRY(IInternetBindInfo)
    COM_INTERFACE_ENTRY_PASSTHROUGH_DEBUG()
  END_COM_MAP()
};

typedef CInternetProtocolSinkTM<> CInternetProtocolSink;

template <class T, class ThreadModel = CComSingleThreadModel>
class CInternetProtocolSinkWithSP :
  public CInternetProtocolSinkTM<ThreadModel>
{
  typedef CInternetProtocolSinkTM<ThreadModel> BaseClass;
public:
  HRESULT OnStart(LPCWSTR szUrl, IInternetProtocolSink *pOIProtSink,
    IInternetBindInfo *pOIBindInfo, DWORD grfPI, HANDLE_PTR dwReserved,
    IInternetProtocol* pTargetProtocol);

  // gears - added by jabdelmalek
  HRESULT OnStartEx(IUri *pUri, IInternetProtocolSink *pOIProtSink,
    IInternetBindInfo *pOIBindInfo, DWORD grfPI, HANDLE_PTR dwReserved,
    IInternetProtocolEx* pTargetProtocol);

  STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void** ppv);

  HRESULT _InternalQueryService(REFGUID guidService, REFIID riid,
    void** ppvObject);

  BEGIN_COM_MAP(CInternetProtocolSinkWithSP)
    COM_INTERFACE_ENTRY(IServiceProvider)
    COM_INTERFACE_ENTRY_CHAIN(BaseClass)
  END_COM_MAP()
};

#define SERVICE_ENTRY_PASSTHROUGH(x) \
  if (InlineIsEqualGUID(guidService, x)) \
  { \
    return ::PassthroughAPP::Detail::QueryServicePassthrough(guidService, \
      GetUnknown(), riid, ppvObject, GetClientServiceProvider()); \
  }

template <class StartPolicy, class ThreadModel = CComSingleThreadModel>
class ATL_NO_VTABLE CInternetProtocol :
  public CComObjectRootEx<ThreadModel>,
  public IInternetProtocolImpl,
  public StartPolicy
{
public:
  BEGIN_COM_MAP(CInternetProtocol)
    COM_INTERFACE_ENTRY(IPassthroughObject)
    COM_INTERFACE_ENTRY(IInternetProtocolRoot)
    COM_INTERFACE_ENTRY(IInternetProtocol)
    // gears - added by michaeln
    COM_INTERFACE_ENTRY_PASSTHROUGH2(IInternetProtocolEx,
      m_spInternetProtocolEx.p, IInternetProtocol)
    COM_INTERFACE_ENTRY_PASSTHROUGH(IInternetProtocolInfo,
      m_spInternetProtocolInfo.p)
    COM_INTERFACE_ENTRY_PASSTHROUGH(IInternetPriority,
      m_spInternetPriority.p)
    COM_INTERFACE_ENTRY_PASSTHROUGH(IInternetThreadSwitch,
      m_spInternetThreadSwitch.p)
    COM_INTERFACE_ENTRY_PASSTHROUGH(IWinInetInfo, m_spWinInetInfo.p)
    COM_INTERFACE_ENTRY_PASSTHROUGH2(IWinInetHttpInfo,
      m_spWinInetHttpInfo.p, IWinInetInfo)
    // gears - added by michaeln
    COM_INTERFACE_ENTRY_PASSTHROUGH(IWinInetCacheHints, m_spWinInetCacheHints.p)
    // gears - added by michaeln
    COM_INTERFACE_ENTRY_PASSTHROUGH2(IWinInetCacheHints2,
      m_spWinInetCacheHints2.p, IWinInetCacheHints)
    COM_INTERFACE_ENTRY_PASSTHROUGH_DEBUG()
  END_COM_MAP()

  // IInternetProtocolRoot
  STDMETHODIMP Start(LPCWSTR szUrl, IInternetProtocolSink *pOIProtSink,
    IInternetBindInfo *pOIBindInfo, DWORD grfPI, HANDLE_PTR dwReserved);

  // gears - added by jabdelmalek
  // IInternetProtocolEx
  STDMETHODIMP StartEx(IUri *pUri, IInternetProtocolSink *pOIProtSink,
    IInternetBindInfo *pOIBindInfo, DWORD grfPI, HANDLE_PTR dwReserved);
};

} // end namespace PassthroughAPP

#include "ProtocolImpl.inl"

#include "SinkPolicy.h"

#endif  // GEARS_THIRD_PARTY_PASSTHRU_APP_PROTOCOLIMPL_H__
