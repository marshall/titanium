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

#ifdef WIN32
#include <windows.h>  // Needs to be included before nsIEventQueueService.h
#endif

#include <gecko_sdk/include/nsIURI.h>
#include <gecko_sdk/include/nsIUploadChannel.h>
#include <gecko_sdk/include/nsIIOService.h>
#include <gecko_sdk/include/nsIInputStream.h>
#include <gecko_sdk/include/nsIHttpHeaderVisitor.h>
#include <gecko_sdk/include/nsIHttpChannel.h>
#include <gecko_sdk/include/nsIObserver.h>
#include <gecko_sdk/include/nsIObserverService.h>
#include <gecko_internal/nsICachingChannel.h>
#include <gecko_internal/nsIEncodedChannel.h>
#if BROWSER_FF3
#include <gecko_internal/nsIThread.h>
#include <gecko_internal/nsThreadUtils.h>
#else
#include <gecko_internal/nsIEventQueueService.h>
#endif
#include <gecko_internal/nsIStringStream.h>
#include <gecko_internal/nsNetError.h>

#include "gears/localserver/firefox/http_request_ff.h"

#include "gears/base/common/leak_counter.h"
#include "gears/base/common/http_utils.h"
#include "gears/base/common/string_utils.h"
#include "gears/base/common/url_utils.h"
#include "gears/localserver/common/safe_http_request.h"
#include "gears/localserver/firefox/cache_intercept.h"
#include "gears/localserver/firefox/progress_input_stream.h"
#include "gears/localserver/firefox/ui_thread.h"

#include "gears/blob/blob_input_stream_ff.h"
#include "gears/blob/blob_interface.h"

// TODO(steveblock): Consider sharing these values with those in
// cache_intercept.cc.
static const char *kObserverServiceContractId =
    "@mozilla.org/observer-service;1";
static const char *kOnModifyRequestTopic = "http-on-modify-request";

NS_IMPL_THREADSAFE_ISUPPORTS5(FFHttpRequest,
                              nsIRequestObserver,
                              nsIStreamListener,
                              nsIChannelEventSink,
                              nsIInterfaceRequestor,
                              SpecialHttpRequestInterface)
#if BROWSER_FF3
NS_IMPL_THREADSAFE_ISUPPORTS5(GearsLoadGroup,
                              nsILoadGroup,
                              nsIDocShellTreeItem,
                              nsIDocShellTreeNode,
                              nsIRequest,
                              nsIInterfaceRequestor)
#endif

//------------------------------------------------------------------------------
// HttpRequestObserver
//------------------------------------------------------------------------------
class HttpRequestObserver : public nsIObserver {
 public:
  NS_DECL_ISUPPORTS

  HttpRequestObserver(nsIChannel *channel) : channel_(channel) {}

 private:
  // nsIObserver implementation
  NS_IMETHODIMP Observe(nsISupports *channel,
                        const char *topic,
                        const PRUnichar * /* data */) {
    // Check that this is for the right topic.
    assert(strcmp(topic, kOnModifyRequestTopic) == 0);

    // Note that we will observe this event for all HTTP requests while the
    // observer is registered. However, we only make changes to the header for
    // this HTTP request. See
    // http://developer.mozilla.org/En/Creating_Sandboxed_HTTP_Connections.
    if (channel_ == channel) {
      nsIHttpChannel *http_channel = nsnull;
      CallQueryInterface(channel_, &http_channel);
      assert(http_channel);
      nsresult rv = http_channel->SetRequestHeader(nsCString("Cookie"),
                                                   nsCString(""),
                                                   PR_FALSE);  // Replace header
      NS_ENSURE_SUCCESS(rv, false);
      // Note that we can't unregister the observer from this callback, so we do
      // so from the FFHttpRequest destructor.
    }
    return NS_OK;
  }

  nsIChannel *channel_;
};

NS_IMPL_THREADSAFE_ISUPPORTS1(HttpRequestObserver,
                              nsIObserver);

//------------------------------------------------------------------------------
// HttpRequest::Create
//------------------------------------------------------------------------------
bool HttpRequest::Create(scoped_refptr<HttpRequest>* request) {
  if (IsUiThread()) {
    request->reset(new FFHttpRequest);
    return true;
  } else {
    return HttpRequest::CreateSafeRequest(request);
  }
}

// static
bool HttpRequest::CreateSafeRequest(scoped_refptr<HttpRequest>* request) {
  request->reset(new SafeHttpRequest(GetUiThread()));
  return true;
}

//------------------------------------------------------------------------------
// Constructor / Destructor / Refcounting
//------------------------------------------------------------------------------
FFHttpRequest::FFHttpRequest()
  : ready_state_(UNINITIALIZED),
    async_(false),
    post_data_stream_attached_(false),
    caching_behavior_(USE_ALL_CACHES),
    redirect_behavior_(FOLLOW_ALL),
    cookie_behavior_(SEND_BROWSER_COOKIES),
    observer_(NULL),
    was_sent_(false),
    was_aborted_(false),
    was_redirected_(false),
    listener_(NULL),
    listener_data_available_enabled_(false) {
  LEAK_COUNTER_INCREMENT(FFHttpRequest);
}

FFHttpRequest::~FFHttpRequest() {
  LEAK_COUNTER_DECREMENT(FFHttpRequest);
  nsCOMPtr<nsIObserverService> observer_service(
      do_GetService(kObserverServiceContractId));
  if (observer_service && observer_) {
    observer_service->RemoveObserver(observer_, kOnModifyRequestTopic);
  }
  if (post_data_stream_attached_) {
    post_data_stream_->OnFFHttpRequestDestroyed(this);
  }
}

void FFHttpRequest::Ref() {
  AddRef();
}

void FFHttpRequest::Unref() {
  Release();
}

//------------------------------------------------------------------------------
// GetReadyState
//------------------------------------------------------------------------------
bool FFHttpRequest::GetReadyState(ReadyState *state) {
  *state = ready_state_;
  return true;
}

//------------------------------------------------------------------------------
// GetResponseBody
//------------------------------------------------------------------------------
bool FFHttpRequest::GetResponseBody(scoped_refptr<BlobInterface> *blob) {
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
bool FFHttpRequest::GetStatus(int *status) {
  NS_ENSURE_TRUE(IsInteractiveOrComplete() && !was_aborted_, false);
  if (IsFileGet()) {
    // If it was a 'file://' get request, and we are interactive or complete -
    // this means the file is present. Equivalent to '200 OK'.
    *status = HTTPResponse::RC_REQUEST_OK;
    return true;
  }
  nsCOMPtr<nsIHttpChannel> http_channel = GetCurrentHttpChannel();
  if (!http_channel) {
    return false;
  }
  PRUint32 pr_status;
  nsresult rv = http_channel->GetResponseStatus(&pr_status);
  NS_ENSURE_SUCCESS(rv, false);
  *status = static_cast<int>(pr_status);
  return true;
}

//------------------------------------------------------------------------------
// GetStatusText
//------------------------------------------------------------------------------
bool FFHttpRequest::GetStatusText(std::string16 *status_text) {
  NS_ENSURE_TRUE(IsInteractiveOrComplete() && !was_aborted_, false);
  if (IsFileGet()) {
    // If it was a 'file://' get request, and we are interactive or complete -
    // this means the file is present. Equivalent to '200 OK'.
    *status_text = STRING16(L"OK");
    return true;
  }
  nsCOMPtr<nsIHttpChannel> http_channel = GetCurrentHttpChannel();
  if (!http_channel) {
    return false;
  }
  nsCString status_str;
  nsresult rv = http_channel->GetResponseStatusText(status_str);
  NS_ENSURE_SUCCESS(rv, false);
  return UTF8ToString16(status_str.get(), status_str.Length(), status_text);
}

//------------------------------------------------------------------------------
// GetStatusLine
//------------------------------------------------------------------------------
bool FFHttpRequest::GetStatusLine(std::string16 *status_line) {
  NS_ENSURE_TRUE(IsInteractiveOrComplete() && !was_aborted_, false);
  if (IsFileGet()) {
    // If it was a 'file://' get request, and we are interactive or complete -
    // this means the file is present. Equivalent to '200 OK'.
    *status_line = STRING16(L"HTTP/1.1 200 OK");
    return true;
  }
  // TODO(michaeln): get the actual status line instead of synthesizing one
  nsCOMPtr<nsIHttpChannel> http_channel = GetCurrentHttpChannel();
  if (!http_channel) {
    return false;
  }

  nsCString status_text;
  nsresult rv = http_channel->GetResponseStatusText(status_text);
  NS_ENSURE_SUCCESS(rv, false);

  int status_code;
  if (!GetStatus(&status_code))
    return false;

  nsCString status_line8;
  status_line8.Assign("HTTP/1.1 ");
  status_line8.Append(IntegerToString(status_code).c_str());
  status_line8.Append(" ");
  status_line8.Append(status_text);

  return UTF8ToString16(status_line8.get(), status_line8.Length(), status_line);
}

//------------------------------------------------------------------------------
// Open
//------------------------------------------------------------------------------
bool FFHttpRequest::Open(const char16 *method, const char16 *url, bool async,
                         BrowsingContext *browsing_context) {
  assert(!IsRelativeUrl(url));
  // TODO(michaeln): Add some of the sanity checks the IE implementation has.

  // This class can only be used on the main UI thread
  if (!IsUiThread())
    return false;

  NS_ENSURE_TRUE(IsUninitialized(), false);
  async_ = async;

  url_ = url;
  if (!origin_.InitFromUrl(url)) {
    return false;
  }

  nsCOMPtr<nsIIOService> ios =
    do_GetService("@mozilla.org/network/io-service;1");
  NS_ENSURE_TRUE(ios, false);

  std::string url_utf8;
  if (!String16ToUTF8(url, &url_utf8)) {
    return false;
  }
  nsCString url_str(url_utf8.c_str());
  nsresult rv = ios->NewChannel(url_str, nsnull, nsnull,
                                getter_AddRefs(channel_));
  NS_ENSURE_SUCCESS(rv, false);
  NS_ENSURE_TRUE(channel_, false);

  nsLoadFlags load_flags = 0;
#if BROWSER_FF3
  if (!load_group_.get()) {
    load_group_.reset(new GearsLoadGroup);
  }
  channel_->SetLoadGroup(load_group_.get());
  load_flags |= nsIChannel::LOAD_DOCUMENT_URI;
#endif

  method_ = method;
  UpperString(method_);

  // The subsequent OpenAsync will test for existence of the file.
  if (IsFileGet()) {
    SetReadyState(HttpRequest::OPEN);
    return true;
  }

  std::string method_utf8;
  if (!String16ToUTF8(method_.c_str(), &method_utf8)) {
    return false;
  }
  nsCString method_str(method_utf8.c_str());
  nsCOMPtr<nsIHttpChannel> http_channel = GetCurrentHttpChannel();
  NS_ENSURE_TRUE(http_channel, false);
  rv = http_channel->SetRequestMethod(method_str);
  NS_ENSURE_SUCCESS(rv, false);

  if (ShouldBypassBrowserCache() || IsPostOrPut()) {
    load_flags |= nsIRequest::LOAD_BYPASS_CACHE | nsIRequest::INHIBIT_CACHING;
  } else if (!async_) {
    load_flags |= nsICachingChannel::LOAD_BYPASS_LOCAL_CACHE_IF_BUSY;
  }
  rv = channel_->SetLoadFlags(load_flags);
  NS_ENSURE_SUCCESS(rv, false);

  SetReadyState(HttpRequest::OPEN);
  return true;
}

//------------------------------------------------------------------------------
// SetRequestHeader
//------------------------------------------------------------------------------
bool FFHttpRequest::SetRequestHeader(const char16* name, const char16* value) {
  if (was_sent_)
    return false;
  if (IsFileGet())
    return true;
  nsCOMPtr<nsIHttpChannel> http_channel = GetCurrentHttpChannel();
  NS_ENSURE_TRUE(http_channel, false);

  std::string name_utf8;
  if (!String16ToUTF8(name, &name_utf8)) {
    return false;
  }
  std::string value_utf8;
  if (!String16ToUTF8(value, &value_utf8)) {
    return false;
  }
  nsCString name_str(name_utf8.c_str());
  nsCString value_str(value_utf8.c_str());
  nsresult rv = http_channel->SetRequestHeader(name_str, value_str, PR_FALSE);
  NS_ENSURE_SUCCESS(rv, false);
  return true;
}

//------------------------------------------------------------------------------
// RedirectBehavior
//------------------------------------------------------------------------------

bool FFHttpRequest::WasRedirected() {
  return IsInteractiveOrComplete() && !was_aborted_ && was_redirected_;
}

bool FFHttpRequest::GetFinalUrl(std::string16 *full_url) {
  if (!IsInteractiveOrComplete() || was_aborted_)
    return false;

  if (WasRedirected())
    *full_url = redirect_url_;
  else
    *full_url = url_;
  return true;
}

bool FFHttpRequest::GetInitialUrl(std::string16 *full_url) {
  *full_url = url_;  // may be empty if request has not occurred
  return true;
}

//------------------------------------------------------------------------------
// Send
//------------------------------------------------------------------------------

bool FFHttpRequest::Send(BlobInterface *blob) {
  if (!IsOpen()) return false;
  if (IsPostOrPut()) {
    nsCOMPtr<BlobInputStream> blob_stream(
        new BlobInputStream(blob ? blob : new EmptyBlob));
    assert(!post_data_stream_attached_);
    post_data_stream_ = new ProgressInputStream(this, blob_stream,
                                                blob ? blob->Length() : 0);
    post_data_stream_attached_ = true;
  } else if (blob) {
    return false;
  }

  NS_ENSURE_TRUE(channel_ && !was_sent_, false);
  nsresult rv = NS_OK;
  was_sent_ = true;

  if (cookie_behavior_ == DO_NOT_SEND_BROWSER_COOKIES) {
    // Register as an observer for the 'http-on-modify-request' topic. We then
    // intercept the request just before it's sent to remove browser cookies.
    nsCOMPtr<nsIObserverService> observer_service(
        do_GetService(kObserverServiceContractId));
    if (!observer_service) {
      LOG(("FFHttpRequest::Send(): Could not get observer service.\n"));
      return false;
    }
    observer_ = new HttpRequestObserver(channel_);
    observer_service->AddObserver(observer_, kOnModifyRequestTopic, false);
  }

  if (!IsFileGet()) {
    nsCOMPtr<nsIHttpChannel> http_channel = GetCurrentHttpChannel();
    NS_ENSURE_TRUE(http_channel, false);

    if (post_data_stream_) {
      nsCOMPtr<nsIUploadChannel> upload_channel(do_QueryInterface(http_channel));
      NS_ENSURE_TRUE(upload_channel, false);

      nsCString method;
      http_channel->GetRequestMethod(method);

      nsCString content_type;
      if (NS_FAILED(http_channel->GetRequestHeader(
                        NS_LITERAL_CSTRING("Content-Type"), content_type)) ||
          content_type.IsEmpty()) {
        // If no content type was set by the client, we set it to text/plain.
        // Ideally, we should not be setting the content type here at all,
        // however not doing so changes the semantics of SetUploadStream such
        // that we would need to prefix the data in the stream with http headers.
        // To avoid doing that, we set it to something.
        content_type = NS_LITERAL_CSTRING("text/plain");
      }

      const int kGetLengthFromStream = -1;
      rv = upload_channel->SetUploadStream(post_data_stream_,
                                           content_type,
                                           kGetLengthFromStream);
      NS_ENSURE_SUCCESS(rv, false);

      // Reset the method to its original value because SetUploadStream has the
      // side effect of squashing the previously set value.
      http_channel->SetRequestMethod(method);
    }
  }
#if BROWSER_FF2
  nsCOMPtr<nsIEventQueueService> event_queue_service;
  nsCOMPtr<nsIEventQueue> modal_event_queue;

  if (!async_) {
    event_queue_service = do_GetService(NS_EVENTQUEUESERVICE_CONTRACTID, &rv);
    NS_ENSURE_SUCCESS(rv, false);

    rv = event_queue_service->PushThreadEventQueue(
                                  getter_AddRefs(modal_event_queue));
    if (NS_FAILED(rv)) {
      return false;
    }
  }
#endif

  channel_->SetNotificationCallbacks(this);
  rv = channel_->AsyncOpen(this, nsnull);

  if (NS_FAILED(rv)) {
#if BROWSER_FF2
    if (!async_) {
      event_queue_service->PopThreadEventQueue(modal_event_queue);
    }
#endif
    return false;
  }

  if (!async_) {
#if BROWSER_FF3
    nsCOMPtr<nsIThread> thread;
    if (NS_FAILED(NS_GetCurrentThread(getter_AddRefs(thread)))) {
      return false;
    }
    while (ready_state_ != HttpRequest::COMPLETE && !was_aborted_) {
      if (!NS_ProcessNextEvent(thread)) {
        return false;
      }
    }
#else
    while (ready_state_ != HttpRequest::COMPLETE && !was_aborted_) {
      modal_event_queue->ProcessPendingEvents();

      // Do not busy wait.
      if (ready_state_ != HttpRequest::COMPLETE && !was_aborted_)
        PR_Sleep(PR_MillisecondsToInterval(10));
    }

    event_queue_service->PopThreadEventQueue(modal_event_queue);
#endif
  }

  return true;
}

bool FFHttpRequest::NewByteInputStream(nsIInputStream **stream,
                                       const char *data,
                                       int data_size) {
  assert(stream);
  assert(!(*stream));
  assert(data);

  nsresult rv = NS_OK;
  nsCOMPtr<nsIStringInputStream> string_stream(
      do_CreateInstance("@mozilla.org/io/string-input-stream;1", &rv));
  if (NS_FAILED(rv))
    return false;

  rv = string_stream->ShareData(data, data_size);
  if (NS_FAILED(rv))
    return false;

  rv = CallQueryInterface(string_stream, stream);
  if (NS_FAILED(rv))
    return false;

  return true;
}

//------------------------------------------------------------------------------
// HeaderVisitor, used to implement getAllResponseHeaders()
//------------------------------------------------------------------------------
class HeaderVisitor : public nsIHttpHeaderVisitor {
public:
  // Stack based
  NS_IMETHODIMP_(nsrefcnt) AddRef() { return 1; }
  NS_IMETHODIMP_(nsrefcnt) Release() { return 1; }

  NS_IMETHODIMP QueryInterface(const nsIID &iid, void **result) {
    if (iid.Equals(NS_GET_IID(nsIHttpHeaderVisitor)) ||
        iid.Equals(NS_GET_IID(nsISupports))) {
      *result = this;
      return NS_OK;
    }
    return NS_ERROR_NO_INTERFACE;
  }

  NS_IMETHODIMP VisitHeader(const nsACString &header, const nsACString &value) {
    std::string header_str(header.BeginReading(), header.Length());
    std::string value_str(value.BeginReading(), value.Length());
    headers_.SetHeader(header_str.c_str(),
                       value_str.c_str(),
                       HTTPHeaders::APPEND);
    return NS_OK;
  }

  HeaderVisitor() { }
  virtual ~HeaderVisitor() {}
  HTTPHeaders headers_;
};

//------------------------------------------------------------------------------
// GetAllResponseHeaders
//------------------------------------------------------------------------------
bool FFHttpRequest::GetAllResponseHeaders(std::string16 *headers) {
  NS_ENSURE_TRUE(IsInteractiveOrComplete() && !was_aborted_, false);
  if (IsFileGet()) {
    *headers = STRING16(L"");
    return true;
  }
  nsCOMPtr<nsIHttpChannel> http_channel = GetCurrentHttpChannel();
  NS_ENSURE_TRUE(http_channel, false);

  HeaderVisitor visitor;
  nsresult rv = http_channel->VisitResponseHeaders(&visitor);
  NS_ENSURE_SUCCESS(rv, false);

  std::string header_str;
  for (HTTPHeaders::const_iterator hdr = visitor.headers_.begin();
       hdr != visitor.headers_.end();
       ++hdr) {
    if (hdr->second != NULL) {  // NULL means do not output
      header_str += hdr->first;
      header_str += ": ";
      header_str += hdr->second;
      header_str += HttpConstants::kCrLfAscii;
    }
  }
  header_str += HttpConstants::kCrLfAscii;  // blank line at the end
  return UTF8ToString16(header_str.c_str(), header_str.length(), headers);
}

//------------------------------------------------------------------------------
// GetResponseCharset
//------------------------------------------------------------------------------
std::string16 FFHttpRequest::GetResponseCharset() {
  // TODO(bgarcia): If the document sets the Content-Type charset, return
  // that value.
  // Also need to update blob/blob_utils.cc.
  return std::string16();
}

//------------------------------------------------------------------------------
// GetResponseHeader
//------------------------------------------------------------------------------
bool FFHttpRequest::GetResponseHeader(const char16 *name,
                                      std::string16 *value) {
  NS_ENSURE_TRUE(IsInteractiveOrComplete() && !was_aborted_, false);
  if (IsFileGet()) {
    *value = STRING16(L"");
    return true;
  }
  nsCOMPtr<nsIHttpChannel> http_channel = GetCurrentHttpChannel();
  NS_ENSURE_TRUE(http_channel, false);

  std::string name_utf8;
  if (!String16ToUTF8(name, &name_utf8)) {
    return false;
  }
  nsCString name_str(name_utf8.c_str());
  nsCString value_str;
  nsresult rv = http_channel->GetResponseHeader(name_str, value_str);
  if (rv == NS_ERROR_NOT_AVAILABLE) {
    value->clear();
    return true;
  }
  return UTF8ToString16(value_str.get(), value_str.Length(), value);
}

//------------------------------------------------------------------------------
// Abort
//------------------------------------------------------------------------------
bool FFHttpRequest::Abort() {
  // NS_BINDING_ABORTED is a special error code reserved for this purpose that
  // should not be generated for any other reason.
  if (channel_) {
    channel_->Cancel(NS_BINDING_ABORTED);
    channel_ = NULL;
  }
  was_aborted_ = true;
  return true;
}

//------------------------------------------------------------------------------
// SetListener
//------------------------------------------------------------------------------
bool FFHttpRequest::SetListener(HttpListener *listener,
                                bool enable_data_available) {
  listener_ = listener;
  listener_data_available_enabled_ = enable_data_available;
  return true;
}

//------------------------------------------------------------------------------
// SetReadyState
//------------------------------------------------------------------------------
void FFHttpRequest::SetReadyState(ReadyState state) {
  if (state > ready_state_) {
    ready_state_ = state;
    if (listener_) {
      listener_->ReadyStateChanged(this);
    }
  }
}

//------------------------------------------------------------------------------
// OnStartRequest
//------------------------------------------------------------------------------
NS_IMETHODIMP FFHttpRequest::OnStartRequest(nsIRequest *request,
                                            nsISupports *context) {
  NS_ENSURE_TRUE(channel_, NS_ERROR_UNEXPECTED);
  response_body_.reset(new ByteStore);
  nsCOMPtr<nsIChannel> chan(do_QueryInterface(request));
  if (chan) {
    PRInt32 length = -1;
    chan->GetContentLength(&length);
    if (length >= 0) {
      response_body_->Reserve(length);
    }
  }
  SetReadyState(HttpRequest::SENT);
  return NS_OK;
}

//------------------------------------------------------------------------------
// StreamReaderFunc
//------------------------------------------------------------------------------
NS_METHOD FFHttpRequest::StreamReaderFunc(nsIInputStream *stream,
                                          void *closure,
                                          const char *from_segment,
                                          PRUint32 to_offset,
                                          PRUint32 count,
                                          PRUint32 *write_count) {
  FFHttpRequest *self = reinterpret_cast<FFHttpRequest*>(closure);
  scoped_refptr<ByteStore> body = self->response_body_;
  if (body.get() && body->AddData(from_segment, count)) {
    *write_count = count;
    return NS_OK;
  }
  return NS_ERROR_UNEXPECTED;
}

//------------------------------------------------------------------------------
// OnDataAvailable
//------------------------------------------------------------------------------
NS_IMETHODIMP FFHttpRequest::OnDataAvailable(nsIRequest *request,
                                             nsISupports *context,
                                             nsIInputStream *stream,
                                             PRUint32 offset,
                                             PRUint32 count) {
  scoped_refptr<FFHttpRequest> reference(this);
  NS_ENSURE_TRUE(channel_, NS_ERROR_UNEXPECTED);
  SetReadyState(HttpRequest::INTERACTIVE);

  if (was_aborted_) {
    return NS_OK;
  }

  if (!response_body_.get()) {
    return NS_ERROR_UNEXPECTED;
  }
  int64 prev_size = response_body_->Length();

  PRUint32 n;
  nsresult rv = stream->ReadSegments(StreamReaderFunc, this, count, &n);
  NS_ENSURE_SUCCESS(rv, rv);

  if (response_body_->Length() > prev_size) {
    if (listener_ && listener_data_available_enabled_) {
      listener_->DataAvailable(this, response_body_->Length());
    }
  }

  return rv;
}

//------------------------------------------------------------------------------
// OnStopRequest
//------------------------------------------------------------------------------
NS_IMETHODIMP FFHttpRequest::OnStopRequest(nsIRequest *request,
                                           nsISupports *context,
                                           nsresult status) {
  LOG(("FFHttpRequest::OnStopRequest - %d\n", status));
  if (!was_aborted_) {
    // The Abort() method resets channel_, so we only do this
    // when we haven't explicitly called Abort().
    NS_ENSURE_TRUE(channel_, NS_ERROR_UNEXPECTED);
    channel_->SetNotificationCallbacks(NULL);
  }
  // nsHttpChannel errors are reflected as aborted requests
  was_aborted_ |= NS_FAILED(status) ? true : false;
  SetReadyState(HttpRequest::COMPLETE);
  return NS_OK;
}

void FFHttpRequest::OnUploadProgress(int64 position, int64 total) {
  if (IsComplete() || was_aborted_) return;
  if (listener_) {
    listener_->UploadProgress(this, position, total);
  }
}

//------------------------------------------------------------------------------
// GetCurrentHttpChannel
//------------------------------------------------------------------------------
already_AddRefed<nsIHttpChannel> FFHttpRequest::GetCurrentHttpChannel() {
  nsIHttpChannel *http_channel = nsnull;
  if (channel_) {
    CallQueryInterface(channel_, &http_channel);
  }
  return http_channel;
}

//------------------------------------------------------------------------------
// nsIChannelEventSink::OnChannelRedirect
//------------------------------------------------------------------------------
NS_IMETHODIMP FFHttpRequest::OnChannelRedirect(nsIChannel *old_channel,
                                               nsIChannel *new_channel,
                                               PRUint32 flags) {
  NS_PRECONDITION(new_channel, "Redirect without a channel?");

  std::string16 redirect_url;

  // Get the redirect url
  nsCOMPtr<nsIURI> url;
  nsresult rv = new_channel->GetURI(getter_AddRefs(url));
  NS_ENSURE_SUCCESS(rv, rv);
  nsCString url_utf8;
  rv = url->GetSpec(url_utf8);
  NS_ENSURE_SUCCESS(rv, rv);
  // Convert to string16
  nsString url_utf16;
  rv = NS_CStringToUTF16(url_utf8, NS_CSTRING_ENCODING_UTF8, url_utf16);
  NS_ENSURE_SUCCESS(rv, rv);
  redirect_url = url_utf16.get();

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

  nsresult nr;
  if (!follow) {
    // After cancelling in this fashion, out response getter methods will
    // reflect the response containing the redirect_url in the location header.
    nr = NS_ERROR_ABORT;
  } else {
    was_redirected_ = true;
    redirect_url_ = redirect_url;
    channel_ = new_channel;
    nr = NS_OK;
  }

  return nr;
}

//-----------------------------------------------------------------------------
// nsIInterfaceRequestor::GetInterface
//-----------------------------------------------------------------------------
NS_IMETHODIMP FFHttpRequest::GetInterface(const nsIID &iid, void **result) {
  return QueryInterface(iid, result);
}

//-----------------------------------------------------------------------------
// SpecialHttpRequestInterface::GetNativeHttpRequest
//-----------------------------------------------------------------------------
NS_IMETHODIMP FFHttpRequest::GetNativeHttpRequest(FFHttpRequest **retval) {
  *retval = this;
  return NS_OK;
}

#if BROWSER_FF3
//-----------------------------------------------------------------------------
// nsILoadGroup::GetNotificationCallbacks
//-----------------------------------------------------------------------------
NS_IMETHODIMP GearsLoadGroup::GetNotificationCallbacks(
    nsIInterfaceRequestor * *aNotificationCallbacks) {
  *aNotificationCallbacks = this;
  NS_ADDREF(*aNotificationCallbacks);
  return NS_OK;
}

//-----------------------------------------------------------------------------
// nsIDocShellTreeItem::GetSameTypeRootTreeItem
//-----------------------------------------------------------------------------
NS_IMETHODIMP GearsLoadGroup::GetSameTypeRootTreeItem(
    nsIDocShellTreeItem * *aSameTypeRootTreeItem) {
  *aSameTypeRootTreeItem = this;
  NS_ADDREF(*aSameTypeRootTreeItem);
  return NS_OK;
}

//-----------------------------------------------------------------------------
// nsIDocShellTreeItem::GetItemType
//-----------------------------------------------------------------------------
NS_IMETHODIMP GearsLoadGroup::GetItemType(PRInt32 *aItemType) {
  *aItemType = nsIDocShellTreeItem::typeContent;
  return NS_OK;
}

//-----------------------------------------------------------------------------
// nsIInterfaceRequestor::GetInterface
//-----------------------------------------------------------------------------
NS_IMETHODIMP GearsLoadGroup::GetInterface(const nsIID &iid, void **result) {
  return QueryInterface(iid, result);
}

void GearsLoadGroup::Ref() {
  AddRef();
}

void GearsLoadGroup::Unref() {
  Release();
}

// The rest of the file is unimplemented functions from the interfaces of
// GearsLoadGroup.

/* attribute nsIRequestObserver groupObserver; */
NS_IMETHODIMP GearsLoadGroup::GetGroupObserver(nsIRequestObserver * *aGroupObserver)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP GearsLoadGroup::SetGroupObserver(nsIRequestObserver * aGroupObserver)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute nsIRequest defaultLoadRequest; */
NS_IMETHODIMP GearsLoadGroup::GetDefaultLoadRequest(nsIRequest * *aDefaultLoadRequest)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP GearsLoadGroup::SetDefaultLoadRequest(nsIRequest * aDefaultLoadRequest)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void addRequest (in nsIRequest aRequest, in nsISupports aContext); */
NS_IMETHODIMP GearsLoadGroup::AddRequest(nsIRequest *aRequest, nsISupports *aContext)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void removeRequest (in nsIRequest aRequest, in nsISupports aContext, in nsresult aStatus); */
NS_IMETHODIMP GearsLoadGroup::RemoveRequest(nsIRequest *aRequest, nsISupports *aContext, nsresult aStatus)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsISimpleEnumerator requests; */
NS_IMETHODIMP GearsLoadGroup::GetRequests(nsISimpleEnumerator * *aRequests)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute unsigned long activeCount; */
NS_IMETHODIMP GearsLoadGroup::GetActiveCount(PRUint32 *aActiveCount)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute nsIInterfaceRequestor notificationCallbacks; */
NS_IMETHODIMP GearsLoadGroup::SetNotificationCallbacks(nsIInterfaceRequestor * aNotificationCallbacks)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute wstring name; */
NS_IMETHODIMP GearsLoadGroup::GetName(PRUnichar * *aName)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP GearsLoadGroup::SetName(const PRUnichar * aName)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean nameEquals (in wstring name); */
NS_IMETHODIMP GearsLoadGroup::NameEquals(const PRUnichar *name, PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute long itemType; */
NS_IMETHODIMP GearsLoadGroup::SetItemType(PRInt32 aItemType)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDocShellTreeItem parent; */
NS_IMETHODIMP GearsLoadGroup::GetParent(nsIDocShellTreeItem * *aParent)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDocShellTreeItem sameTypeParent; */
NS_IMETHODIMP GearsLoadGroup::GetSameTypeParent(nsIDocShellTreeItem * *aSameTypeParent)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDocShellTreeItem rootTreeItem; */
NS_IMETHODIMP GearsLoadGroup::GetRootTreeItem(nsIDocShellTreeItem * *aRootTreeItem)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDocShellTreeItem findItemWithName (in wstring name, in nsISupports aRequestor, in nsIDocShellTreeItem aOriginalRequestor); */
NS_IMETHODIMP GearsLoadGroup::FindItemWithName(const PRUnichar *name, nsISupports *aRequestor, nsIDocShellTreeItem *aOriginalRequestor, nsIDocShellTreeItem **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsIDocShellTreeOwner treeOwner; */
NS_IMETHODIMP GearsLoadGroup::GetTreeOwner(nsIDocShellTreeOwner * *aTreeOwner)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* [noscript] void setTreeOwner (in nsIDocShellTreeOwner treeOwner); */
NS_IMETHODIMP GearsLoadGroup::SetTreeOwner(nsIDocShellTreeOwner *treeOwner)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute AUTF8String name; */
NS_IMETHODIMP GearsLoadGroup::GetName(nsACString & aName)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean isPending (); */
NS_IMETHODIMP GearsLoadGroup::IsPending(PRBool *_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* readonly attribute nsresult status; */
NS_IMETHODIMP GearsLoadGroup::GetStatus(nsresult *aStatus)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void cancel (in nsresult aStatus); */
NS_IMETHODIMP GearsLoadGroup::Cancel(nsresult aStatus)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void suspend (); */
NS_IMETHODIMP GearsLoadGroup::Suspend()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void resume (); */
NS_IMETHODIMP GearsLoadGroup::Resume()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute nsILoadGroup loadGroup; */
NS_IMETHODIMP GearsLoadGroup::GetLoadGroup(nsILoadGroup * *aLoadGroup)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP GearsLoadGroup::SetLoadGroup(nsILoadGroup * aLoadGroup)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* attribute nsLoadFlags loadFlags; */
NS_IMETHODIMP GearsLoadGroup::GetLoadFlags(nsLoadFlags *aLoadFlags)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
NS_IMETHODIMP GearsLoadGroup::SetLoadFlags(nsLoadFlags aLoadFlags)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
/* readonly attribute long childCount; */
NS_IMETHODIMP GearsLoadGroup::GetChildCount(PRInt32 *aChildCount)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void addChild (in nsIDocShellTreeItem child); */
NS_IMETHODIMP GearsLoadGroup::AddChild(nsIDocShellTreeItem *child)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void removeChild (in nsIDocShellTreeItem child); */
NS_IMETHODIMP GearsLoadGroup::RemoveChild(nsIDocShellTreeItem *child)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDocShellTreeItem getChildAt (in long index); */
NS_IMETHODIMP GearsLoadGroup::GetChildAt(PRInt32 index, nsIDocShellTreeItem **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* nsIDocShellTreeItem findChildWithName (in wstring aName, in boolean aRecurse, in boolean aSameType, in nsIDocShellTreeItem aRequestor, in nsIDocShellTreeItem aOriginalRequestor); */
NS_IMETHODIMP GearsLoadGroup::FindChildWithName(const PRUnichar *aName, PRBool aRecurse, PRBool aSameType, nsIDocShellTreeItem *aRequestor, nsIDocShellTreeItem *aOriginalRequestor, nsIDocShellTreeItem **_retval)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

#endif
