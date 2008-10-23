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


#include <windows.h>
#include <wininet.h>
#include <algorithm>
#include <set>
#include <string>
#include <vector>

#include "gears/localserver/ie/http_handler_ie.h"
#include "gears/base/common/mutex.h"
#include "gears/base/common/string_utils.h"
#ifdef WINCE
#include "gears/base/common/wince_compatibility.h"  // For BrowserCache
#endif
#include "gears/base/ie/activex_utils.h"
#include "gears/base/ie/ie_version.h"
#include "gears/localserver/common/http_constants.h"
#include "gears/localserver/ie/urlmon_utils.h"

#ifdef WINCE
// On WinCE, when a request is made for a network resource, the device first
// checks the cache. If the resource is not present in the cache, the device
// attempts to make a network connection. If this fails, a pop-up a dialog box
// is shown to inform the user. This all happens before our HTTP handler
// receives the request callback, so we're not able to disable the popup for
// resources which are in the LocalServer.
//
// A workaround for this problem is to add an empty entry to the browser cache
// for each resource in the LocalServer. This occurs at the following points ...
// - For a resource store, when capturing a new resource.
// - For a resource store, when copying or moving a resource.
// - For a managed resource store, when reading a new manifest file.
//
// Note that we can not guarantee that the cache entries will persists (cache
// entries may be removed by the system when the cache becomes full or manually
// by the user), so we re-insert the entries at the following points ...
// - For a resource store, when refreshing a resource, but it's unmodified.
// - For a managed resource store, when checking for a manifest update.
// - When serving a resource from the local server.
//
// Also, the LocalServer can contain multiple resources (from seperate stores)
// for a given url which are conditionally eligible for local serving based on
// the enable property of the containing store and the presence or absence of
// particular cookies. This obviously doesn't map very well to the browser
// cache, so our approach isn't completely foolproof.
//
// Note that SetSessionOption, which is used on Win32 with
// kBypassIInternetProtocolInfoOption to prevent the device from attempting to
// connect to the network for LocalServer resources, is not available on WinCE.
#else
// NOTE: Undocumented voodoo to kick IE into using IInternetProtocolInfo
// for well known protocols.  We depend on this to indicate that items
// in our webcapture database do not require the network to be accessed.
static const DWORD kBypassIInternetProtocolInfoOption = 0x40;
#endif

static Mutex global_mutex;
static bool has_registered_handler = false;
static CComPtr<IClassFactory> factory_http;
static CComPtr<IClassFactory> factory_https;
static CComQIPtr<IInternetProtocolInfo> factory_protocol_info;
static bool bypass_cache = false;

// {B320A625-02E1-4cc8-A864-04E825297A20}
const GUID HttpHandler::SID_QueryBypassCache =
{ 0xb320a625, 0x2e1, 0x4cc8,
  { 0xa8, 0x64, 0x4, 0xe8, 0x25, 0x29, 0x7a, 0x20 } };

// {2C3ED4D8-F7A1-48b4-A979-5B197C15D0FC}
const GUID HttpHandler::IID_QueryBypassCache =
{ 0x2c3ed4d8, 0xf7a1, 0x48b4,
  { 0xa9, 0x79, 0x5b, 0x19, 0x7c, 0x15, 0xd0, 0xfc } };


static HRESULT RegisterNoLock();
static HRESULT UnregisterNoLock();


// static
HRESULT HttpHandler::Register() {
  if (has_registered_handler) return S_OK;
  MutexLock lock(&global_mutex);
  return RegisterNoLock();
}

// static
HRESULT HttpHandler::Unregister() {
  MutexLock lock(&global_mutex);
  return UnregisterNoLock();
}


// Registers our HttpHandler with URLMON to intercept HTTP and HTTPS requests,
// and configures URLMON to call our implementation of IInternetProtocolInfo
// rather than using the default implementation.
// TODO(michaeln): should this only be done if the process is iexplorer.exe?
static HRESULT RegisterNoLock() {
  typedef PassthroughAPP::CMetaFactory<HttpHandlerFactory, HttpHandler>
      PassthruMetaFactory;

  if (has_registered_handler) {
    return S_OK;
  }
  has_registered_handler = true;

  CComPtr<IInternetSession> session;
  HRESULT hr = CoInternetGetSession(0, &session, 0);
  if (FAILED(hr) || !session) {
    UnregisterNoLock();
    return FAILED(hr) ? hr : E_FAIL;
  }
  hr = PassthruMetaFactory::CreateInstance(CLSID_HttpProtocol, &factory_http);
  if (FAILED(hr) || !factory_http) {
    UnregisterNoLock();
    return FAILED(hr) ? hr : E_FAIL;
  }
  hr = PassthruMetaFactory::CreateInstance(CLSID_HttpSProtocol, &factory_https);
  if (FAILED(hr) || !factory_https) {
    UnregisterNoLock();
    return FAILED(hr) ? hr : E_FAIL;
  }
  factory_protocol_info = factory_http;
  if (!factory_protocol_info) {
    UnregisterNoLock();
    return E_FAIL;
  }
  hr = session->RegisterNameSpace(factory_http, CLSID_NULL,
                                  HttpConstants::kHttpScheme, 0, 0, 0);
  if (FAILED(hr)) {
    UnregisterNoLock();
    return hr;
  }
  hr = session->RegisterNameSpace(factory_https, CLSID_NULL,
                                  HttpConstants::kHttpsScheme, 0, 0, 0);
  if (FAILED(hr)) {
    UnregisterNoLock();
    return hr;
  }
#ifdef WINCE
  // SetSessionOption is not implemented on WinCE.
#else
  BOOL bypass = FALSE;
  hr = session->SetSessionOption(kBypassIInternetProtocolInfoOption,
                                 &bypass, sizeof(BOOL), 0);
  if (FAILED(hr)) {
    UnregisterNoLock();
    return hr;
  }
#endif
  LOG16((L"HttpHandler::Registered\n"));
  return hr;
}


static HRESULT UnregisterNoLock() {
  if (!has_registered_handler) {
    return S_OK;
  }
  has_registered_handler = false;

  CComPtr<IInternetSession> session;
  HRESULT rv = CoInternetGetSession(0, &session, 0);
  if (FAILED(rv) || !session) {
    return FAILED(rv) ? rv : E_FAIL;
  }
  if (factory_http != NULL) {
    session->UnregisterNameSpace(factory_http, HttpConstants::kHttpScheme);
    factory_http.Release();
  }
  if (factory_https != NULL) {
    session->UnregisterNameSpace(factory_https, HttpConstants::kHttpScheme);
    factory_https.Release();
  }
  factory_protocol_info.Release();
#ifdef WINCE
  // SetSessionOption is not implemented on WinCE.
#else
  BOOL bypass = TRUE;
  session->SetSessionOption(kBypassIInternetProtocolInfoOption,
                            &bypass, sizeof(BOOL), 0);
#endif
  LOG16((L"HttpHandler::Unregistered\n"));
  return S_OK;
}


// static
void HttpHandler::SetBypassCache() {
  bypass_cache = true;
}

#ifdef DEBUG
// To support debugging, this class can be configured via the registry to
// display an alert when an uncaptured resource is requested while the
// browser is working in offline mode.
// See StartImpl()
static bool InitAlertCacheMiss() {
  bool default_value = false;
  CRegKey key;
  LONG res = key.Open(HKEY_CURRENT_USER,
                      L"Software\\" PRODUCT_FRIENDLY_NAME,
                      KEY_READ);
  if (res != ERROR_SUCCESS)
    return default_value;
  DWORD val;
  res = key.QueryDWORDValue(_T("AlertCacheMiss"), val);
  if (res != ERROR_SUCCESS)
    return default_value;
  return val != 0;
}
static const bool kAlertCacheMiss = InitAlertCacheMiss();
#else
static const bool kAlertCacheMiss = false;
#endif


//------------------------------------------------------------------------------
// class PassthruSink
//------------------------------------------------------------------------------

STDMETHODIMP PassthruSink::ReportProgress(
    /* [in] */ ULONG status_code,
    /* [in] */ LPCWSTR status_text) {
  if (!http_handler_)
    return E_UNEXPECTED;
  if (!http_handler_->is_passingthru_)
    return E_FAIL;

#ifdef DEBUG
  LOG16((L"PassthruSink::ReportProgress( %s, %s )\n",
         GetBindStatusLabel(status_code),
         status_text ? status_text : L""));
#endif

  if (status_code == BINDSTATUS_REDIRECTING) {
    // status_text contains the redirect url in this case
    WebCacheDB* db = WebCacheDB::GetDB();
    if (db && db->CanService(status_text, NULL)) {
      // Here we detect 302s into our cache here and intervene to hijack
      // handling of the request. Reporting a result of INET_E_REDIRECT_FAILED
      // causes URMLON to abandon this handler instance an create a new one
      // to follow the redirect. When that new instance of our handler is
      // created, it will satisfy the request locally.
      LOG16((L"PassthruSink::ReportProgress - hijacking redirect\n"));
      return BaseClass::ReportResult(INET_E_REDIRECT_FAILED,
                                     HttpConstants::HTTP_FOUND,
                                     status_text);
    }
  }
  return BaseClass::ReportProgress(status_code, status_text);
}


//------------------------------------------------------------------------------
// ActiveHandlers
//
// This class is here to workaround a crash in IE6SP2 when the browser process
// is exiting. In some circumstances, HttpHandlers are not terminated as they
// should be. Some time after our DLL is unloaded during shutdown, IE
// invokes methods on these orphaned handlers resulting in a crash. Note that
// WinCE does not suffer from this problem.
// See http://code.google.com/p/google-gears/issues/detail?id=182
//
// To avoid this problem, we maintain a collection of the active HttpHandlers
// when running in IE6 or earlier. When our DLL is unloaded, we explicitly
// Terminate() any orphaned handlers. This prevents the crash.
// TODO(michaeln): If and when we find and fix the source of this bug,
// remove this workaround code.
//------------------------------------------------------------------------------

#ifdef WINCE
class ActiveHandlers {
 public:
  void Add(HttpHandler *handler) {}
  void Remove(HttpHandler *handler) {}
};
#else
class ActiveHandlers : public std::set<HttpHandler*> {
 public:
  ActiveHandlers()
      : has_determined_ie_version_(false), is_at_least_version_7_(false) {}

  virtual ~ActiveHandlers() {
    while (HttpHandler* handler = GetAndRemoveFirstHandler()) {
      handler->Terminate(0);
      handler->Release();
    }
  }

  void Add(HttpHandler *handler) {
    assert(handler);
    if (!IsIEAtLeastVersion7()) {
      MutexLock lock(&mutex_);
      insert(handler);
    }
  }

  void Remove(HttpHandler *handler) {
    assert(handler);
    if (!IsIEAtLeastVersion7()) {
      MutexLock lock(&mutex_);
      erase(handler);
    }
  }

 private:
  HttpHandler *GetAndRemoveFirstHandler() {
    MutexLock lock(&mutex_);
    if (empty()) return NULL;
    HttpHandler *handler = *begin();
    erase(handler);
    handler->AddRef();  // released by our caller
    return handler;
  }

  bool IsIEAtLeastVersion7() {
    if (!has_determined_ie_version_) {
      MutexLock lock(&mutex_);
      is_at_least_version_7_ = IsIEAtLeastVersion(7, 0, 0, 0);
      has_determined_ie_version_ = true;
    }
    return is_at_least_version_7_;
  }

  Mutex mutex_;
  bool has_determined_ie_version_;
  bool is_at_least_version_7_;
};
#endif

static ActiveHandlers g_active_handlers;

//------------------------------------------------------------------------------
// class HttpHandler
//------------------------------------------------------------------------------

HttpHandler::HttpHandler()
    : is_passingthru_(false), is_handling_(false),
      is_head_request_(false), has_reported_result_(false),
      was_aborted_(false), was_terminated_(false),
      read_pointer_(0), passthru_sink_(NULL) {
}

HttpHandler::~HttpHandler() {
  LOG16((L"~HttpHandler\n"));
  g_active_handlers.Remove(this);   // okay to Remove() multiple times from set
}

HRESULT HttpHandler::FinalConstruct() {
  passthru_sink_ = PassthruStartPolicy::GetSink(this);
  passthru_sink_->SetHttpHandler(this);
  return S_OK;
}

// IInternetProtocolEx
STDMETHODIMP HttpHandler::StartEx(
    /* [in] */ IUri *uri,
    /* [in] */ IInternetProtocolSink *protocol_sink,
    /* [in] */ IInternetBindInfo *bind_info,
    /* [in] */ DWORD flags,
    /* [in] */ HANDLE_PTR reserved) {
  CComBSTR uri_bstr;
  HRESULT rv = uri->GetAbsoluteUri(&uri_bstr);
  if (FAILED(rv)) {
    return rv;
  }
  LOG16((L"HttpHandler::StartEx( %s )\n", uri_bstr.m_str));
  rv = StartImpl(uri_bstr.m_str, protocol_sink, bind_info, flags, reserved);
  if (rv == INET_E_USE_DEFAULT_PROTOCOLHANDLER) {
    protocol_sink_.Release();
    http_negotiate_.Release();
    if (is_passingthru_) {
      return BaseClass::StartEx(uri, protocol_sink, bind_info, flags, reserved);
    } else {
      return rv;
    }
  } else {
    return rv;
  }
}

// IInternetProtocolRoot
STDMETHODIMP HttpHandler::Start(
    /* [in] */ LPCWSTR url,
    /* [in] */ IInternetProtocolSink *protocol_sink,
    /* [in] */ IInternetBindInfo *bind_info,
    /* [in] */ DWORD flags,
    /* [in] */ HANDLE_PTR reserved) {
  LOG16((L"HttpHandler::Start( %s )\n", url));
  HRESULT rv = StartImpl(url, protocol_sink, bind_info, flags, reserved);
  if (rv == INET_E_USE_DEFAULT_PROTOCOLHANDLER) {
    protocol_sink_.Release();
    http_negotiate_.Release();
    if (is_passingthru_) {
      return BaseClass::Start(url, protocol_sink, bind_info, flags, reserved);
    } else {
      return rv;
    }
  } else {
    return rv;
  }
}

STDMETHODIMP HttpHandler::Continue(
    /* [in] */ PROTOCOLDATA *data) {
  LOG16((L"HttpHandler::Continue(data)\n"));
  if (is_passingthru_)
    return BaseClass::Continue(data);
  else if (is_handling_)
    return S_OK;
  else
    return E_UNEXPECTED;
}

STDMETHODIMP HttpHandler::Abort(
    /* [in] */ HRESULT reason,
    /* [in] */ DWORD options) {
  LOG16((L"HttpHandler::Abort()\n"));
  if (is_passingthru_) {
    return BaseClass::Abort(reason, options);
  } else if (is_handling_) {
    was_aborted_ = true;
    // We intentionally don't propogate the return value from this method,
    // our handler is aborted regardless of the sink's return value.
    CallReportResult(reason, E_ABORT, L"Aborted");
    return S_OK;
  } else {
    return E_UNEXPECTED;
  }
}

STDMETHODIMP HttpHandler::Terminate(
    /* [in] */ DWORD options) {
  LOG16((L"HttpHandler::Terminate()\n"));
  protocol_sink_.Release();
  http_negotiate_.Release();
  g_active_handlers.Remove(this);
  if (is_passingthru_) {
    return BaseClass::Terminate(options);
  } else if (is_handling_) {
    was_terminated_ = true;
    return S_OK;
  } else {
    return E_UNEXPECTED;
  }
}

STDMETHODIMP HttpHandler::Suspend() {
  LOG16((L"HttpHandler::Suspend()\n"));
  if (is_passingthru_)
    return BaseClass::Suspend();
  else if (is_handling_)
    return S_OK;
  else
    return E_UNEXPECTED;
}

STDMETHODIMP HttpHandler::Resume() {
  LOG16((L"HttpHandler::Resume()\n"));
  if (is_passingthru_)
    return BaseClass::Resume();
  else if (is_handling_)
    return S_OK;
  else
    return E_UNEXPECTED;
}

// IInternetProtocol
STDMETHODIMP HttpHandler::Read(
    /* [in, out] */ void *pv,
    /* [in] */ ULONG cb,
    /* [out] */ ULONG *pcbRead) {
  if (is_passingthru_) {
    HRESULT hr = BaseClass::Read(pv, cb, pcbRead);
    LOG16((L"HttpHandler::Read() - passing thru, %d bytes\n", *pcbRead));
    return hr;
  } else if (is_handling_) {
    return ReadImpl(pv, cb, pcbRead);
  } else {
    LOG16((L"HttpHandler::Read() - unexpected\n"));
    return E_UNEXPECTED;
  }
}

STDMETHODIMP HttpHandler::Seek(
    /* [in] */ LARGE_INTEGER dlibMove,
    /* [in] */ DWORD dwOrigin,
    /* [out] */ ULARGE_INTEGER *plibNewPosition) {
  LOG16((L"HttpHandler::Seek()\n"));
  if (is_passingthru_)
    return BaseClass::Seek(dlibMove, dwOrigin, plibNewPosition);
  else if (is_handling_)
    return S_OK;
  else
    return E_UNEXPECTED;
}

STDMETHODIMP HttpHandler::LockRequest(
    /* [in] */ DWORD dwOptions) {
  LOG16((L"HttpHandler::LockRequest()\n"));
  if (is_passingthru_)
    return BaseClass::LockRequest(dwOptions);
  else if (is_handling_)
    return S_OK;
  else
    return E_UNEXPECTED;
}

STDMETHODIMP HttpHandler::UnlockRequest() {
  LOG16((L"HttpHandler::UnlockRequest()\n"));
  if (is_passingthru_)
    return BaseClass::UnlockRequest();
  else if (is_handling_)
    return S_OK;
  else
    return E_UNEXPECTED;
}

// IInternetProtocolInfo
// This interface can be used out of the context of handling a request, this
// is why we don't return E_UNEXPECTED in that case
STDMETHODIMP HttpHandler::ParseUrl(
    /* [in] */ LPCWSTR pwzUrl,
    /* [in] */ PARSEACTION ParseAction,
    /* [in] */ DWORD dwParseFlags,
    /* [out] */ LPWSTR pwzResult,
    /* [in] */ DWORD cchResult,
    /* [out] */ DWORD *pcchResult,
    /* [in] */ DWORD dwReserved) {
  LOG16((L"HttpHandler::ParseUrl()\n"));
  if (is_passingthru_)
    return BaseClass::ParseUrl(pwzUrl, ParseAction, dwParseFlags,
                               pwzResult, cchResult, pcchResult,
                               dwReserved);
  else
    return factory_protocol_info->ParseUrl(pwzUrl, ParseAction, dwParseFlags,
                                           pwzResult, cchResult, pcchResult,
                                           dwReserved);
}

STDMETHODIMP HttpHandler::CombineUrl(
    /* [in] */ LPCWSTR pwzBaseUrl,
    /* [in] */ LPCWSTR pwzRelativeUrl,
    /* [in] */ DWORD dwCombineFlags,
    /* [out] */ LPWSTR pwzResult,
    /* [in] */ DWORD cchResult,
    /* [out] */ DWORD *pcchResult,
    /* [in] */ DWORD dwReserved) {
  LOG16((L"HttpHandler::CombineUrl()\n"));
  if (is_passingthru_)
    return BaseClass::CombineUrl(pwzBaseUrl, pwzRelativeUrl,
                                 dwCombineFlags, pwzResult,
                                 cchResult, pcchResult, dwReserved);
  else
    return factory_protocol_info->CombineUrl(pwzBaseUrl, pwzRelativeUrl,
                                             dwCombineFlags, pwzResult,
                                             cchResult, pcchResult, dwReserved);
}

STDMETHODIMP HttpHandler::CompareUrl(
    /* [in] */ LPCWSTR pwzUrl1,
    /* [in] */ LPCWSTR pwzUrl2,
    /* [in] */ DWORD dwCompareFlags) {
  LOG16((L"HttpHandler::CompareUrl()\n"));
  if (is_passingthru_)
    return BaseClass::CompareUrl(pwzUrl1, pwzUrl2, dwCompareFlags);
  else
    return factory_protocol_info->CompareUrl(pwzUrl1, pwzUrl2, dwCompareFlags);
}

STDMETHODIMP HttpHandler::QueryInfo(
    /* [in] */ LPCWSTR pwzUrl,
    /* [in] */ QUERYOPTION QueryOption,
    /* [in] */ DWORD dwQueryFlags,
    /* [in, out] */ LPVOID pBuffer,
    /* [in] */ DWORD cbBuffer,
    /* [in, out] */ DWORD *pcbBuf,
    /* [in] */ DWORD dwReserved) {
  LOG16((L"HttpHandler::QueryInfo()\n"));
  if (is_passingthru_)
    return BaseClass::QueryInfo(pwzUrl, QueryOption, dwQueryFlags,
                                pBuffer, cbBuffer, pcbBuf,
                                dwReserved);
  else
    return factory_protocol_info->QueryInfo(pwzUrl, QueryOption, dwQueryFlags,
                                            pBuffer, cbBuffer, pcbBuf,
                                            dwReserved);
}

// IInternetPriority
// TODO(michaeln): our handler implements this interface for the purpose
// of passing thru only, when not in passthru mode what should we do?
// We call thru to the default handler in all cases so Set/Get sequence
// will return the value expected by the caller.
STDMETHODIMP HttpHandler::SetPriority(
    /* [in] */ LONG nPriority) {
  LOG16((L"HttpHandler::SetPriority()\n"));
  return BaseClass::SetPriority(nPriority);
}

STDMETHODIMP HttpHandler::GetPriority(
    /* [out] */ LONG *pnPriority) {
  LOG16((L"HttpHandler::GetPriority()\n"));
  return BaseClass::GetPriority(pnPriority);
}

// IInternetThreadSwitch
// TODO(michaeln): our handler implements this interface for the purpose
// of passing thru only, when not in passthru mode what should we do?
STDMETHODIMP HttpHandler::Prepare() {
  LOG16((L"HttpHandler::Prepare()\n"));
  if (is_passingthru_)
    return BaseClass::Prepare();
  else if (is_handling_)
    return S_OK;
  else
    return E_UNEXPECTED;
}

STDMETHODIMP HttpHandler::Continue() {
  LOG16((L"HttpHandler::Continue()\n"));
  if (is_passingthru_)
    return BaseClass::Continue();
  else if (is_handling_)
    return S_OK;
  else
    return E_UNEXPECTED;
}

// IWinInetInfo
STDMETHODIMP HttpHandler::QueryOption(
    /* [in] */ DWORD option,
    /* [in, out] */ LPVOID buffer,
    /* [in, out] */ DWORD *len) {
  LOG16((L"HttpHandler::QueryOption(%d)\n", option));
  if (is_passingthru_)
    return BaseClass::QueryOption(option, buffer, len);
  else if (is_handling_)
    return QueryOptionImpl(option, buffer, len);
  else
    return E_UNEXPECTED;
}

// IWinInetHttpInfo
STDMETHODIMP HttpHandler::QueryInfo(
    /* [in] */ DWORD option,
    /* [in, out] */ LPVOID buffer,
    /* [in, out] */ DWORD *len,
    /* [in, out] */ DWORD *flags,
    /* [in, out] */ DWORD *reserved) {
  LOG16((L"HttpHandler::QueryInfo(%d)\n", option));
  if (is_passingthru_)
    return BaseClass::QueryInfo(option, buffer, len, flags, reserved);
  else if (is_handling_)
    return QueryInfoImpl(option, buffer, len, flags, reserved);
  else
    return E_UNEXPECTED;
}

// IWinInetCacheHints
// TODO(michaeln): our handler implements this interface for the purpose
// of passing thru only, when not in passthru mode what should we do?
STDMETHODIMP HttpHandler::SetCacheExtension(
    /* [in] */ LPCWSTR pwzExt,
    /* [size_is][out][in] */ LPVOID pszCacheFile,
    /* [out][in] */ DWORD *pcbCacheFile,
    /* [out][in] */ DWORD *pdwWinInetError,
    /* [out][in] */ DWORD *pdwReserved) {
  LOG16((L"HttpHandler::SetCacheExtension()\n"));
  if (is_passingthru_)
    return BaseClass::SetCacheExtension(pwzExt, pszCacheFile, pcbCacheFile,
                                        pdwWinInetError, pdwReserved);
  else if (is_handling_)
    return E_FAIL;
  else
    return E_UNEXPECTED;
}

// IWinInetCacheHints2
// TODO(michaeln): our handler implements this interface for the purpose
// of passing thru only, when not in passthru mode what should we do?
STDMETHODIMP HttpHandler::SetCacheExtension2(
    /* [in] */ LPCWSTR pwzExt,
    /* [size_is][out] */ WCHAR *pwzCacheFile,
    /* [out][in] */ DWORD *pcchCacheFile,
    /* [out] */ DWORD *pdwWinInetError,
    /* [out] */ DWORD *pdwReserved) {
  LOG16((L"HttpHandler::SetCacheExtension2()\n"));
  if (is_passingthru_)
    return BaseClass::SetCacheExtension2(pwzExt, pwzCacheFile, pcchCacheFile,
                                         pdwWinInetError, pdwReserved);
  else if (is_handling_)
    return E_FAIL;
  else
    return E_UNEXPECTED;
}

//------------------------------------------------------------------------------
// Helpers to call methods of the Sink object
//------------------------------------------------------------------------------

HRESULT HttpHandler::CallReportProgress(ULONG status_code,
                                        LPCWSTR status_text) {
  if (was_terminated_ || was_aborted_ || !protocol_sink_)
    return E_UNEXPECTED;
#ifdef DEBUG
  LOG16((L"Calling ReportProgress( %s, %s )\n", GetBindStatusLabel(status_code),
         status_text ? status_text : L""));
#endif
  HRESULT hr = protocol_sink_->ReportProgress(status_code, status_text);
  if (FAILED(hr))
    LOG16((L"  protocol_sink->ReportProgress error = %d\n", hr));
  return hr;
}

HRESULT HttpHandler::CallReportData(DWORD flags, ULONG progress,
                                    ULONG progress_max) {
  if (was_terminated_ || was_aborted_ || !protocol_sink_)
    return E_UNEXPECTED;
  HRESULT hr = protocol_sink_->ReportData(flags, progress, progress_max);
  if (FAILED(hr))
    LOG16((L"  protocol_sink->ReportData error = %d\n", hr));
  return hr;
}

HRESULT HttpHandler::CallReportResult(HRESULT result,
                                      DWORD error,
                                      LPCWSTR result_text) {
  if (was_terminated_ || has_reported_result_ || !protocol_sink_)
    return E_UNEXPECTED;
  has_reported_result_ = true;
  HRESULT hr = protocol_sink_->ReportResult(result, error, result_text);
  if (FAILED(hr))
    LOG16((L"  protocol_sink->ReportResult error = %d\n", hr));
  return hr;
}

HRESULT HttpHandler::CallBeginningTransaction(LPCWSTR url,
                                              LPCWSTR headers,
                                              DWORD reserved,
                                              LPWSTR *additional_headers) {
  if (was_terminated_ || was_aborted_ || !http_negotiate_)
    return E_UNEXPECTED;
  HRESULT hr = http_negotiate_->BeginningTransaction(url, headers, reserved,
                                                     additional_headers);
  if (FAILED(hr))
    LOG16((L"  http_negotiate->BeginningTransaction error = %d\n", hr));
  return hr;
}

HRESULT HttpHandler::CallOnResponse(DWORD status_code,
                                    LPCWSTR response_headers,
                                    LPCWSTR request_headers,
                                    LPWSTR *additional_request_headers) {
  if (was_terminated_ || was_aborted_ || !http_negotiate_)
    return E_UNEXPECTED;
  HRESULT hr = http_negotiate_->OnResponse(status_code, response_headers,
                                           request_headers,
                                           additional_request_headers);
  if (FAILED(hr))
    LOG16((L"  http_negotiate->OnResponse error = %d\n", hr));
  return hr;
}


//------------------------------------------------------------------------------
// Called from both Start and StartEx, this method determines whether or not
// our handler will run in one of three possible modes. The return value and
// the state of the flags is_passingthru_ and is_handling_ indicate which mode
// should be used.
//
// 1) passthru - We delegate all method calls to an instance of the default
//      handler created by the passthru framework. The return value is
//      INET_E_USE_DEFAULT_PROTOCOLHANDLER and is_passingthru_ == true
// 2) bypassed - The Start or StartEx method will return
//      INET_E_USE_DEFAULT_PROTOCOLHANDLER which prevents this instance from
//      being used. The return value is INET_E_USE_DEFAULT_PROTOCOLHANDLER and
//      is_passingthru_ == false
// 3) handling - Our handler will be used to satisfy the request from our cache
//      The return value is S_OK and is_handling_ == true.
//------------------------------------------------------------------------------
HRESULT HttpHandler::StartImpl(LPCWSTR url,
                               IInternetProtocolSink *protocol_sink,
                               IInternetBindInfo *bind_info,
                               DWORD flags,  // PI_FLAGS
                               HANDLE_PTR reserved) {
  if (!protocol_sink || !bind_info) {
    ATLASSERT(protocol_sink);
    ATLASSERT(bind_info);
    return E_INVALIDARG;
  }

  g_active_handlers.Add(this);

  protocol_sink_ = protocol_sink;

  BINDINFO bindinfo = {0};
  DWORD bindinfoflags = 0;
  bindinfo.cbSize = sizeof(bindinfo);
  HRESULT hr = bind_info->GetBindInfo(&bindinfoflags, &bindinfo);
  if (FAILED(hr)) {
    LOG16((L"GetBindInfo failed, error = %d\n", hr));
    return hr;
  }

  // TODO(michaeln): Better understand flags, bindinfoFlags, and bindinfoOptions
  // TODO(michaeln): Better understand how our method calls should respond
  // at various points in the 'bind process'. For example, what should
  // IWinInetHttpInfo::QueryInfo return when invoked from within the caller's
  // implementation of IHttpNegotiate::BeginningTransaction?

  LOG16((L"HttpHandler::StartImpl( %s ) "
         L"flags=0x%x, bindinfoflags=0x%x, dwOptions=0x%x\n",
         url, flags, bindinfoflags, bindinfo.dwOptions));
#ifdef DEBUG
  TraceBindFlags(bindinfoflags);
#endif

  // We only directly handle GET and HEAD requests, but we run in
  // passthru mode for all requests to detect when redirects back
  // into our cache occur, see ReportProgress.
  is_head_request_ = false;
  if (bindinfo.dwBindVerb != BINDVERB_GET) {
    if ((bindinfo.dwBindVerb == BINDVERB_CUSTOM) &&
        (wcscmp(HttpConstants::kHttpHEAD, bindinfo.szCustomVerb) == 0)) {
      LOG16((L" HEAD request"));
      is_head_request_ = true;
    } else {
      LOG16((L"  not a GET or HEAD request - "
             L"passing thru to default handler\n"));
      is_passingthru_ = true;
      return INET_E_USE_DEFAULT_PROTOCOLHANDLER;
    }
  }

  // When capturing resources and fetching manifest files we bypass our cache
  CComQIPtr<IServiceProvider> service_provider = protocol_sink;
  if (service_provider) {
    MutexLock lock(&global_mutex);
    CComPtr<IUnknown> not_used;
    bypass_cache = false;
    service_provider->QueryService(SID_QueryBypassCache,
                                   IID_QueryBypassCache,
                                   reinterpret_cast<void**>(&not_used));
    if (bypass_cache) {
      LOG16((L"  by passing cache - using default protocol handler\n"));
      return INET_E_USE_DEFAULT_PROTOCOLHANDLER;
    }
  }

  // Fetch a response from our DB for this url
  WebCacheDB* db = WebCacheDB::GetDB();
  if (!db) {
    return INET_E_USE_DEFAULT_PROTOCOLHANDLER;
  }
  bool is_captured = db->Service(url, NULL, is_head_request_, &payload_);

  if (!is_captured) {
    LOG16((L"  cache miss - passing thru to default protocol handler\n"));
    if (kAlertCacheMiss && !ActiveXUtils::IsOnline()) {
      MessageBoxW(NULL, url, L"WebCapture cache miss",
                  MB_OK | MB_SETFOREGROUND | MB_TOPMOST);
    }
    is_passingthru_ = true;
    return INET_E_USE_DEFAULT_PROTOCOLHANDLER;
  }

#ifdef WINCE
  // Re-insert the cache entry to make sure it's there. We add an entry whether
  // or not this is a redirect.
  BrowserCache::EnsureBogusEntry(url);
#endif

  // The requested url may redirect to another location
  std::string16 redirect_url;
  if (payload_.IsHttpRedirect()) {
    // We collapse a chain of redirects and hop directly to the final
    // location for which we have a cache entry.
    int num_redirects = 0;
    while (payload_.IsHttpRedirect()) {
      if (!payload_.GetHeader(HttpConstants::kLocationHeader, &redirect_url)) {
        LOG16((L"  redirect with no location - using default handler\n"));
        is_passingthru_ = true;
        return INET_E_USE_DEFAULT_PROTOCOLHANDLER;
      }
      // Fetch a response for redirect_url from our DB
      if (!db->Service(redirect_url.c_str(), NULL, is_head_request_, &payload_)) {
        // We don't have a response for redirect_url. So report
        // INET_E_REDIRECT_FAILED which causes the aggregating outer
        // object (URLMON) to create a new inner APP and release us.
        LOG16((L"  cache hit, almost - redirect out of cache ( %s )\n",
               redirect_url.c_str()));
        is_handling_ = true;  // set to true so we respond to QueryInfo calls
        return CallReportResult(INET_E_REDIRECT_FAILED,
                                HttpConstants::HTTP_FOUND,
                                redirect_url.c_str());
      }
      
      // Make sure we don't get stuck in an infinite redirect loop.
      ++num_redirects;
      if (num_redirects > HttpConstants::kMaxRedirects) {
        LOG16((STRING16(L"Redirect chain too long %s -> %s"),
               url, redirect_url.c_str()));
        return E_FAIL;
      }
    }
    LOG16((L"  cache hit - redirect ( %s )\n", redirect_url.c_str()));
  } else {
    LOG16((L"  cache hit\n"));
  }

  // Ok, we're going to satisfy this request from our cache
  is_handling_ = true;

  if (service_provider) {
    service_provider->QueryService(__uuidof(IHttpNegotiate),
                                   __uuidof(IHttpNegotiate),
                                   reinterpret_cast<void**>(&http_negotiate_));
  }

  if (http_negotiate_) {
    WCHAR *extra_headers = NULL;
    hr = CallBeginningTransaction(url, L"", 0, &extra_headers);
    CoTaskMemFree(extra_headers);
    if (FAILED(hr)) return hr;
  }

  if (!redirect_url.empty()) {
    hr = CallReportProgress(BINDSTATUS_REDIRECTING, redirect_url.c_str());
    if (FAILED(hr)) return hr;
  }

  // Drive the IHttpNegotiate interface if this interface is provided
  // by the client, this is how XmlHttpRequest accesses header information
  if (http_negotiate_) {
    WCHAR *extra_headers = NULL;
    std::string16 statusline_and_headers;
    statusline_and_headers = payload_.status_line;
    statusline_and_headers += HttpConstants::kCrLf;
    statusline_and_headers += payload_.headers;
    hr = CallOnResponse(HttpConstants::HTTP_OK, statusline_and_headers.c_str(),
                        L"", &extra_headers);
    CoTaskMemFree(extra_headers);
    if (FAILED(hr)) return hr;
  }

  // Drive the IInternetProtocolSink interface
  std::string16 mimetype;
  payload_.GetHeader(HttpConstants::kContentTypeHeader, &mimetype);
  if (!mimetype.empty()) {
    hr = CallReportProgress(BINDSTATUS_MIMETYPEAVAILABLE, mimetype.c_str());
    if (FAILED(hr)) return hr;
  }

  // The cached_filepath is not provided for head requests
  if (!is_head_request_ && !payload_.cached_filepath.empty()) {
    // One way to support file downloads is to provide a file path, I don't
    // know if there are other options?
      hr = CallReportProgress(BINDSTATUS_CACHEFILENAMEAVAILABLE,
                              payload_.cached_filepath.c_str());
      if (FAILED(hr)) return hr;

    // Report content disposition, including a file path
    // TODO(michaeln): a better parser for this header value, perhaps CAtlRegExp
    // The value uses a character encoding that is particular to this header
    // The value has multiple semi-colon delimited fields
    // See the following for an implementation
    // '//depot/google3/java/com/google/httputil/ContentDisposition.java'
    // '//depot/google3/java/com/google/parser/Parser.java'
    std::string16 content_disposition;
    payload_.GetHeader(L"Content-Disposition", &content_disposition);
    if (content_disposition.find(L"attachment") != std::string16::npos) {
      hr = CallReportProgress(BINDSTATUS_CONTENTDISPOSITIONATTACH,
                              payload_.cached_filepath.c_str());
      if (FAILED(hr)) return hr;
    }
  }

  size_t response_size = payload_.data.get() ? payload_.data->size() : 0;

  hr = CallReportData(BSCF_DATAFULLYAVAILABLE |
                      BSCF_FIRSTDATANOTIFICATION |
                      BSCF_LASTDATANOTIFICATION,
                      static_cast<ULONG>(response_size),
                      static_cast<ULONG>(response_size));
  if (FAILED(hr)) return hr;

  std::string16 status_text;
  if (!ParseHttpStatusLine(payload_.status_line, NULL, NULL, &status_text)) {
    // We never expect to get here because when inserting payloads into
    // the database, we verify that they can be parsed w/o error. To make
    // ReportResults happy we return a made up value.
    assert(false);
    status_text = L"UNKNOWN";
  }

  hr = CallReportResult(S_OK, payload_.status_code, status_text.c_str());
  if (FAILED(hr)) return hr;

  LOG16((L"HttpHandler::StartImpl( %s, %d ): YES\n", url, response_size));
  return S_OK;
}

//------------------------------------------------------------------------------
// Called from Read, this method implements Read when we're satisfying
// a request from our cache
//------------------------------------------------------------------------------
HRESULT HttpHandler::ReadImpl(void *buffer,
                              ULONG byte_count,
                              ULONG *bytes_read) {
  LOG16((L"HttpHandler::ReadImpl(%d)\n", byte_count));
  if (is_handling_) {
    std::vector<uint8> *data = payload_.data.get();
    size_t bytes_available = data ? (data->size() - read_pointer_) : 0;
    size_t bytes_to_copy = std::min<size_t>(byte_count, bytes_available);

    if (bytes_to_copy != 0) {
      memcpy(buffer, &(*data)[read_pointer_], bytes_to_copy);
      read_pointer_ += bytes_to_copy;
    }

    if (bytes_read != NULL) {
      *bytes_read = static_cast<ULONG>(bytes_to_copy);
    }

    if (bytes_available - bytes_to_copy == 0) {
      LOG16((L"----> HttpHandler::ReadImpl() complete\n"));
      return S_FALSE;
    } else {
      return S_OK;
    }
  } else {
    LOG16((L"----> HttpHandler::ReadImpl() E_UNEXPECTED\n"));
    assert(false);
    return E_UNEXPECTED;
  }
}

//------------------------------------------------------------------------------
// IWinInetInfo::QueryOptionImpl
// Called from QueryOption, this method implements QueryOption when we're
// satisfying a request from our cache
//------------------------------------------------------------------------------
HRESULT HttpHandler::QueryOptionImpl(/* [in] */ DWORD dwOption,
                                     /* [in, out] */ LPVOID pBuffer,
                                     /* [in, out] */ DWORD *pcbBuf) {
#ifdef DEBUG
  LOG16((L"HttpHandler::QueryOption(%s (%d))\n",
         GetWinInetInfoLabel(dwOption), dwOption));
#endif

  if (!is_handling_) {
    return E_UNEXPECTED;
  }

  // TODO(michaeln): determine which options we need to support and how?
  // Options of interests
  //   INTERNET_OPTION_DATAFILE_NAME
  //   INTERNET_OPTION_REQUEST_FLAGS
  //   INTERNET_OPTION_SECURITY_FLAGS
  //   unknown option with dwOption value of 66
  //
  //switch (dwOption) {
  //  case INTERNET_OPTION_REQUEST_FLAGS:
  //  case INTERNET_OPTION_SECURITY_FLAGS:
  //    if (*pcbBuf < sizeof(DWORD)) {
  //      *pcbBuf = sizeof(DWORD);
  //    } else {
  //      DWORD *number_out = reinterpret_cast<DWORD*>(pBuffer);
  //      *number_out = 0;
  //      *pcbBuf = sizeof(DWORD);
  //      return S_OK;
  //    }
  //    break;
  //}

  return S_FALSE;
}


//------------------------------------------------------------------------------
// IWinInetHttpInfo::QueryInfoImpl
// Called from QueryInfo, this method implements QueryInfo when we're
// satisfying a request from our cache
//------------------------------------------------------------------------------
HRESULT HttpHandler::QueryInfoImpl(/* [in] */ DWORD dwOption,
                                   /* [in, out] */ LPVOID pBuffer,
                                   /* [in, out] */ DWORD *pcbBuf,
                                   /* [in, out] */ DWORD *pdwFlags,
                                   /* [in, out] */ DWORD *pdwReserved) {
  DWORD flags = dwOption & HTTP_QUERY_MODIFIER_FLAGS_MASK;
  dwOption &= HTTP_QUERY_HEADER_MASK;
  bool flag_request_headers = (flags & HTTP_QUERY_FLAG_REQUEST_HEADERS) != 0;
  bool flag_systemtime = (flags & HTTP_QUERY_FLAG_SYSTEMTIME) != 0;
  bool flag_number = (flags & HTTP_QUERY_FLAG_NUMBER) != 0;
  bool flag_coalesce = (flags & HTTP_QUERY_FLAG_COALESCE) != 0;

#ifdef DEBUG
  LOG16((L"HttpHandler::QueryInfo(%s (%d), %d)\n",
         GetWinInetHttpInfoLabel(dwOption),
         dwOption,
         pdwFlags ? *pdwFlags : -1));
#endif

  if (!is_handling_) {
    return E_UNEXPECTED;
  }

  // We don't respond to queries about request headers
  if (flag_request_headers) {
    SetLastError(ERROR_HTTP_HEADER_NOT_FOUND);
    return S_FALSE;
  }

  // Most queries have to do with a particular header, but not all.
  // We special case those that don't map to a header, and by default
  // lookup a header value
  const char16 *value = NULL;
  std::string16 value_str;
  int value_len = -1;
  switch (dwOption) {
    case HTTP_QUERY_STATUS_CODE:
      if (IsValidResponseCode(payload_.status_code)) {
        value_str = IntegerToString16(payload_.status_code);
        value = value_str.c_str();
        value_len = value_str.length();
      }
      break;

    case HTTP_QUERY_STATUS_TEXT:
      if (ParseHttpStatusLine(payload_.status_line, NULL, NULL, &value_str)) {
        value = value_str.c_str();
        value_len = value_str.length();
      }
      break;

    case HTTP_QUERY_VERSION:
      if (ParseHttpStatusLine(payload_.status_line, &value_str, NULL, NULL)) {
        value = value_str.c_str();
        value_len = value_str.length();
      }
      break;

    case HTTP_QUERY_REQUEST_METHOD:
      if (is_head_request_) {
        value = HttpConstants::kHttpHEAD;
      } else {
        value = HttpConstants::kHttpGET;
      }
      break;

    case HTTP_QUERY_RAW_HEADERS:
      // The returned value includes the status line followed by each header.
      // Each line is terminated by "\0" and an additional "\0" terminates
      // the list of headers
      value_str = payload_.status_line;
      value_str += HttpConstants::kCrLf;
      value_str += payload_.headers;
      ReplaceAll(value_str,
                 std::string16(HttpConstants::kCrLf),
                 std::string16(1, '\0'));  // string containing an embedded NULL
      value_str.append('\0');
      value = value_str.c_str();
      value_len = value_str.length();
      break;

    case HTTP_QUERY_RAW_HEADERS_CRLF:
      // The returned value includes the status line followed by each header.
      // Each line is separated by a carriage return/line feed sequence
      value_str = payload_.status_line;
      value_str += HttpConstants::kCrLf;
      value_str += payload_.headers;
      value = value_str.c_str();
      value_len = value_str.length();
      break;

    default:
      // Lookup a particular header value
      // TODO(michaeln): flesh out the table of options to header values
      // contained in GetWinInetHttpInfoHeaderName, there are many options
      // not mapped to a header_name yet
      const char16* header_name = GetWinInetHttpInfoHeaderName(dwOption);
      if (header_name && payload_.GetHeader(header_name, &value_str)) {
        value = value_str.c_str();
        value_len = value_str.length();
      }
      break;
  }

  if (!value) {
    SetLastError(ERROR_HTTP_HEADER_NOT_FOUND);
    return S_FALSE;
  }

  // The high-bits of dwOptions contain flags that influence how values
  // are returned

  if (!flags) {
    // Caller is asking for a char string value
    // TODO(michaeln): what character encoding should be used, for now UTF8
    if (value_len < 0) {
      value_len = wcslen(value);
    }
    std::string value8;
    if (!String16ToUTF8(value, value_len, &value8)) {
      return E_FAIL;
    }
    if (!pBuffer) {
      *pcbBuf = value8.length() + 1;
      SetLastError(ERROR_INSUFFICIENT_BUFFER);
      return S_FALSE;
    } else {
      if (*pcbBuf <= value8.length()) {
        *pcbBuf = value8.length() + 1;
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return S_FALSE;
      } else {
        *pcbBuf = value8.length();
        strcpy(reinterpret_cast<char*>(pBuffer), value8.c_str());
        return S_OK;
      }
    }
  } else if (flag_number) {
    // Caller is asking for a 32-bit integer value
    if (*pcbBuf < sizeof(int32)) {
      *pcbBuf = sizeof(int32);
      SetLastError(ERROR_INSUFFICIENT_BUFFER);
      return S_FALSE;
    } else {
      *pcbBuf = sizeof(int32);
      int32 *number_out = reinterpret_cast<int32*>(pBuffer);
      *number_out = static_cast<int32>(_wtol(value));
      return S_OK;
    }
  } else if (flag_systemtime) {
    // Caller is asking for a SYSTEMTIME value
    if (*pcbBuf < sizeof(SYSTEMTIME)) {
      *pcbBuf = sizeof(SYSTEMTIME);
      SetLastError(ERROR_INSUFFICIENT_BUFFER);
      return S_FALSE;
    } else {
      *pcbBuf = sizeof(SYSTEMTIME);
      SYSTEMTIME *system_time = reinterpret_cast<SYSTEMTIME*>(pBuffer);
      if (!InternetTimeToSystemTimeW(value, system_time, 0)) {
        return E_FAIL;
      }
      return S_OK;
    }
  } else if (flag_coalesce) {
    // MSDN says this flag is not implemented
    return E_NOTIMPL;
  } else {
    // shouldn't get here
    assert(false);
    return E_FAIL;
  }
}

//------------------------------------------------------------------------------
// HttpHandlerFactory
//------------------------------------------------------------------------------

STDMETHODIMP HttpHandlerFactory::ParseUrl(LPCWSTR pwzUrl,
                                          PARSEACTION ParseAction,
                                          DWORD dwParseFlags,
                                          LPWSTR pwzResult,
                                          DWORD cchResult,
                                          DWORD *pcchResult,
                                          DWORD reserved) {
  return INET_E_DEFAULT_ACTION;
}

STDMETHODIMP HttpHandlerFactory::CombineUrl(LPCWSTR baseUrl,
                                            LPCWSTR relateiveUrl,
                                            DWORD dwCombineFlags,
                                            LPWSTR pwzResult,
                                            DWORD cchResult,
                                            DWORD *pcchResult,
                                            DWORD reserved) {
  return INET_E_DEFAULT_ACTION;
}

STDMETHODIMP HttpHandlerFactory::CompareUrl(LPCWSTR pwzUrl1,
                                            LPCWSTR pwzUrl2,
                                            DWORD dwCompareFlags) {
  return INET_E_DEFAULT_ACTION;
}

// Helper to pass BOOL return values from IInternetProtocolInfo::QueryInfo
static HRESULT ReturnBoolean(bool value, LPVOID pBuffer,
                             DWORD cbBuffer, DWORD *pcbBuf) {
  if (cbBuffer < sizeof(BOOL)) {
    return S_FALSE;
  }
  if (pBuffer != NULL) {
    *static_cast<BOOL*>(pBuffer) = value ? TRUE : FALSE;
  }
  if (pcbBuf != NULL) {
    *pcbBuf = sizeof(BOOL);
  }
  return S_OK;
}

STDMETHODIMP HttpHandlerFactory::QueryInfo(LPCWSTR pwzUrl,
                                           QUERYOPTION queryOption,
                                           DWORD dwQueryFlags,
                                           LPVOID pBuffer,
                                           DWORD cbBuffer,
                                           DWORD *pcbBuf,
                                           DWORD reserved) {
  // We are careful to only intercept queries about the cachedness of urls.
  // As a page loads, this method is called multiple times for each embedded
  // resource with a queryOption unrelated to cachedness. We don't want to
  // hit the database in those unrelated cases.
  bool result;
  switch (queryOption) {
    case QUERY_USES_NETWORK:
      // Checks if the URL needs to access the network
      result = false;
      break;

    case QUERY_IS_CACHED:
      // Checks if the resource is cached locally
      result = true;
      break;

    case QUERY_IS_CACHED_OR_MAPPED:
      // Checks if this resource is stored in the cache or if it is on a
      // mapped drive (in a cache container)
      result = true;
      break;

    default:
      return INET_E_DEFAULT_ACTION;
  }

  // Determine if we would serve this url from our cache at this time. If
  // not, defer to the default handling.
  WebCacheDB* db = WebCacheDB::GetDB();
  if (!db || !db->CanService(pwzUrl, NULL)) {
    return INET_E_DEFAULT_ACTION;
  }

#ifdef DEBUG
  LOG16((L"HttpHandler::IInternetProtocolInfo::QueryInfo(%s (%d), %s)\n",
         GetProtocolInfoLabel(queryOption), queryOption, pwzUrl));
#endif

  return ReturnBoolean(result, pBuffer, cbBuffer, pcbBuf);
}

