// Copyright 2005, Google Inc.
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

#include <string>
#include <vector>
#include <gecko_sdk/include/nsXPCOM.h>
#include <gecko_sdk/include/nsIURI.h>
#include <gecko_sdk/include/nsIRequest.h>
#include <gecko_sdk/include/nsIOutputStream.h>
#include <gecko_sdk/include/nsIObserverService.h>
#include <gecko_sdk/include/nsIObserver.h>
#include <gecko_sdk/include/nsIInterfaceRequestor.h>
#include <gecko_sdk/include/nsIInputStream.h>
#include <gecko_sdk/include/nsIHttpChannel.h>
#include <gecko_sdk/include/nsIComponentRegistrar.h>
#include <gecko_sdk/include/nsIChannel.h>
#include <gecko_sdk/include/nsICategoryManager.h>
#include <gecko_sdk/include/prtime.h>
#include <gecko_internal/nsICacheEntryDescriptor.h>
#include <gecko_internal/nsICacheListener.h>
#include <gecko_internal/nsICacheService.h>
#include <gecko_internal/nsICacheSession.h>
#include <gecko_internal/nsICacheVisitor.h>
#if BROWSER_FF3
#include <gecko_internal/nsThreadUtils.h>
#endif
#include "genfiles/interfaces.h"
#include "gears/base/common/exception_handler.h"
#include "gears/base/common/message_queue.h"
#include "gears/base/common/string_utils.h"
#include "gears/base/common/trace_buffers_win32/trace_buffers_win32.h"
#include "gears/base/common/user_config.h"
#include "gears/factory/factory_utils.h"
#include "gears/localserver/common/localserver_db.h"
#include "gears/localserver/firefox/cache_intercept.h"
#include "gears/localserver/firefox/http_request_ff.h"

// Used to determine when we're executing on the main thread of control
static ThreadId g_ui_thread = 0;


// Object identifiers
const char *kCacheInterceptContractId = "@google.com/gears/cacheintercept;1";
const char *kCacheInterceptClassName = "CacheIntercept";
const nsCID kCacheInterceptClassId = {0x3ec37181, 0x7f37, 0x4e49, {0xbb, 0xeb,
                                      0x2c, 0xc3, 0xdf, 0x39, 0x16, 0x28}};
                                     // {3EC37181-7F37-4e49-BBEB-2CC3DF391628}


// We use nsIObserverService to get notifications of shift+reload.
const char *kObserverServiceContractId = "@mozilla.org/observer-service;1";
const char *kOnModifyRequestTopic = "http-on-modify-request";


// ClassID of the built-in cache service
#define NS_CACHESERVICE_CID                        \
{ /* 6c84aec9-29a5-4264-8fbc-bee8f922ea67 */       \
  0x6c84aec9,                                      \
  0x29a5,                                          \
  0x4264,                                          \
  {0x8f, 0xbc, 0xbe, 0xe8, 0xf9, 0x22, 0xea, 0x67} \
}
// ContractID of the built-in cache service
#define NS_CACHESERVICE_CONTRACTID \
  "@mozilla.org/network/cache-service;1"

//-----------------------------------------------------------------------------
// NowInSeconds
//-----------------------------------------------------------------------------
static PRUint32 NowInSeconds() {
  return PRUint32(PR_Now() / PR_USEC_PER_SEC);  // PR_Now returns microseconds
}

//-----------------------------------------------------------------------------
// AbstractCacheMetaDataVisitor
//-----------------------------------------------------------------------------
class AbstractCacheMetaDataVisitor : public nsICacheMetaDataVisitor {
 public:
  // Stack allocated
  NS_IMETHODIMP_(nsrefcnt) AddRef() { return 1; }
  NS_IMETHODIMP_(nsrefcnt) Release() { return 1; }

  NS_IMETHODIMP QueryInterface(const nsIID &iid, void **result) {
    if (iid.Equals(NS_GET_IID(nsICacheMetaDataVisitor)) ||
        iid.Equals(NS_GET_IID(nsISupports))) {
      *result = this;
      return NS_OK;
    }
    return NS_ERROR_NO_INTERFACE;
  }
};


//-----------------------------------------------------------------------------
// VisitSerializedCacheEntryMetaData
//-----------------------------------------------------------------------------
static void VisitSerializedCacheEntryMetaData(const nsCString &meta_data,
    nsICacheMetaDataVisitor *visitor) {
  // Names and values are separated by null bytes

  const char *start;
  PRUint32 len = NS_CStringGetData(meta_data, &start);

  PRBool keep_going = PR_TRUE;

  const char *iter = start;
  const char *end = start + len;
  for (; keep_going && iter < end; ++iter) {
    // Find next null byte
    if (*iter)
      continue;

    const char *name = start;
    start = ++iter;

    // Find next null byte
    for (; *iter && iter < end; ++iter);

    // Ensure we found a null
    if (*iter)
      return;

    const char *value = start;
    start = ++iter;

    if (NS_FAILED(visitor->VisitMetaDataElement(name, value, &keep_going)))
      keep_going = PR_FALSE;
  }
}


//-----------------------------------------------------------------------------
// ReplayInputStream
//-----------------------------------------------------------------------------
class ReplayInputStream : public nsIInputStream {
 public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIINPUTSTREAM

  ReplayInputStream(nsISupports *entry,
                    std::vector<unsigned char> *data,
                    PRUint32 offset)
      : entry_(entry),
        data_(data),
        offset_(offset) {
  }

 private:
  ~ReplayInputStream() { Close(); }

  nsCOMPtr<nsISupports> entry_;
  std::vector<unsigned char> *data_;
  PRUint32 offset_;
};

NS_IMPL_THREADSAFE_ISUPPORTS1(ReplayInputStream, nsIInputStream)

NS_IMETHODIMP ReplayInputStream::Close() {
  entry_ = nsnull;
  data_ = nsnull;
  return NS_OK;
}

NS_IMETHODIMP ReplayInputStream::Available(PRUint32 *avail) {
  if (!entry_)
    return NS_BASE_STREAM_CLOSED;

  *avail = data_ ? (data_->size() - offset_) : 0;
  return NS_OK;
}

NS_IMETHODIMP ReplayInputStream::Read(char *buf, PRUint32 count,
                                          PRUint32 *result) {
  if (!entry_ || !data_) {
    *result = 0;
    return NS_OK;
  }

  PRUint32 avail = data_->size() - offset_;
  if (count > avail)
    count = avail;

  if (count) {
    memcpy(buf, &(*data_)[0] + offset_, count);
  }

  offset_ += count;
  *result = count;
  return NS_OK;
}

NS_IMETHODIMP ReplayInputStream::ReadSegments(nsWriteSegmentFun callback,
                                              void *closure,
                                              PRUint32 count,
                                              PRUint32 *result) {
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP ReplayInputStream::IsNonBlocking(PRBool *result) {
  *result = PR_FALSE;
  return NS_OK;
}

//-----------------------------------------------------------------------------
// ReplayCacheEntry
//-----------------------------------------------------------------------------
class ReplayCacheEntry : public nsICacheEntryDescriptor {
 public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSICACHEENTRYINFO
  NS_DECL_NSICACHEENTRYDESCRIPTOR

 protected:
  ~ReplayCacheEntry() { Close(); }

 private:
  friend class CacheSession;

  ReplayCacheEntry(const nsACString &client_id, const nsACString &key)
      : client_id_(client_id), key_(key) {
  }

  static bool AttemptToReplay(const nsACString &key,
                              nsICacheEntryDescriptor **result);
  bool LoadFromCache();

  nsCString client_id_;
  nsCString key_;
  nsCString meta_data_;
  WebCacheDB::PayloadInfo payload_;
};

// static
bool ReplayCacheEntry::AttemptToReplay(const nsACString &key,
                                       nsICacheEntryDescriptor **result) {
  // TODO(michaeln): support HEAD requests
  // TODO(darin): use proper client_id
  NS_NAMED_LITERAL_CSTRING(client_id, "HTTP");
  nsCOMPtr<ReplayCacheEntry> entry = new ReplayCacheEntry(client_id, key);
  if (!entry->LoadFromCache()) {
    return false;
  }
  NS_ADDREF(*result = entry);
  return true;
}

bool ReplayCacheEntry::LoadFromCache() {
  WebCacheDB* db = WebCacheDB::GetDB();
  if (!db) {
    return false;
  }

  std::string16 url;
  if (!UTF8ToString16(key_.get(), key_.Length(), &url)) {
    return false;
  }

  if (!db->Service(url.c_str(), NULL, false, &payload_)) {
    return false;  // we have no response for this url
  }

  // synthesize cache meta_data_ based on our cached response

  // Values are stored as a wide character string in the webcache database,
  // we convert to ASCII here to satisfy what firefox wants to see in its
  // meta data.  The conversion to UTF8 == a converstion to ASCII since the
  // source data for the string was ASCII to begin with.
  std::string headers_utf8;
  std::string status_line_utf8;
  if (!String16ToUTF8(payload_.status_line.c_str(),
                      payload_.status_line.length(),
                      &status_line_utf8) ||
      !String16ToUTF8(payload_.headers.c_str(),
                      payload_.headers.length(),
                      &headers_utf8)) {
    return false;
  }

  const char *kRequestMethod = "request-method";
  const char *kResponseHead = "response-head";
  const char kNullChar = '\0';
  meta_data_.Append(kRequestMethod);
  meta_data_.Append(kNullChar);
  meta_data_.Append(HttpConstants::kHttpGETAscii);
  meta_data_.Append(kNullChar);
  meta_data_.Append(kResponseHead);
  meta_data_.Append(kNullChar);
  meta_data_.Append(status_line_utf8.c_str());
  meta_data_.Append(HttpConstants::kCrLfAscii);
  meta_data_.Append(headers_utf8.c_str());
  meta_data_.Append(kNullChar);

  return true;
}

NS_IMPL_THREADSAFE_ISUPPORTS2(ReplayCacheEntry,
                              nsICacheEntryDescriptor,
                              nsICacheEntryInfo)

NS_IMETHODIMP ReplayCacheEntry::GetClientID(char **value) {
  *value = NS_CStringCloneData(client_id_);
  return *value ? NS_OK : NS_ERROR_OUT_OF_MEMORY;
}

NS_IMETHODIMP ReplayCacheEntry::GetDeviceID(char **value) {
  *value = NS_CStringCloneData(NS_LITERAL_CSTRING(PRODUCT_SHORT_NAME_ASCII));
  return *value ? NS_OK : NS_ERROR_OUT_OF_MEMORY;
}

NS_IMETHODIMP ReplayCacheEntry::GetKey(nsACString &value) {
  value = key_;
  return NS_OK;
}

NS_IMETHODIMP ReplayCacheEntry::GetFetchCount(PRInt32 *value) {
  *value = 1;  // fudge
  return NS_OK;
}

NS_IMETHODIMP ReplayCacheEntry::GetLastFetched(PRUint32 *value) {
  *value = NowInSeconds() - 10;  // fudge
  return NS_OK;
}

NS_IMETHODIMP ReplayCacheEntry::GetLastModified(PRUint32 *value) {
  *value = NowInSeconds() - 10;  // fudge
  return NS_OK;
}

NS_IMETHODIMP ReplayCacheEntry::GetExpirationTime(PRUint32 *value) {
  *value = NowInSeconds() + 10;  // fudge
  return NS_OK;
}

NS_IMETHODIMP ReplayCacheEntry::GetDataSize(PRUint32 *value) {
  *value = payload_.data.get() ? payload_.data->size() : 0;
  return NS_OK;
}

NS_IMETHODIMP ReplayCacheEntry::IsStreamBased(PRBool *value) {
  *value = PR_TRUE;
  return NS_OK;
}

NS_IMETHODIMP ReplayCacheEntry::SetExpirationTime(PRUint32 value) {
  // Ignore
  return NS_OK;
}

NS_IMETHODIMP ReplayCacheEntry::SetDataSize(PRUint32 value) {
  // Ignore
  return NS_OK;
}

NS_IMETHODIMP ReplayCacheEntry::OpenInputStream(PRUint32 offset,
                                                nsIInputStream **result) {
  LOG(("ReplayCacheEntry::OpenInputStream\n"));

  size_t payload_size = payload_.data.get() ? payload_.data->size() : 0;
  NS_ENSURE_ARG(offset <= payload_size);

  *result = new ReplayInputStream(this, payload_.data.get(), offset);
  if (!*result)
    return NS_ERROR_OUT_OF_MEMORY;

  NS_ADDREF(*result);
  return NS_OK;
}

NS_IMETHODIMP
ReplayCacheEntry::OpenOutputStream(PRUint32 offset,
                                   nsIOutputStream **result) {
  // We're not open for writing.
  return NS_ERROR_UNEXPECTED;
}

NS_IMETHODIMP ReplayCacheEntry::GetCacheElement(nsISupports **value) {
  *value = nsnull;
  return NS_OK;
}

NS_IMETHODIMP ReplayCacheEntry::SetCacheElement(nsISupports *value) {
  // We're not open for writing.
  return NS_ERROR_UNEXPECTED;
}

NS_IMETHODIMP
ReplayCacheEntry::GetAccessGranted(nsCacheAccessMode *value) {
  *value = nsICache::ACCESS_READ;
  return NS_OK;
}

NS_IMETHODIMP
ReplayCacheEntry::GetStoragePolicy(nsCacheStoragePolicy *value) {
  // TODO(darin): make this depend on client_id_
  *value = nsICache::STORE_ON_DISK;
  return NS_OK;
}

NS_IMETHODIMP ReplayCacheEntry::SetStoragePolicy(nsCacheStoragePolicy) {
  // We're not open for writing.
  return NS_ERROR_UNEXPECTED;
}

NS_IMETHODIMP ReplayCacheEntry::GetFile(nsIFile **value) {
  *value = nsnull;
  return NS_OK;
}

NS_IMETHODIMP ReplayCacheEntry::GetSecurityInfo(nsISupports **value) {
  // TODO(darin): HTTPS caching needs to store a value here.
  *value = nsnull;
  return NS_OK;
}

NS_IMETHODIMP ReplayCacheEntry::SetSecurityInfo(nsISupports *value) {
  // We're not open for writing.
  return NS_OK;
}

NS_IMETHODIMP ReplayCacheEntry::Doom() {
  // Ignore
  return NS_OK;
}

NS_IMETHODIMP ReplayCacheEntry::DoomAndFailPendingRequests(nsresult) {
  // Ignore
  return NS_OK;
}

NS_IMETHODIMP ReplayCacheEntry::MarkValid() {
  // Ignore
  return NS_OK;
}

NS_IMETHODIMP ReplayCacheEntry::Close() {
  // TODO(darin): Make us act like we've been closed?
  return NS_OK;
}


//-----------------------------------------------------------------------------
// FindMetaDataElement
//-----------------------------------------------------------------------------
class FindMetaDataElement : public AbstractCacheMetaDataVisitor {
 public:
  FindMetaDataElement(const char *name, char **value)
      : name_(name),
        value_(value) {
  }

  NS_IMETHODIMP VisitMetaDataElement(const char *name,
                                     const char *value,
                                     PRBool *keep_going) {
    *keep_going = (strcmp(name, name_) != 0);

    if (!*keep_going)  // found it
      *value_ = NS_CStringCloneData(nsDependentCString(value));

    return NS_OK;
  }

 private:
  const char *name_;
  char **value_;
};

NS_IMETHODIMP ReplayCacheEntry::GetMetaDataElement(const char *name,
                                                   char **value) {
  FindMetaDataElement finder(name, value);
  VisitSerializedCacheEntryMetaData(meta_data_, &finder);
  return *value ? NS_OK : NS_ERROR_NOT_AVAILABLE;
}

NS_IMETHODIMP ReplayCacheEntry::SetMetaDataElement(const char *name,
                                                   const char *value) {
  // We're not open for writing.
  return NS_ERROR_UNEXPECTED;
}

NS_IMETHODIMP
ReplayCacheEntry::VisitMetaData(nsICacheMetaDataVisitor *visitor) {
  VisitSerializedCacheEntryMetaData(meta_data_, visitor);
  return NS_OK;
}



//-----------------------------------------------------------------------------
// CacheSession
//-----------------------------------------------------------------------------
class CacheSession : public nsICacheSession {
 public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSICACHESESSION

  CacheSession(nsICacheSession *inner) : inner_(inner) {
  }

 private:
  nsCOMPtr<nsICacheSession> inner_;
};

NS_IMPL_THREADSAFE_ISUPPORTS1(CacheSession, nsICacheSession)

NS_IMETHODIMP CacheSession::GetDoomEntriesIfExpired(PRBool *value) {
  return inner_->GetDoomEntriesIfExpired(value);
}

NS_IMETHODIMP CacheSession::SetDoomEntriesIfExpired(PRBool value) {
  return inner_->SetDoomEntriesIfExpired(value);
}

NS_IMETHODIMP
CacheSession::OpenCacheEntry(const nsACString &key,
                             nsCacheAccessMode access_req,
                             PRBool blockingMode,
                             nsICacheEntryDescriptor **result) {
  // NOTE: force reloads (shift-reload) bypass this intercept layer
  LOG(("CacheSession::OpenCacheEntry( %s )\n",
       nsCString(key).get()));

  if (!(access_req & nsICache::ACCESS_READ)) {
    LOG(("CacheSession: not a read request, using default handling\n"));
    return inner_->OpenCacheEntry(key, access_req, blockingMode, result);
  }

  if (ReplayCacheEntry::AttemptToReplay(key, result)) {
    LOG(("CacheSession: cache hit\n"));
    return NS_OK;
  } else {
    LOG(("CacheSession: cache miss, using default handling\n"));
    return inner_->OpenCacheEntry(key, access_req, blockingMode, result);
  }
}

NS_IMETHODIMP
CacheSession::AsyncOpenCacheEntry(const nsACString &key,
                                  nsCacheAccessMode access_req,
                                  nsICacheListener *listener) {
  // No need to lookup in our cache here.
  return inner_->AsyncOpenCacheEntry(key, access_req, listener);
}

NS_IMETHODIMP CacheSession::EvictEntries() {
  return inner_->EvictEntries();
}

NS_IMETHODIMP CacheSession::IsStorageEnabled(PRBool *result) {
  return inner_->IsStorageEnabled(result);
}

//-----------------------------------------------------------------------------
// CacheIntercept
//-----------------------------------------------------------------------------
NS_IMPL_THREADSAFE_ISUPPORTS2(CacheIntercept,
                              nsICacheService,
                              nsIObserver)

NS_IMETHODIMP CacheIntercept::CreateSession(const char *client_id,
                                            nsCacheStoragePolicy policy,
                                            PRBool stream_based,
                                            nsICacheSession **result) {
  LOG(("CacheIntercept::CreateSession [%s]\n", client_id));
  NS_ENSURE_TRUE(default_cache_, NS_ERROR_NOT_INITIALIZED);

  nsCOMPtr<nsICacheSession> inner_session;
  nsresult rv = default_cache_->CreateSession(client_id, policy, stream_based,
                                              getter_AddRefs(inner_session));
  if (NS_FAILED(rv))
    return rv;

  if (!MayIntercept(client_id) ||
      !(*result = new CacheSession(inner_session))) {
    NS_ADDREF(*result = inner_session);
  } else {
    NS_ADDREF(*result);
  }
  return NS_OK;
}

#if BROWSER_FF3
NS_IMETHODIMP CacheIntercept::CreateTemporaryClientID(
                                  nsCacheStoragePolicy policy,
                                  nsACString &retval) {
  LOG(("CacheIntercept::CreateTemporaryClientID\n"));
  NS_ENSURE_TRUE(default_cache_, NS_ERROR_NOT_INITIALIZED);
  return default_cache_->CreateTemporaryClientID(policy, retval);
}
#endif

NS_IMETHODIMP CacheIntercept::VisitEntries(nsICacheVisitor *visitor) {
  LOG(("CacheIntercept::VisitEntries\n"));
  NS_ENSURE_TRUE(default_cache_, NS_ERROR_NOT_INITIALIZED);
  return default_cache_->VisitEntries(visitor);
}

NS_IMETHODIMP CacheIntercept::EvictEntries(nsCacheStoragePolicy policy) {
  LOG(("CacheIntercept::EvictEntries\n"));
  NS_ENSURE_TRUE(default_cache_, NS_ERROR_NOT_INITIALIZED);
  return default_cache_->EvictEntries(policy);
}

NS_IMETHODIMP CacheIntercept::Observe(nsISupports *subject,
                                      const char *topic,
                                      const PRUnichar *data) {
  if (0 == strcmp(topic, NS_XPCOM_STARTUP_CATEGORY)) {
    Init();
  } else if (0 == strcmp(topic, kOnModifyRequestTopic)) {
    MaybeForceToCache(subject);
  } else {
    // We didn't sign up for any other topics, so we should not get here.
    assert(false);
    LOG(("CacheIntercept: Unexpected observer topic: %s", topic));
  }

  return NS_OK;
}

void CacheIntercept::Init() {
#if defined(WIN32) && !defined(WINCE)
// Only send crash reports for offical builds.  Crashes on an engineer's machine
// during internal development are confusing false alarms.
#ifdef OFFICIAL_BUILD
  if (IsUsageReportingAllowed()) {
    static ExceptionManager exception_manager(false);  // false == only our DLL
    exception_manager.StartMonitoring();
    // Trace buffers only exist in dbg official builds.
#ifdef DEBUG
    exception_manager.AddMemoryRange(g_trace_buffers,
                                     sizeof(g_trace_buffers));
    exception_manager.AddMemoryRange(g_trace_positions,
                                     sizeof(g_trace_positions));
#endif  // DEBUG
  }
#endif  // OFFICIAL_BUILD
#endif  // WIN32 && !WINCE

  std::string16 buildinfo;
  AppendBuildInfo(&buildinfo);
  std::string buildinfo_utf8;
  String16ToUTF8(buildinfo.c_str(), &buildinfo_utf8);
  LOG(("CacheIntercept::Init, Gears %s\n", buildinfo_utf8.c_str()));

  // We're initialized on the main ui thread of which there is exactly one
  // in firefox/mozila. Save this thread value so that we can easily
  // determine when we're executing on this thread of control.
  g_ui_thread = ThreadMessageQueue::GetInstance()->GetCurrentThreadId();

  const nsCID kCacheCID = NS_CACHESERVICE_CID;
  default_cache_ = do_GetService(kCacheCID);
  if (!default_cache_) {
    LOG(("CacheIntercept: no cache service\n"));
    return;
  }

  nsCOMPtr<nsIFactory> factory =
      do_GetClassObject(kCacheInterceptContractId);
  if (!factory) {
    LOG(("CacheIntercept: no factory\n"));
    return;
  }

  // Override the default cache.
  nsCOMPtr<nsIComponentRegistrar> reg;
  NS_GetComponentRegistrar(getter_AddRefs(reg));
  if (!reg) {
    LOG(("CacheIntercept: no compreg\n"));
    return;
  }

  nsresult rv = reg->RegisterFactory(kCacheCID,
                                     kCacheInterceptClassName,
                                     NS_CACHESERVICE_CONTRACTID,
                                     factory);
  if (NS_FAILED(rv)) {
    LOG(("CacheIntercept: failed to register factory\n"));
  }

  nsCOMPtr<nsIObserverService> observer_service(
      do_GetService(kObserverServiceContractId));
  if (!observer_service) {
    LOG(("CacheIntercept: Could not get observer service"));
  }

  // We let the observer service hold a strong reference to us since we are
  // alive for the life of the application anyway.
  observer_service->AddObserver(this, kOnModifyRequestTopic, false);

  LOG(("CacheIntercept: registration complete\n"));
}

void CacheIntercept::MaybeForceToCache(nsISupports *subject) {
  nsCOMPtr<nsIRequest> request(do_QueryInterface(subject));
  if (!request) {
    LOG(("CacheIntercept: could not get request"));
    return;
  }

  // If the request is already going to the cache do nothing.
  nsLoadFlags flags(0);
  request->GetLoadFlags(&flags);
  if (0 == (flags & nsIRequest::LOAD_BYPASS_CACHE)) {
    return;
  }

  nsCOMPtr<nsIChannel> channel(do_QueryInterface(request));
  if (!channel) {
    LOG(("CacheIntercept: could not get channel"));
    return;
  }

  // Gears requests can intentionally bypass caches.
  FFHttpRequest *gears_http_request = GetGearsHttpRequest(channel);
  if (gears_http_request &&
      gears_http_request->GetCachingBehavior() ==
                              HttpRequest::BYPASS_ALL_CACHES) {
    return;
  }

  // Only drive GET and HEAD requests to our cache.
  nsCOMPtr<nsIHttpChannel> http_channel(do_QueryInterface(channel));
  if (!http_channel) {
    LOG(("CacheIntercept: could not get http channel"));
    return;
  }

  nsCString method;
  http_channel->GetRequestMethod(method);
  if (!method.Equals(NS_LITERAL_CSTRING("GET")) &&
      !method.Equals(NS_LITERAL_CSTRING("HEAD"))) {
    return;
  }

  // If the URL can be served locally, force it to go to the cache.
  nsCOMPtr<nsIURI> uri;
  channel->GetURI(getter_AddRefs(uri));
  if (!uri) {
    LOG(("CacheIntercept: could not get uri"));
    return;
  }

  WebCacheDB *db = WebCacheDB::GetDB();
  if (!db) {
    LOG(("CacheIntercept: Could not get WebCacheDB"));
    return;
  }

  nsCString spec;
  uri->GetSpec(spec);

  // TODO(aa): It would be nice to not have to hit the database twice for these
  // requests. Perhaps cache the result of this query somewhere?
  if (db->CanService(NS_ConvertUTF8toUTF16(spec).get(), NULL)) {
    LOG(("CacheIntercept: Caught ctrl+refresh, removing LOAD_BYPASS_CACHE"));
    request->SetLoadFlags(flags & ~nsIRequest::LOAD_BYPASS_CACHE);
  }
}

FFHttpRequest *CacheIntercept::GetGearsHttpRequest(nsIChannel *channel) {
  // TODO(michaeln): perhaps we should use Get/SetOwner() for this purpose
  // instead. If some other OnModifyRequest listener injects their own
  // listener which chains to the existing listener, we could get cut
  // out of the loop?
  nsCOMPtr<nsIInterfaceRequestor> listener;
  channel->GetNotificationCallbacks(getter_AddRefs(listener));
  if (!listener) return NULL;

  nsCOMPtr<SpecialHttpRequestInterface> gears_request(
      do_QueryInterface(listener));
  if (!gears_request) return NULL;

  FFHttpRequest *http_request = NULL;
  gears_request->GetNativeHttpRequest(&http_request);
  return http_request;
}

// See ui_thread.h for declaration.
ThreadId GetUiThread() {
  assert(g_ui_thread);
  return g_ui_thread;
}

// See ui_thread.h for declaration.
bool IsUiThread() {
  assert(g_ui_thread);
  return g_ui_thread == ThreadMessageQueue::GetInstance()->GetCurrentThreadId();
}
