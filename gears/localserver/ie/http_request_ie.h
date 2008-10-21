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

#ifndef GEARS_LOCALSERVER_IE_HTTP_REQUEST_IE_H__
#define GEARS_LOCALSERVER_IE_HTTP_REQUEST_IE_H__

#include <string>
#include <vector>
#include "gears/base/common/security_model.h"
#include "gears/base/common/scoped_refptr.h"
#include "gears/base/ie/atl_headers.h"
#include "gears/localserver/common/http_request.h"
#include "gears/localserver/common/localserver_db.h"
#include "gears/localserver/common/progress_event.h"
#include "gears/localserver/ie/progress_input_stream.h"

class BlobInterface;
class ByteStore;

class IEHttpRequest
    : public CComObjectRootEx<CComMultiThreadModel::ThreadModelNoCS>,
      public IBindStatusCallback,
      public IHttpNegotiate,
      public IServiceProviderImpl<IEHttpRequest>,
      public HttpRequest,
      public ProgressEvent::Listener {
 public:
  // HttpRequest interface

  // refcounting
  virtual void Ref();
  virtual void Unref();

  // Get or set whether to use or bypass caches, the default is USE_ALL_CACHES
  // May only be set prior to calling Send.
  virtual CachingBehavior GetCachingBehavior() {
    return caching_behavior_;
  }

  virtual bool SetCachingBehavior(CachingBehavior behavior) {
    if (!(IsUninitialized() || IsOpen())) return false;
    caching_behavior_ = behavior;
    return true;
  }

  // Get or set the redirect behavior, the default is FOLLOW_ALL
  // May only be set prior to calling Send.
  virtual RedirectBehavior GetRedirectBehavior() {
    return redirect_behavior_;
  }

  virtual bool SetRedirectBehavior(RedirectBehavior behavior) {
    if (!(IsUninitialized() || IsOpen())) return false;
    redirect_behavior_ = behavior;
    return true;
  }

  // Set whether browser cookies are sent with the request. The default is
  // SEND_BROWSER_COOKIES. May only be set prior to calling Send.
  virtual bool SetCookieBehavior(CookieBehavior behavior) {
    if (!(IsUninitialized() || IsOpen())) return false;
    cookie_behavior_ = behavior;
    return true;
  }

  // properties
  virtual bool GetReadyState(ReadyState *state);
  virtual bool GetResponseBody(scoped_refptr<BlobInterface>* blob);
  virtual bool GetStatus(int *status);
  virtual bool GetStatusText(std::string16 *status_text);
  virtual bool GetStatusLine(std::string16 *status_line);

  virtual bool WasRedirected();
  virtual bool GetFinalUrl(std::string16 *full_url);
  virtual bool GetInitialUrl(std::string16 *full_url);

  // methods
  virtual bool Open(const char16 *method, const char16* url, bool async,
                    BrowsingContext *browsing_context);
  virtual bool SetRequestHeader(const char16* name, const char16* value);
  virtual bool Send(BlobInterface *data);
  virtual bool GetAllResponseHeaders(std::string16 *headers);
  virtual std::string16 GetResponseCharset();
  virtual bool GetResponseHeader(const char16* name, std::string16 *header);
  virtual bool Abort();

  // events
  virtual bool SetListener(HttpListener *listener, bool enable_data_available);

  // IE implementation specific

  BEGIN_COM_MAP(IEHttpRequest)
    COM_INTERFACE_ENTRY(IBindStatusCallback)
    COM_INTERFACE_ENTRY(IHttpNegotiate)
    COM_INTERFACE_ENTRY(IServiceProvider)
  END_COM_MAP()

  BEGIN_SERVICE_MAP(IEHttpRequest)
    SERVICE_ENTRY(__uuidof(IHttpNegotiate))
  END_SERVICE_MAP()

  IEHttpRequest();
  HRESULT FinalConstruct();
  void FinalRelease();

  // IBindStatusCallback

  virtual HRESULT STDMETHODCALLTYPE OnStartBinding(
      /* [in] */ DWORD dwReserved,
      /* [in] */ IBinding *pib);

  virtual HRESULT STDMETHODCALLTYPE GetPriority(
      /* [out] */ LONG *pnPriority);

  virtual HRESULT STDMETHODCALLTYPE OnLowResource(
      /* [in] */ DWORD reserved);

  virtual HRESULT STDMETHODCALLTYPE OnProgress(
      /* [in] */ ULONG ulProgress,
      /* [in] */ ULONG ulProgressMax,
      /* [in] */ ULONG ulStatusCode,
      /* [in] */ LPCWSTR szStatusText);

  virtual HRESULT STDMETHODCALLTYPE OnStopBinding(
      /* [in] */ HRESULT hresult,
      /* [unique][in] */ LPCWSTR szError);

  virtual HRESULT STDMETHODCALLTYPE GetBindInfo(
      /* [out] */ DWORD *grfBINDF,
      /* [unique][out][in] */ BINDINFO *pbindinfo);

  virtual HRESULT STDMETHODCALLTYPE OnDataAvailable(
      /* [in] */ DWORD grfBSCF,
      /* [in] */ DWORD dwSize,
      /* [in] */ FORMATETC *pformatetc,
      /* [in] */ STGMEDIUM *pstgmed);

  virtual HRESULT STDMETHODCALLTYPE OnObjectAvailable(
      /* [in] */ REFIID riid,
      /* [iid_is][in] */ IUnknown *punk);

  // IHttpNegotiate

  virtual HRESULT STDMETHODCALLTYPE BeginningTransaction(
      /* [in] */ LPCWSTR szURL,
      /* [unique][in] */ LPCWSTR szHeaders,
      /* [in] */ DWORD dwReserved,
      /* [out] */ LPWSTR *pszAdditionalHeaders);

  virtual HRESULT STDMETHODCALLTYPE OnResponse(
      /* [in] */ DWORD dwResponseCode,
      /* [unique][in] */ LPCWSTR szResponseHeaders,
      /* [unique][in] */ LPCWSTR szRequestHeaders,
      /* [out] */ LPWSTR *pszAdditionalRequestHeaders);

  // IServiceProvider

  virtual HRESULT STDMETHODCALLTYPE QueryService(
      /* [in] */ REFGUID guidService,
      /* [in] */ REFIID riid,
      /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);

 private:
  // ProgressEvent::Listener implementation
  virtual void OnUploadProgress(int64 position, int64 total);

  HRESULT OnRedirect(const char16 *redirect_url);
  void SetReadyState(ReadyState state);
  bool IsUninitialized() { return ready_state_ == HttpRequest::UNINITIALIZED; }
  bool IsOpen() { return ready_state_ == HttpRequest::OPEN; }
  bool IsSent() { return ready_state_ == HttpRequest::SENT; }
  bool IsInteractive() { return ready_state_ == HttpRequest::INTERACTIVE; }
  bool IsComplete() { return ready_state_ == HttpRequest::COMPLETE; }
  bool IsInteractiveOrComplete() { return IsInteractive() || IsComplete(); }
  bool IsPostOrPut() {
    return bind_verb_ == BINDVERB_POST ||
           bind_verb_ == BINDVERB_PUT;
  }
  bool IsFileGet() {
#ifdef OFFICIAL_BUILD
    return false;
#else
    return method_ == HttpConstants::kHttpGET &&
           origin_.scheme() == HttpConstants::kFileScheme;
#endif
  }

  // The (non-relative) request url
  std::string16 url_;
  SecurityOrigin origin_;

  // Whether the request should be performed asynchronously
  bool async_;

  // Whether to bypass caches
  CachingBehavior caching_behavior_;

  // Whether to follow redirects
  RedirectBehavior redirect_behavior_;

  // Whether to include browser cookies in the request.
  CookieBehavior cookie_behavior_;

  // The request method
  std::string16 method_;
  int bind_verb_;

  // The POST data
  friend ProgressInputStream;
  scoped_refptr<BlobInterface> post_data_;
  CComPtr<ProgressInputStream> post_data_stream_;

  // Additional request headers we've been asked to send with the request
  std::string16 additional_headers_;

  // Our XmlHttpRequest like ready state, 0 thru 4
  ReadyState ready_state_;

  // Whether this request was aborted by the caller
  bool was_aborted_;

  // Used to distinguish between actual binding errors and intentionally
  // induced failures (see OnStopBinding).
  bool ignore_stopbinding_error_;

  // Whether or not we have been redirected
  bool was_redirected_;

  // Whether or not we should disable sending browser cookies with the request.
  bool disable_browser_cookies_;

  // If we've been redirected, the location of the redirect. If we experience
  // a chain of redirects, this will be the last in the chain upon completion.
  std::string16 redirect_url_;

  // Our listener
  HttpRequest::HttpListener *listener_;
  bool listener_data_available_enabled_;

  // We populate this structure with various pieces of response data:
  // status code, status line, headers, data
  WebCacheDB::PayloadInfo response_payload_;
  scoped_refptr<ByteStore> response_body_;

  bool has_synthesized_response_payload_;

  // URLMON object references
  CComPtr<IMoniker> url_moniker_;
  CComPtr<IBindCtx> bind_ctx_;
  CComPtr<IBinding> binding_;
};

#endif  // GEARS_LOCALSERVER_IE_HTTP_REQUEST_IE_H__
