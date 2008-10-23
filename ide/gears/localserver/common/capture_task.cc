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

#include "gears/localserver/common/capture_task.h"

#include "gears/base/common/exception_handler.h"
#ifdef WINCE
#include "gears/base/common/wince_compatibility.h"  // For BrowserCache
#endif
#include "gears/blob/blob_interface.h"
#include "gears/blob/blob_utils.h"
#include "gears/localserver/common/http_constants.h"


//------------------------------------------------------------------------------
// Init
//------------------------------------------------------------------------------
bool CaptureTask::Init(ResourceStore *store, CaptureRequest *request) {
  if (!AsyncTask::Init()) {
    return false;
  }
  assert(store);
  assert(request);
  if (!store->StillExistsInDB() || !store_.Clone(store)) {
    is_initialized_ = false;
    return false;
  }
  capture_request_ = request;
  processed_urls_.clear();
  return true;
}


//------------------------------------------------------------------------------
// NotifyTaskComplete
//------------------------------------------------------------------------------
void CaptureTask::NotifyTaskComplete(bool success) {
  NotifyListener(CAPTURE_TASK_COMPLETE, success ? -1 : 0);
}


//------------------------------------------------------------------------------
// NotifyUrlComplete
//------------------------------------------------------------------------------
void CaptureTask::NotifyUrlComplete(int index, bool success) {
  NotifyListener(success ? CAPTURE_URL_SUCCEEDED
                         : CAPTURE_URL_FAILED,
                 index);
}


//------------------------------------------------------------------------------
// Run
//------------------------------------------------------------------------------
void CaptureTask::Run() {
  WebCacheDB* db = WebCacheDB::GetDB();
  if (!db) {
    return;
  }

  int num_urls = GetUrlCount();
  for (int i = 0; i < num_urls; i++) {
    std::string16 url;
    bool success = GetUrl(i, &url) && ProcessUrl(url);
    NotifyUrlComplete(i, success);
  }
  NotifyTaskComplete(!is_aborted_);
}

//------------------------------------------------------------------------------
// GetUrlCount
//------------------------------------------------------------------------------
int CaptureTask::GetUrlCount() {
  CritSecLock locker(lock_);
  if (is_aborted_) {
    return 0;
  }
  return capture_request_->full_urls.size();
}

//------------------------------------------------------------------------------
// GetUrl
//------------------------------------------------------------------------------
bool CaptureTask::GetUrl(int index, std::string16 *url) {
  CritSecLock locker(lock_);
  if (is_aborted_) {
    return false;
  }
  *url = capture_request_->full_urls[index];
  return true;
}

//------------------------------------------------------------------------------
// HttpGetUrl
//------------------------------------------------------------------------------
bool CaptureTask::HttpGetUrl(const char16 *full_url,
                             const char16 *if_mod_since_date,
                             WebCacheDB::PayloadInfo *payload) {
  // TODO(michaeln): see UpdateTask::HttpGetUrl for details

  // TODO(andreip): remove this once WebCacheDB::PayloadInfo.data is a Blob.
  scoped_refptr<BlobInterface> payload_data;
  // Fetch the url from a server
  if (!AsyncTask::HttpGet(full_url,
                          true,  // is_capturing
                          NULL,  // X-Gears-Reason header value
                          if_mod_since_date,
                          store_.GetRequiredCookie(),
                          payload,
                          &payload_data,
                          NULL, NULL, NULL)) {
    LOG(("CaptureTask::HttpGetUrl - failed to get url\n"));
    return false;  // TODO(michaeln): retry?
  }

  if (payload_data.get()) {
    payload->data.reset(new std::vector<uint8>(
                            static_cast<size_t>(payload_data->Length())));
    if (!BlobToVector(payload_data.get(), payload->data.get())) {
      LOG(("CaptureTask::HttpGetUrl - could not extract the payload\n"));
    }
  }

  if (!payload->PassesValidationTests(NULL)) {
    LOG(("CaptureTask::HttpGetUrl - received invalid payload\n"));
    return false;  // TODO(michaeln): retry?
  }

  return true;
}

//------------------------------------------------------------------------------
// ProcessUrl
//------------------------------------------------------------------------------
bool CaptureTask::ProcessUrl(const std::string16 &url) {
  // Don't continue to fetch urls if the store has been removed
  if (!store_.StillExistsInDB()) {
    return false;
  }

  // Should already have been checked when Store.capture(...) executed
  // at the JavaScript bindings layer
  assert(store_.GetSecurityOrigin().IsSameOriginAsUrl(url.c_str()));

  // If we've already processed this url within this capture task, there is
  // no need to fetch it again. This can occur if the array urls the caller
  // has passed in contains duplicates, or when working with multiple urls
  // that redirect to the same location.
  if (processed_urls_.find(url) != processed_urls_.end()) {
    return true;
  }

  // If we have an existing item for this url in our store, get it's
  // modification date
  std::string16 previous_version_mod_date;
  ResourceStore::Item existing_item;
  if (store_.GetItemInfo(url.c_str(), &existing_item)) {
    existing_item.payload.GetHeader(HttpConstants::kLastModifiedHeader,
                                    &previous_version_mod_date);
  }

  // Make an HTTP GET IF_MODIFIED_SINCE request
  ResourceStore::Item new_item;
  if (!HttpGetUrl(url.c_str(),
                  previous_version_mod_date.c_str(),
                  &new_item.payload)) {
    return false;
  }

  if (new_item.payload.status_code == HttpConstants::HTTP_NOT_MODIFIED) {
    LOG(("CaptureTask::ProcessUrl - received HTTP_NOT_MODIFIED\n"));
    processed_urls_.insert(url);
#ifdef WINCE
    BrowserCache::EnsureBogusEntry(url.c_str());
#endif
    return true;
  } else if (new_item.payload.status_code != HttpConstants::HTTP_OK) {
    LOG(("CaptureTask::ProcessUrl - received bad response %d\n",
         new_item.payload.status_code));
    return false;
  }

  new_item.entry.url = url;
  bool success = store_.PutItem(&new_item);
  if (success) {
    processed_urls_.insert(url);
  }
  return success;
}
