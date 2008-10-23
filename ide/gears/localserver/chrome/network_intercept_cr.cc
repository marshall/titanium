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

#include "gears/localserver/chrome/network_intercept_cr.h"

#include "gears/base/common/string_utils.h"
#include "gears/localserver/chrome/gears_protocol_handler.h"


// static
NetworkIntercept *NetworkIntercept::Intercept(CPRequest *request,
                                              const std::string16 &url,
                                              BrowsingContext *context) {
  scoped_ptr<WebCacheDB::PayloadInfo> payload(new WebCacheDB::PayloadInfo);

  if (GearsProtocolHandler::Service(url, payload.get())) {
    return new NetworkIntercept(payload.release());
  }

  WebCacheDB* db = WebCacheDB::GetDB();
  if (db && db->Service(url.c_str(), context, false, payload.get())) {
    return new NetworkIntercept(payload.release());
  }

  return NULL;  // we have no response for this url
}

NetworkIntercept::NetworkIntercept(WebCacheDB::PayloadInfo* payload)
    : payload_(payload), offset_(0) {
  assert(payload);
}

int NetworkIntercept::GetResponseInfo(CPResponseInfoType type, void *buf,
                                      uint32 buf_size) {
  switch (type) {
    case CPRESPONSEINFO_HTTP_STATUS:
      if (buf) {
        int status = payload_->status_code;
        memcpy(buf, &status, buf_size);
      }
      break;
    case CPRESPONSEINFO_HTTP_RAW_HEADERS: {
      std::string status_line_utf8;
      if (!String16ToUTF8(payload_->status_line.c_str(), &status_line_utf8))
        return CPERR_FAILURE;

      std::string headers_utf8;
      if (!String16ToUTF8(payload_->headers.c_str(), &headers_utf8))
        return CPERR_FAILURE;

      // Massage headers into the right format, and prepend the status line.
      ReplaceAll(headers_utf8, std::string("\r\n"), std::string("\0", 1));
      headers_utf8 = status_line_utf8 + '\0' + headers_utf8;

      if (buf_size < headers_utf8.size())
        return headers_utf8.size();
      if (buf)
        memcpy(buf, headers_utf8.data(), headers_utf8.size());
      break;
    }
    default:
      return CPERR_INVALID_VERSION;
  }

  return CPERR_SUCCESS;
}

int NetworkIntercept::Read(void *buf, uint32 count) {
  assert(count <= kint32max);

  if (!payload_->data.get())
    return 0;

  uint32 avail = payload_->data->size() - static_cast<uint32>(offset_);
  if (count > avail)
    count = avail;

  if (count) {
    memcpy(buf, &(*payload_->data)[0] + static_cast<uint32>(offset_), count);
    offset_ += count;
  }
  return count;
}
