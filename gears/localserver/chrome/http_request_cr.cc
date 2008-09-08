// Copyright 2008, Google Inc.
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

#include <limits>
#include <vector>

#include "gears/localserver/chrome/http_request_cr.h"

#include "gears/base/chrome/browsing_context_cr.h"
#include "gears/base/chrome/module_cr.h"
#include "gears/base/common/async_router.h"
#include "gears/base/common/atomic_ops.h"
#include "gears/base/common/byte_store.h"
#include "gears/base/common/http_utils.h"
#include "gears/base/common/string_utils.h"
#include "gears/base/common/url_utils.h"
#include "gears/blob/blob_interface.h"
#include "gears/blob/buffer_blob.h"
#include "gears/localserver/common/safe_http_request.h"


static const int kReadAheadAmount = 32 * 1024;


//------------------------------------------------------------------------------
// Create
//------------------------------------------------------------------------------
// static
bool HttpRequest::Create(scoped_refptr<HttpRequest>* request) {
  if (CP::IsPluginThread()) {
    request->reset(new CRHttpRequest);
    return true;
  } else {
    return HttpRequest::CreateSafeRequest(request);
  }
}

// static
bool HttpRequest::CreateSafeRequest(scoped_refptr<HttpRequest>* request) {
  request->reset(new SafeHttpRequest(CP::plugin_thread_id()));
  return true;
}

//------------------------------------------------------------------------------
// Construction, destruction and refcounting
//------------------------------------------------------------------------------

CRHttpRequest::CRHttpRequest()
    : caching_behavior_(USE_ALL_CACHES), redirect_behavior_(FOLLOW_ALL),
      was_redirected_(false), was_aborted_(false),
      listener_(NULL), listener_data_available_enabled_(false),
      ready_state_(UNINITIALIZED), has_synthesized_response_payload_(false),
      browsing_context_(0), async_(false), request_(NULL), content_length_(0) {
}

CRHttpRequest::~CRHttpRequest() {
  assert(!request_);
}

void CRHttpRequest::Ref() {
  refcount_.Ref();
}

void CRHttpRequest::Unref() {
  if (refcount_.Unref())
    delete this;
}

bool CRHttpRequest::CreateCPRequest(const char *method, const char *url) {
  assert(CP::IsPluginThread());
  assert(!request_);
  int rv = g_cpbrowser_funcs.create_request(CP::cpid(), browsing_context_,
                                            method, url, &request_);
  if (rv != CPERR_SUCCESS || !request_)
    return false;
  assert(request_);
  request_->pdata = this;  // see FromCPRequest()
  Ref();  // Keep ourselves referenced until we destroy the CPRequest
  return true;
}

void CRHttpRequest::DestroyCPRequest() {
  assert(CP::IsPluginThread());
  assert(request_);
  request_->pdata = NULL;
  g_cprequest_funcs.end_request(request_, CPERR_CANCELLED);
  request_ = NULL;
  Unref();
}

//------------------------------------------------------------------------------
// GetReadyState
//------------------------------------------------------------------------------
bool CRHttpRequest::GetReadyState(ReadyState *state) {
  *state = ready_state_;
  return true;
}

//------------------------------------------------------------------------------
// GetResponseBody
//------------------------------------------------------------------------------
bool CRHttpRequest::GetResponseBody(scoped_refptr<BlobInterface> *blob) {
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
bool CRHttpRequest::GetStatus(int *status) {
  if (!IsInteractiveOrComplete() || was_aborted_)
    return false;
  *status = response_payload_.status_code;
  return true;
}

//------------------------------------------------------------------------------
// GetStatusText
// TODO(michaeln): remove this method from the interface, prefer getStatusLine
//------------------------------------------------------------------------------
bool CRHttpRequest::GetStatusText(std::string16 *status_text) {
  if (!IsInteractiveOrComplete() || was_aborted_)
    return false;
  return ParseHttpStatusLine(response_payload_.status_line,
                             NULL, NULL, status_text);
}

//------------------------------------------------------------------------------
// GetStatusLine
//------------------------------------------------------------------------------
bool CRHttpRequest::GetStatusLine(std::string16 *status_line) {
  if (!IsInteractiveOrComplete() || was_aborted_)
    return false;
  *status_line = response_payload_.status_line;
  return true;
}

//------------------------------------------------------------------------------
// GetAllResponseHeaders
//------------------------------------------------------------------------------
bool CRHttpRequest::GetAllResponseHeaders(std::string16 *headers) {
  if (!IsInteractiveOrComplete() || was_aborted_)
    return false;
  headers->assign(response_payload_.headers);
  return true;
}

//------------------------------------------------------------------------------
// GetResponseCharset
//------------------------------------------------------------------------------
std::string16 CRHttpRequest::GetResponseCharset() {
  // TODO(bgarcia): If the document sets the Content-Type charset, return
  // that value.
  // Also need to update blob/blob_utils.cc.
  return std::string16();
}

//------------------------------------------------------------------------------
// GetResponseHeader
//------------------------------------------------------------------------------
bool CRHttpRequest::GetResponseHeader(const char16* name,
                                      std::string16 *value) {
  if (!IsInteractiveOrComplete() || was_aborted_)
    return false;
  return response_payload_.GetHeader(name, value);
}

//------------------------------------------------------------------------------
// Open
//------------------------------------------------------------------------------
bool CRHttpRequest::Open(const char16 *method, const char16* url, bool async,
                         BrowsingContext *browsing_context) {
  assert(!IsRelativeUrl(url));
  if (!IsUninitialized())
    return false;

  // sync requests are only supported in the plugin process
  assert(async || CP::IsPluginProcess());

  async_ = async;
  url_ = url;
  if (!origin_.InitFromUrl(url)) {
    return false;
  }
  method_ = method;
  UpperString(method_);
  SetReadyState(HttpRequest::OPEN);

  CRBrowsingContext *cr_context =
      static_cast<CRBrowsingContext*>(browsing_context);
  if (cr_context)
    browsing_context_ = cr_context->context;
  else
    LOG16((STRING16(L"Warning: CRHttpRequest has no context\n")));
  return true;
}

//------------------------------------------------------------------------------
// SetRequestHeader
// Here we gather additional request headers to be sent. They are plumbed
// into URLMON in our IHttpNegotiate::BeginningTransaction method.
//------------------------------------------------------------------------------
bool CRHttpRequest::SetRequestHeader(const char16* name, const char16* value) {
  if (!IsOpen())
    return false;
  additional_headers_ += name;
  additional_headers_ += L": ";
  additional_headers_ += value;
  additional_headers_ += HttpConstants::kCrLf;
  return true;
}

bool CRHttpRequest::WasRedirected() {
  return IsInteractiveOrComplete() && !was_aborted_ && was_redirected_;
}

bool CRHttpRequest::GetFinalUrl(std::string16 *full_url) {
  if (!IsInteractiveOrComplete() || was_aborted_)
    return false;

  if (WasRedirected())
    *full_url = redirect_url_;
  else
    *full_url = url_;
  return true;
}

bool CRHttpRequest::GetInitialUrl(std::string16 *full_url) {
  *full_url = url_;  // may be empty if request has not occurred
  return true;
}


//------------------------------------------------------------------------------
// Send
//------------------------------------------------------------------------------

bool CRHttpRequest::Send(BlobInterface *blob) {
  if (!IsOpen()) return false;
  if (IsPostOrPut()) {
    post_data_ = blob ? blob : new EmptyBlob;
  } else if (blob) {
    return false;
  }

  assert(CP::IsPluginThread());
  assert(!request_);
  LOG16((L"CRHttpRequest::SendImpl(%s %s)\n", method_.c_str(), url_.c_str()));

  if (was_aborted_)
    return false;  // already aborted, no need to send

  std::string method_utf8, url_utf8, additional_headers_utf8;
  if (!String16ToUTF8(method_.c_str(), &method_utf8))
    return false;

  if (!String16ToUTF8(url_.c_str(), &url_utf8))
    return false;

  if (!String16ToUTF8(additional_headers_.c_str(), &additional_headers_utf8))
    return false;

  if (!CreateCPRequest(method_utf8.c_str(), url_utf8.c_str()))
    return false;

  // Chop off the last \r\n - the CPRequest API doesn't like it.
  size_t crlf = additional_headers_utf8.size() - 2;
  if (additional_headers_utf8.find("\r\n", crlf) != std::string::npos) {
    additional_headers_utf8.erase(crlf, 2);
  }
  g_cprequest_funcs.set_extra_request_headers(
      request_, additional_headers_utf8.c_str());

  // Set the load flags to bypass the cache and/or Gears interception.
  uint32 load_flags = 0;
  if (!async_) {
    // Setting this flag will force start_request and read to block until we
    // have a result, so we should never get back CPERR_IO_PENDING.
    load_flags |= CPREQUESTLOAD_SYNCHRONOUS;
  }
  if (ShouldBypassLocalServer()) {
    if (CP::offline_debug_mode()) {
      // We are trying to simulate offline.  Without interception, this request
      // would certainly fail while offline.
      Abort();
      return false;
    }
    load_flags |= CPREQUESTLOAD_DISABLE_INTERCEPT;
  }
  if (ShouldBypassBrowserCache() || IsPostOrPut()) {
    load_flags |= CPREQUESTLOAD_BYPASS_CACHE | CPREQUESTLOAD_DISABLE_CACHE;
  }
  g_cprequest_funcs.set_request_load_flags(request_, load_flags);

  // Set our POST data.
  if (post_data_.get()) {
    std::vector<DataElement> data_elements;
    if (!post_data_->GetDataElements(&data_elements)) {
      Abort();
      return false;
    }

    for (size_t i = 0; i < data_elements.size(); ++i) {
      switch (data_elements[i].type()) {
        case DataElement::TYPE_BYTES: {
          g_cprequest_funcs.append_data_to_upload(
              request_,
              reinterpret_cast<const char*>(data_elements[i].bytes()),
              data_elements[i].bytes_length());
          break;
        }
        case DataElement::TYPE_FILE: {
          std::string file_path_utf8;
          if (!String16ToUTF8(data_elements[i].file_path().c_str(),
                              &file_path_utf8)) {
            Abort();
            return false;
          }
          g_cprequest_funcs.append_file_to_upload(
              request_,
              file_path_utf8.c_str(),
              data_elements[i].file_range_offset(),
              data_elements[i].file_range_length());
          break;
        }
      }
    }
  }

  int rv = g_cprequest_funcs.start_request(request_);
  if (rv == CPERR_SUCCESS) {
    // The response was ready synchronously.
    OnStartCompleted(CPERR_SUCCESS);
    return true;
  } else if (rv != CPERR_IO_PENDING) {
    // An error occurred.
    Abort();
    return false;
  }

  // else, IO is pending, and we will be notified via OnStartCompleted when
  // it completes.
  assert(async_);

  SetReadyState(HttpRequest::SENT);
  return true;
}

//------------------------------------------------------------------------------
// Abort
//------------------------------------------------------------------------------
bool CRHttpRequest::Abort() {
  assert(CP::IsPluginThread());
  LOG16((L"CRHttpRequest::Abort(%s %s)\n", method_.c_str(), url_.c_str()));

  was_aborted_ = true;

  // guard against last reference being removed
  scoped_refptr<CRHttpRequest> reference(this);

  if (request_) {
    // request_ can be NULL if Abort() is called prior to Send()
    DestroyCPRequest();
  }

  SetReadyState(HttpRequest::COMPLETE);
  return true;
}

void CRHttpRequest::OnRequestComplete() {
  assert(CP::IsPluginThread());

  LOG16((L"CRHttpRequest::OnRequestComplete(%s %s)\n",
         method_.c_str(), url_.c_str()));

  // guard against last reference being removed
  scoped_refptr<CRHttpRequest> reference(this);
  DestroyCPRequest();
  SetReadyState(HttpRequest::COMPLETE);
}

//------------------------------------------------------------------------------
// SetOnReadyStateChange
//------------------------------------------------------------------------------
bool CRHttpRequest::SetListener(HttpListener *listener,
                                bool enable_data_available) {
  listener_ = listener;
  listener_data_available_enabled_ = enable_data_available;
  return true;
}

void CRHttpRequest::SetReadyState(ReadyState state) {
  if (ready_state_ >= state)
    return;

  ready_state_ = state;
  if (listener_) {
    listener_->ReadyStateChanged(this);
  }
}

void CRHttpRequest::NotifyDataAvailable() {
  if (IsComplete())
    return;

  if (listener_ && listener_data_available_enabled_) {
    listener_->DataAvailable(this, response_body_->Length());
  }
}

//------------------------------------------------------------------------------
// OnRedirect
// Depending on whether or not we're set up to follow redirects, this method
// either aborts the request, or remembers the location of the redirect and
// allows it to continue.
//------------------------------------------------------------------------------

void CRHttpRequest::OnReceivedRedirect(const std::string16& redirect_url) {
  assert(CP::IsPluginThread());
  LOG16((L"CRHttpRequest::OnRedirect(%s -> %s)\n",
         url_.c_str(), redirect_url.c_str()));

  if (was_aborted_)
    return;

  bool follow = false;
  switch (redirect_behavior_) {
    case FOLLOW_ALL:
      follow = true;
      break;

    case FOLLOW_NONE:
      follow = false;
      break;

    case FOLLOW_WITHIN_ORIGIN:
      follow = origin_.IsSameOriginAsUrl(redirect_url.c_str());
      break;
  }

  if (!follow) {
    // When we're not supposed to follow redirects, we end the request when
    // a redirect is reported to us. This causes the request to fail w/o ever
    // having seen the actual response data. Our HttpRequest interface dictates
    // that we return a valid response payload containing the redirect in this
    // case. Here we synthesize a valid redirect response for that purpose.
    // TODO(michaeln): we 'should' get the actual response data
    response_payload_.SynthesizeHttpRedirect(NULL, redirect_url.c_str());
    response_body_.reset(new ByteStore);
    has_synthesized_response_payload_ = true;
    OnRequestComplete();
  } else {
    was_redirected_ = true;
    redirect_url_ = redirect_url;
  }
}

//------------------------------------------------------------------------------
// OnStartCompleted
// Called by Chrome to inform us that all redirects have been followed and a
// response is available.  All headers are ready, and we can start reading the
// data.
//------------------------------------------------------------------------------

void CRHttpRequest::OnStartCompleted(CPError result) {
  LOG16((L"CRHttpRequest::OnStartCompleted(%d)\n", result));

  if (was_aborted_)
    return;

  // guard against last reference being removed
  scoped_refptr<CRHttpRequest> reference(this);

  // Be careful not to overwrite a synthesized redirect response
  if (has_synthesized_response_payload_) {
    return;
  }

  if (result != CPERR_SUCCESS) {
    Abort();
    return;
  }

  int status_code = 0, rv, size;
  std::string16 response_headers;

  // Get the status code
  rv = g_cprequest_funcs.get_response_info(
      request_, CPRESPONSEINFO_HTTP_STATUS,
      &status_code, sizeof(status_code));
  if (rv != CPERR_SUCCESS) {
    Abort();
    return;
  }

  // Get the response headers
  size = g_cprequest_funcs.get_response_info(
      request_, CPRESPONSEINFO_HTTP_RAW_HEADERS, NULL, 0);
  if (size < 0) {
    Abort();
    return;
  }
  std::string response_headers_utf8(size, 0);
  rv = g_cprequest_funcs.get_response_info(
      request_, CPRESPONSEINFO_HTTP_RAW_HEADERS,
      &response_headers_utf8[0], size);
  if (rv != CPERR_SUCCESS) {
    Abort();
    return;
  }

  // We get the headers back \0-delimited.  Convert them to \r\n-delimited.
  ReplaceAll(response_headers_utf8, std::string(1, '\0'), std::string("\r\n"));

  const char* body = response_headers_utf8.c_str();
  uint32 body_len = response_headers_utf8.size();
  HTTPHeaders parsed_headers;
  if (!HTTPUtils::ParseHTTPHeaders(&body, &body_len, &parsed_headers, true)) {
    Abort();
    return;
  }

  // Flatten the headers back out again.
  response_headers_utf8.clear();
  for (HTTPHeaders::const_iterator hdr = parsed_headers.begin();
       hdr != parsed_headers.end(); ++hdr) {
    if (hdr->second != NULL) {  // NULL means do not output
      response_headers_utf8 += hdr->first;
      response_headers_utf8 += ": ";
      response_headers_utf8 += hdr->second;
      response_headers_utf8 += HttpConstants::kCrLfAscii;
    }
  }
  response_headers_utf8 += HttpConstants::kCrLfAscii;  // blank line at the end

  response_payload_.status_code = status_code;

  // Convert the status line and headers to wide char.
  if (!UTF8ToString16(parsed_headers.firstline(),
                      &response_payload_.status_line)) {
    Abort();
    return;
  }

  if (!UTF8ToString16(response_headers_utf8.c_str(),
                      &response_payload_.headers)) {
    Abort();
    return;
  }

  response_body_.reset(new ByteStore);

  // We only gather the body for good 200 OK responses
  if (status_code != HttpConstants::HTTP_OK) {
    OnRequestComplete();
    return;
  }

  // Allocate a data buffer based on the content-length header.
  // If this isn't sufficiently large, it will be resized as needed.
  content_length_ = 0;
  const char* header_value =
      parsed_headers.GetHeader(HTTPHeaders::CONTENT_LENGTH);
  if (header_value) {
    content_length_ = atoi(header_value);
    if (content_length_ < 0)
      content_length_ = 0;
  }
  response_body_->Reserve(content_length_ + kReadAheadAmount);

  SetReadyState(HttpRequest::INTERACTIVE);

  // Start reading the body.
  StartReading();
}

//------------------------------------------------------------------------------
// StartReading
// Reads more data from the stream until we hit an error or EOF.
//------------------------------------------------------------------------------

namespace {

class Writer : public ByteStore::Writer {
 public:
  explicit Writer(CPRequest *request) : request_(request), rv_(0) {
  }
  virtual int64 WriteToBuffer(uint8 *buffer, int64 max_length) {
    rv_ = g_cprequest_funcs.read(request_, buffer,
                                 static_cast<uint32>(max_length));
    if (rv_ == CPERR_IO_PENDING) return ASYNC;
    if (rv_ < 0) return 0;
    return rv_;
  }
  int GetResult() const { return rv_; }
 private:
  CPRequest *request_;
  int rv_;
};

}  // namespace

void CRHttpRequest::StartReading() {
  assert(CP::IsPluginThread());

  if (was_aborted_)
    return;

  if (response_payload_.status_code != HttpConstants::HTTP_OK) {
    // We should have closed the request for a non-200 OK response.
    assert(false);
    OnRequestComplete();
    return;
  }

  bool is_new_data_available = false;
  Writer writer(request_);
  do {
    int64 added = response_body_->AddDataDirect(&writer, kReadAheadAmount);
    is_new_data_available |= (added > 0);
  } while (writer.GetResult() > 0);

  if (is_new_data_available) {
    NotifyDataAvailable();
  }

  if (writer.GetResult() != CPERR_IO_PENDING) {
    // Either an error occurred, or we hit EOF.
    OnReadCompleted(writer.GetResult());
  } else {
    assert(async_);
  }
}

//------------------------------------------------------------------------------
// OnReadCompleted
// Called by Chrome to inform us that an asynchronous read operation finished,
// and 'bytes_read' was written into the buffer we passed it.  0 indicates EOF,
// while a negative value indicates error.
//------------------------------------------------------------------------------

void CRHttpRequest::OnReadCompleted(int bytes_read) {
  assert(CP::IsPluginThread());

  if (was_aborted_)
    return;

  // guard against last reference being removed
  scoped_refptr<CRHttpRequest> reference(this);

  if (bytes_read > 0) {
    response_body_->AddDataDirectFinishAsync(bytes_read);
    NotifyDataAvailable();
    // Keep reading data until EOF or error.
    StartReading();
  } else if (bytes_read == 0) {
    // 0 means EOF
    response_body_->AddDataDirectFinishAsync(bytes_read);
    OnRequestComplete();
  } else {
    // An error occurred.
    Abort();
  }
}

//------------------------------------------------------------------------------
// Called by Chrome to inform us of upload progress
//------------------------------------------------------------------------------
void CRHttpRequest::OnUploadProgress(int64 position, int64 size) {
  assert(CP::IsPluginThread());

  if (was_aborted_)
    return;

  if (listener_) {
    listener_->UploadProgress(this, position, size);
  }
}
