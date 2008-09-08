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

#include <msxml2.h>
#include <algorithm>
#include <vector>
#include <wininet.h>  // For INTERNET_FLAG_NO_COOKIES

#include "gears/localserver/ie/http_request_ie.h"

#include "gears/base/common/byte_store.h"
#include "gears/base/common/http_utils.h"
#include "gears/base/common/string_utils.h"
#include "gears/base/common/url_utils.h"
#include "gears/base/ie/atl_headers.h"
#include "gears/blob/blob_interface.h"
#include "gears/blob/blob_stream_ie.h"
#include "gears/blob/buffer_blob.h"
#include "gears/localserver/ie/http_handler_ie.h"
#include "gears/localserver/ie/urlmon_utils.h"

// We use URLMON's pull-data model which requires making stream read calls
// beyond the amount of data currently available to force the underlying
// stream implementation to read more data from the network. This constant
// determines how much data beyond the end we'll request for this purpose.
static const int kReadAheadAmount = 16 * 1024;

//------------------------------------------------------------------------------
// Create
//------------------------------------------------------------------------------
// static
bool HttpRequest::Create(scoped_refptr<HttpRequest>* request) {
  CComObject<IEHttpRequest> *ie_request;
  HRESULT hr = CComObject<IEHttpRequest>::CreateInstance(&ie_request);
  if (FAILED(hr)) {
    LOG16((L"HttpRequest::Create - CreateInstance failed - %d\n", hr));
    return false;
  }
  request->reset(ie_request);
  return true;
}

// static
bool HttpRequest::CreateSafeRequest(scoped_refptr<HttpRequest>* request) {
  return HttpRequest::Create(request);
}

//------------------------------------------------------------------------------
// Construction, destruction and refcounting
//------------------------------------------------------------------------------

IEHttpRequest::IEHttpRequest()
    : caching_behavior_(USE_ALL_CACHES),
      redirect_behavior_(FOLLOW_ALL),
      cookie_behavior_(SEND_BROWSER_COOKIES),
      was_redirected_(false),
      was_aborted_(false),
      ignore_stopbinding_error_(false),
      listener_(NULL),
      listener_data_available_enabled_(false),
      ready_state_(UNINITIALIZED),
      has_synthesized_response_payload_(false),
      async_(false) {
}

HRESULT IEHttpRequest::FinalConstruct() {
  return S_OK;
}

void IEHttpRequest::FinalRelease() {
  if (post_data_stream_) {
    post_data_stream_->DetachRequest();
    post_data_stream_.Release();
  }
}

void IEHttpRequest::Ref() {
  AddRef();
}

void IEHttpRequest::Unref() {
  Release();
}

//------------------------------------------------------------------------------
// GetReadyState
//------------------------------------------------------------------------------
bool IEHttpRequest::GetReadyState(ReadyState *state) {
  *state = ready_state_;
  return true;
}

//------------------------------------------------------------------------------
// GetResponseBody
//------------------------------------------------------------------------------
bool IEHttpRequest::GetResponseBody(scoped_refptr<BlobInterface> *blob) {
  assert(blob);
  if (!IsInteractiveOrComplete() || was_aborted_) {
    return false;
  }
  response_body_->CreateBlob(blob);
  return true;
}

//------------------------------------------------------------------------------
// GetStatus
//------------------------------------------------------------------------------
bool IEHttpRequest::GetStatus(int *status) {
  if (!IsInteractiveOrComplete() || was_aborted_)
    return false;
  *status = response_payload_.status_code;
  return true;
}

//------------------------------------------------------------------------------
// GetStatusText
// TODO(michaeln): remove this method from the interface, prefer getStatusLine
//------------------------------------------------------------------------------
bool IEHttpRequest::GetStatusText(std::string16 *status_text) {
  if (!IsInteractiveOrComplete() || was_aborted_)
    return false;
  return ParseHttpStatusLine(response_payload_.status_line,
                             NULL, NULL, status_text);
}

//------------------------------------------------------------------------------
// GetStatusLine
//------------------------------------------------------------------------------
bool IEHttpRequest::GetStatusLine(std::string16 *status_line) {
  if (!IsInteractiveOrComplete() || was_aborted_)
    return false;
  *status_line = response_payload_.status_line;
  return true;
}

//------------------------------------------------------------------------------
// GetAllResponseHeaders
//------------------------------------------------------------------------------
bool IEHttpRequest::GetAllResponseHeaders(std::string16 *headers) {
  if (!IsInteractiveOrComplete() || was_aborted_)
    return false;
  headers->assign(response_payload_.headers);
  return true;
}

//------------------------------------------------------------------------------
// GetResponseCharset
//------------------------------------------------------------------------------
std::string16 IEHttpRequest::GetResponseCharset() {
  // TODO(bgarcia): If the document sets the Content-Type charset, return
  // that value.
  // Also need to update blob/blob_utils.cc.
  return std::string16();
}

//------------------------------------------------------------------------------
// GetResponseHeader
//------------------------------------------------------------------------------
bool IEHttpRequest::GetResponseHeader(const char16* name,
                                      std::string16 *value) {
  if (!IsInteractiveOrComplete() || was_aborted_)
    return false;
  return response_payload_.GetHeader(name, value);
}

//------------------------------------------------------------------------------
// Open
//------------------------------------------------------------------------------
bool IEHttpRequest::Open(const char16 *method, const char16* url, bool async,
                    BrowsingContext *browsing_context) {
  assert(!IsRelativeUrl(url));
  if (!IsUninitialized())
    return false;

  async_ = async;
  url_ = url;
  if (!origin_.InitFromUrl(url)) {
    return false;
  }
  method_ = method;
  UpperString(method_);
  if (method_ == HttpConstants::kHttpGET)
    bind_verb_ = BINDVERB_GET;
  else if (method_ == HttpConstants::kHttpPOST)
    bind_verb_ = BINDVERB_POST;
  else if (method == HttpConstants::kHttpPUT)
    bind_verb_ = BINDVERB_PUT;
  else
    bind_verb_ = BINDVERB_CUSTOM;
  SetReadyState(HttpRequest::OPEN);
  return true;
}

//------------------------------------------------------------------------------
// SetRequestHeader
// Here we gather additional request headers to be sent. They are plumbed
// into URLMON in our IHttpNegotiate::BeginningTransaction method.
//------------------------------------------------------------------------------
bool IEHttpRequest::SetRequestHeader(const char16* name, const char16* value) {
  if (!IsOpen())
    return false;
  additional_headers_ += name;
  additional_headers_ += L": ";
  additional_headers_ += value;
  additional_headers_ += HttpConstants::kCrLf;
  return true;
}

bool IEHttpRequest::WasRedirected() {
  return IsInteractiveOrComplete() && !was_aborted_ && was_redirected_;
}

bool IEHttpRequest::GetFinalUrl(std::string16 *full_url) {
  if (!IsInteractiveOrComplete() || was_aborted_)
    return false;

  if (WasRedirected())
    *full_url = redirect_url_;
  else
    *full_url = url_;
  return true;
}

bool IEHttpRequest::GetInitialUrl(std::string16 *full_url) {
  *full_url = url_;  // may be empty if request has not occurred
  return true;
}

//------------------------------------------------------------------------------
// Send
//------------------------------------------------------------------------------

bool IEHttpRequest::Send(BlobInterface *blob) {
  if (!IsOpen()) return false;
  if (IsPostOrPut()) {
    post_data_ = blob ? blob : new EmptyBlob;
  } else if (blob) {
    return false;
  }

  if (post_data_.get()) {
    // TODO(bpm): do we have to set this or will URLMON do so based
    // on the size of our stream?
    std::string16 size_str = Integer64ToString16(post_data_->Length());
    SetRequestHeader(HttpConstants::kContentLengthHeader, size_str.c_str());
  }

  // The request can complete prior to Send returning depending on whether
  // the response is retrieved from the cache. We guard against a caller's
  // listener removing the last reference prior to return by adding our own
  // reference here.
  CComPtr<IUnknown> reference(_GetRawUnknown());

  if (!IsOpen() || url_.empty())
    return false;

  HRESULT hr = CreateURLMonikerEx(NULL, url_.c_str(), &url_moniker_,
                                  URL_MK_UNIFORM);
  if (FAILED(hr)) {
    return false;
  }
  hr = CreateBindCtx(0, &bind_ctx_);
  if (FAILED(hr)) {
    return false;
  }
  hr = RegisterBindStatusCallback(bind_ctx_,
                                  static_cast<IBindStatusCallback*>(this),
                                  0, 0L);
  if (FAILED(hr)) {
    return false;
  }
  CComPtr<IStream> stream;
  hr = url_moniker_->BindToStorage(bind_ctx_, 0,
                                   __uuidof(IStream),
                                   reinterpret_cast<void**>(&stream));
  if (FAILED(hr)) {
    return false;
  }

  return !was_aborted_;
}

//------------------------------------------------------------------------------
// Abort
//------------------------------------------------------------------------------
bool IEHttpRequest::Abort() {
  if (!binding_)
    return true;
  HRESULT hr = binding_->Abort();
  was_aborted_ = true;
  return SUCCEEDED(hr);
}

//------------------------------------------------------------------------------
// SetListener
//------------------------------------------------------------------------------
bool IEHttpRequest::SetListener(HttpListener *listener,
                                bool enable_data_available) {
  listener_ = listener;
  listener_data_available_enabled_ = enable_data_available;
  return true;
}

void IEHttpRequest::SetReadyState(ReadyState state) {
  if (state > ready_state_) {
    ready_state_ = state;
    if (listener_) {
      listener_->ReadyStateChanged(this);
    }
  }
}

void IEHttpRequest::OnUploadProgress(int64 position, int64 total) {
  if (was_aborted_) return;
  if (listener_) {
    listener_->UploadProgress(this, position, total);
  }
}

//------------------------------------------------------------------------------
// IServiceProvider::QueryService
// Implemented to return an interface pointer for IHttpNegotiate and
// to conduct hand-shaking with our HttpHandler to bypass our webcache.
//------------------------------------------------------------------------------
STDMETHODIMP IEHttpRequest::QueryService(REFGUID guidService, REFIID riid,
                                         void** ppvObject) {
  ATLASSERT(ppvObject != NULL);
  if (ppvObject == NULL)
    return E_POINTER;
  *ppvObject = NULL;

  // Our HttpHandler (see http_handler_ie.cc h) provides a mechanism to
  // bypass the LocalServer that involves querying for a particular service
  // id that does not exists. Here we detect that query and set our handler
  // in bypass mode.
  if (InlineIsEqualGUID(guidService, HttpHandler::SID_QueryBypassCache) &&
      InlineIsEqualGUID(riid, HttpHandler::IID_QueryBypassCache)) {
    if (ShouldBypassLocalServer()) {
      HttpHandler::SetBypassCache();
    }
    return E_NOINTERFACE;
  }

  // Our superclass will return our IHttpNegotiate interface pointer for us
  return IServiceProviderImpl<IEHttpRequest>::QueryService(guidService, riid,
                                                           ppvObject);
}

//------------------------------------------------------------------------------
// IBindStatusCallback::OnStartBinding
// This method is called by URLMON once per bind operation, even if the bind
// involves a chain of redirects, its only called once at the start.
// Note: binding->Abort() should not be called within this callback, instead
// return E_FAIL to cancel the bind process
//------------------------------------------------------------------------------
STDMETHODIMP IEHttpRequest::OnStartBinding(DWORD reserved, IBinding *binding) {
  LOG16((L"IEHttpRequest::OnStartBinding\n"));
  if (!binding) {
    return E_POINTER;
  }
  assert(!binding_);
  binding_ = binding;
  SetReadyState(HttpRequest::SENT);
  return S_OK;
}

// IBindStatusCallback
STDMETHODIMP IEHttpRequest::GetPriority(LONG *priority) {
  if (!priority)
    return E_POINTER;
  *priority = THREAD_PRIORITY_NORMAL;
  return S_OK;
}

// IBindStatusCallback
STDMETHODIMP IEHttpRequest::OnLowResource(DWORD reserved) {
  return S_OK;
}

//------------------------------------------------------------------------------
// IBindStatusCallback::OnProgress
// Implemented to receive redirect notifications
//------------------------------------------------------------------------------
STDMETHODIMP IEHttpRequest::OnProgress(ULONG progress, ULONG progress_max,
                                       ULONG status_code, LPCWSTR status_text) {
#ifdef DEBUG
  LOG16((L"IEHttpRequest::OnProgress(%s (%d), %s)\n",
         GetBindStatusLabel(status_code), status_code,
         status_text ? status_text : L"NULL"));
#endif
  if (status_code == BINDSTATUS_REDIRECTING) {
    return OnRedirect(status_text);
  } else if (status_code == BINDSTATUS_BEGINDOWNLOADDATA &&
             IsFileGet()) {
    // In case of 'file:' URLs, URLMON does not go via IHTTPNegotiate
    // but rather reads the file and uses IBindStatusCallback to report
    // progress. So we need to initialize status and data buffer to be
    // ready for IBSC::OnDataAvailable().
    response_body_.reset(new ByteStore);
    response_payload_.status_code = HTTPResponse::RC_REQUEST_OK;
  }
  return S_OK;
}

//------------------------------------------------------------------------------
// OnRedirect
// Depending on whether or not we're set up to follow redirects, this
// method either aborts the current bind operation, or remembers the location
// of the redirect and allows it to continue.
//------------------------------------------------------------------------------
HRESULT IEHttpRequest::OnRedirect(const char16 *redirect_url) {
  LOG16((L"IEHttpRequest::OnRedirect( %s )\n", redirect_url));

  bool follow = false;
  switch (redirect_behavior_) {
    case FOLLOW_ALL:
      follow = true;
      break;

    case FOLLOW_NONE:
      follow = false;
      break;

    case FOLLOW_WITHIN_ORIGIN:
      follow = origin_.IsSameOriginAsUrl(redirect_url);
      break;
  }

  if (!follow) {
    if (!binding_)
      return E_UNEXPECTED;
    // When we're not supposed to follow redirects, we abort the request when
    // a redirect is reported to us. This causes the bind to fail w/o ever
    // having seen the actual response data. Our HttpRequest interface dictates
    // that we return a valid reponse payload containing the redirect in this
    // case. Here we synthesize a valid redirect response for that purpose.
    // TODO(michaeln): we 'should' get the actual response data
    response_payload_.SynthesizeHttpRedirect(NULL, redirect_url);
    response_body_.reset(new ByteStore);
    has_synthesized_response_payload_ = true;
    ignore_stopbinding_error_ = true;
    return E_ABORT;
  } else {
    was_redirected_ = true;
    redirect_url_ = redirect_url;
  }
  return S_OK;
}


//------------------------------------------------------------------------------
// IBindStatusCallback::OnStopBinding
// This is called once per bind operation in both success and failure cases.
//------------------------------------------------------------------------------
STDMETHODIMP IEHttpRequest::OnStopBinding(HRESULT hresult, LPCWSTR error_text) {
  LOG16((L"IEHttpRequest::OnStopBinding - %d, %s\n",
             hresult, error_text ? error_text : L"null"));
  binding_.Release();
  bind_ctx_.Release();
  url_moniker_.Release();
  if (!ignore_stopbinding_error_) {
    was_aborted_ |= FAILED(hresult);
  }
  SetReadyState(HttpRequest::COMPLETE);
  return S_OK;
}

//------------------------------------------------------------------------------
// IBindStatusCallback::GetBindInfo
// Called by URLMON to determine how the 'bind' should be conducted
//------------------------------------------------------------------------------
STDMETHODIMP IEHttpRequest::GetBindInfo(DWORD *flags, BINDINFO *info) {
  LOG16((L"IEHttpRequest::GetBindInfo\n"));
  if (!info || !flags)
    return E_POINTER;
  if (!info->cbSize)
    return E_INVALIDARG;

  // When a POST results in a redirect, we GET the new url. The POSTMON
  // sample app does this; although, using IE7 I have not seen a second
  // call to GetBindInfo in this case. BeginningTransaction can get called
  // twice in specific circumstances (see that method for comments).
  // TODO(michaeln): run this in IE6 and see what happens?
  int bind_verb = bind_verb_;
  bool is_post_or_put = IsPostOrPut();
  if (is_post_or_put && was_redirected_) {
    bind_verb = BINDVERB_GET;
    is_post_or_put = false;
  }

  *flags = 0;

  // We use the pull-data model as push is unreliable.
  *flags |= BINDF_FROMURLMON | BINDF_PULLDATA | BINDF_NO_UI;

  if (async_) {
    *flags |= BINDF_ASYNCHRONOUS | BINDF_ASYNCSTORAGE;
  }

  if (ShouldBypassBrowserCache() || is_post_or_put) {
    // Setup bind flags such that we send a request all the way through
    // to the server, bypassing caching proxies between here and there;
    // and such that we don't write the response to the browser's cache.
    *flags |= BINDF_GETNEWESTVERSION | BINDF_NOWRITECACHE |
              BINDF_PRAGMA_NO_CACHE;
  }

  int save_size = static_cast<int>(info->cbSize);
  memset(info, 0, save_size);
  info->cbSize = save_size;
  info->dwBindVerb = bind_verb;
  if (bind_verb == BINDVERB_CUSTOM) {
    info->szCustomVerb = static_cast<LPWSTR>
        (CoTaskMemAlloc((method_.size() + 1) * sizeof(char16)));
    wcscpy(info->szCustomVerb, method_.c_str());
  }

  CComPtr<IStream> data_stream;
  int64 data_length;
  if (is_post_or_put && post_data_.get()) {
    CComObject<BlobStream> *blob_stream = NULL;
    HRESULT hr = CComObject<BlobStream>::CreateInstance(&blob_stream);
    if (FAILED(hr))
      return hr;
    blob_stream->Initialize(post_data_.get(), 0);
    data_stream = blob_stream;
    data_length = post_data_->Length();
  }

  if (data_stream) {
    CComObject<ProgressInputStream> *stream = NULL;
    HRESULT hr = CComObject<ProgressInputStream>::CreateInstance(&stream);
    if (FAILED(hr)) {
      return hr;
    }
    stream->Initialize(this, data_stream);

    info->stgmedData.tymed = TYMED_ISTREAM;
    info->stgmedData.pstm = static_cast<IStream*>(stream);

    post_data_stream_ = stream;
  }

  if (cookie_behavior_ == DO_NOT_SEND_BROWSER_COOKIES) {
    // Undocumented voodoo to pass options through to WinInet.
    //
    // The MSDN says that the dwOptions and dwOptionsFlags fields of the
    // BINDINFO parameter are reserved and should be set to NULL (see 
    // http://msdn.microsoft.com/en-us/library/ms774966(VS.85).aspx). However,
    // it seems that these fields can be used to pass options to WinInet (see
    // http://groups.google.com/group/csexwb/browse_thread/thread/d1f17a391dd323a6
    // and
    // http://groups.google.com/group/microsoft.public.inetsdk.programming.urlmonikers/browse_thread/thread/c9f21e1fc92f8e4d).
    //
    // See http://msdn.microsoft.com/en-us/library/ms775132(VS.85).aspx for the
    // possible values of the dwOptions field and
    // http://msdn.microsoft.com/en-us/library/aa384233(VS.85).aspx for the
    // flags that can be pased to WinInet in the dwOptionsFlags field.
    info->dwOptions |= BINDINFO_OPTIONS_WININETFLAG;
    info->dwOptionsFlags |= INTERNET_FLAG_NO_COOKIES; 
  }

  return S_OK;
}

//------------------------------------------------------------------------------
// IBindStatusCallback::OnDataAvailable
// Called by URLMON to inform us of data being available for reading
//------------------------------------------------------------------------------
namespace {

class Writer : public ByteStore::Writer {
 public:
  explicit Writer(STGMEDIUM *stgmed) : stgmed_(stgmed), hr_(S_OK) {
  }
  virtual int64 WriteToBuffer(uint8 *buffer, int64 max_length) {
    // Force quit if last operation failed or indicated that we should be done.
    if (Finished()) return 0;
    DWORD amount_read = 0;
    hr_ = stgmed_->pstm->Read(buffer, static_cast<size_t>(max_length),
                              &amount_read);
    assert(amount_read >= 0);
    return static_cast<int64>(amount_read);
  }
  bool Finished() const {
    return (hr_ == E_PENDING || hr_ == S_FALSE || FAILED(hr_));
  }
  HRESULT GetResult() const { return hr_; }
 private:
  STGMEDIUM *stgmed_;
  HRESULT hr_;
};

}  // namespace

STDMETHODIMP IEHttpRequest::OnDataAvailable(
    DWORD flags,
    DWORD unreliable_stream_size,  // With IE6, this value is not reliable
    FORMATETC *formatetc,
    STGMEDIUM *stgmed) {
  LOG16((L"IEHttpRequest::OnDataAvailable( 0x%x, %d )\n",
         flags, unreliable_stream_size));
  HRESULT hr = S_OK;

  if (!stgmed || !formatetc) {
    Abort();
    return E_POINTER;
  }
  if ((stgmed->tymed != TYMED_ISTREAM) || !stgmed->pstm) {
    Abort();
    return E_UNEXPECTED;
  }

  // Be careful not to overwrite a synthesized redirect response
  if (has_synthesized_response_payload_) {
    do {
      // We don't expect to get here. If we do for some reason, just read
      // data and drop it on the floor, otherwise the bind will stall
      uint8 buf[kReadAheadAmount];
      DWORD amount_read = 0;
      hr = stgmed->pstm->Read(buf, kReadAheadAmount, &amount_read);
    } while (!(hr == E_PENDING || hr == S_FALSE) && SUCCEEDED(hr));
    return hr;
  }

  if (!response_body_.get()) {
    assert(response_body_.get());
    Abort();
    return E_UNEXPECTED;
  }

  if (flags & BSCF_FIRSTDATANOTIFICATION) {
    assert(response_body_->Length() == 0);
  }

  bool is_new_data_available = false;

  // We use the data-pull model as the push model doesn't work
  // in some circumstances. In the pull model we have to read
  // beyond the end of what's currently available to encourage
  // the stream to read from the wire, otherwise the bind will stall
  // http://msdn2.microsoft.com/en-us/library/aa380034.aspx
  Writer writer(stgmed);
  do {
    int64 added = response_body_->AddDataDirect(&writer, kReadAheadAmount);
    is_new_data_available |= (added > 0);
  } while (!writer.Finished());

  if (is_new_data_available && listener_ && listener_data_available_enabled_) {
    listener_->DataAvailable(this, response_body_->Length());
  }

  return writer.GetResult();
}

//------------------------------------------------------------------------------
// IBindStatusCallback::OnObjectAvailable
// Since we call BindToStorage rather than BindToObject, we should never
// get here.
//------------------------------------------------------------------------------
STDMETHODIMP IEHttpRequest::OnObjectAvailable(REFIID riid, IUnknown *punk) {
  assert(false);
  return E_UNEXPECTED;
}

//------------------------------------------------------------------------------
// IHttpNegotiate::BeginningTransaction
// Called by URLMON to determine HTTP specific infomation about how to
// conduct the 'bind'. This is where we inform URLMON of the additional
// headers we would like sent with the HTTP request.
//------------------------------------------------------------------------------
STDMETHODIMP IEHttpRequest::BeginningTransaction(LPCWSTR url,
                                                 LPCWSTR headers,
                                                 DWORD reserved,
                                                 LPWSTR *additional_headers) {
  LOG16((L"IEHttpRequest::BeginningTransaction\n"));
  // In the case of a POST with a body which results in a redirect, this
  // method is called more than once. We don't set the additional headers
  // in this case. Those headers include content-length and all user specified
  // headers set by our SetRequestHeader method. The addition of the content
  // length header in this case would result in a malformed request being
  // sent over the network.
  if (!was_redirected_ && !additional_headers_.empty()) {
    *additional_headers = static_cast<LPWSTR>
        (CoTaskMemAlloc((additional_headers_.size() + 1) * sizeof(char16)));
    if (!(*additional_headers)) {
      Abort();
      return E_OUTOFMEMORY;
    }
    wcscpy(*additional_headers, additional_headers_.c_str());
  }
  return S_OK;
}

//------------------------------------------------------------------------------
// IHttpNegotiate::OnResponse
// Called by URLMON when a response is received with the caveat that redirect
// responses are not seen here, only the last response at the end of a chain.
//------------------------------------------------------------------------------
STDMETHODIMP IEHttpRequest::OnResponse(DWORD status_code,
                                       LPCWSTR response_headers,
                                       LPCWSTR request_headers,
                                       LPWSTR *additional_request_headers) {
  LOG16((L"IEHttpRequest::OnResponse (%d)\n", status_code));
  // Be careful not to overwrite a redirect response synthesized in OnRedirect
  if (has_synthesized_response_payload_) {
    return E_ABORT;
  }

  response_payload_.status_code = status_code;

  // 'response_headers' contains the status line followed by the headers,
  // we split them apart at the CRLF that seperates them
  const char16 *crlf = wcsstr(response_headers, HttpConstants::kCrLf);
  if (!crlf) {
    assert(false);
    Abort();
    return E_UNEXPECTED;
  }
  response_payload_.status_line.assign(response_headers,
                                       crlf - response_headers);
  response_payload_.headers = (crlf + 2);  // skip over the LF
  response_body_.reset(new ByteStore);

  // We only gather the body for good 200 OK responses
  if (status_code == HttpConstants::HTTP_OK) {
    // Allocate a data buffer based on the content-length header.
    // If this isn't sufficent large, it will be resized as needed in
    // our OnDataAvailable method.
    int content_length = 0;
    std::string16 header_value;
    response_payload_.GetHeader(HttpConstants::kContentLengthHeader,
                                &header_value);
    if (!header_value.empty()) {
      content_length = _wtoi(header_value.c_str());
      if (content_length < 0)
        content_length = 0;
    }
    response_body_->Reserve(content_length + kReadAheadAmount);
  }

  SetReadyState(HttpRequest::INTERACTIVE);

  // According to http://msdn2.microsoft.com/en-us/library/ms775055.aspx
  // Returning S_OK when the response_code indicates an error implies resending
  // the request with additional_request_headers appended. Presumably to
  // negotiate challenge/response type of authentication with the server.
  // We don't want to resend the request for this or any other purpose.
  // Additionally, on some systems returning S_OK for 304 responses results
  // in an undesireable 60 second delay prior to OnStopBinding happening,
  // so we return E_ABORT to avoid that delay.
  if (status_code == HttpConstants::HTTP_OK) {
    return S_OK;
  } else {
    ignore_stopbinding_error_ = true;
    return E_ABORT;
  }
}
