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

#include "gears/localserver/firefox/progress_input_stream.h"

#include "gears/base/common/async_router.h"
#include "gears/base/common/leak_counter.h"
#include "gears/localserver/common/progress_event.h"
#include "gears/localserver/firefox/http_request_ff.h"
#include "gears/localserver/firefox/ui_thread.h"

//------------------------------------------------------------------------------
// ProgressInputStream implementation
//------------------------------------------------------------------------------

ProgressInputStream::ProgressInputStream(
    FFHttpRequest *request,
    nsIInputStream *input_stream,
    int64 total)
    : request_(request),
      input_stream_(input_stream),
      position_(0),
      total_(total) {
  LEAK_COUNTER_INCREMENT(ProgressInputStream);
  assert(request);
}

ProgressInputStream::~ProgressInputStream() {
  LEAK_COUNTER_DECREMENT(ProgressInputStream);
}

//------------------------------------------------------------------------------
// nsIInputStream implementation
//------------------------------------------------------------------------------

NS_IMPL_THREADSAFE_ISUPPORTS1(ProgressInputStream,
                              nsIInputStream)

NS_IMETHODIMP ProgressInputStream::Available(PRUint32 *out_available_bytes) {
  return input_stream_->Available(out_available_bytes);
}

NS_IMETHODIMP ProgressInputStream::Close() {
  return input_stream_->Close();
}

NS_IMETHODIMP ProgressInputStream::IsNonBlocking(PRBool *out_non_blocking) {
  return input_stream_->IsNonBlocking(out_non_blocking);
}

NS_IMETHODIMP ProgressInputStream::Read(char *buffer,
                                        PRUint32 buffer_size,
                                        PRUint32 *out_num_bytes_read) {
  nsresult result = input_stream_->Read(buffer, buffer_size, out_num_bytes_read);
  if (result == NS_OK && *out_num_bytes_read > 0) {
    position_ += *out_num_bytes_read;
    if (request_) {
      ProgressEvent::Update(request_, request_, position_, total_);
    }
  }
  return result;
}

NS_IMETHODIMP ProgressInputStream::ReadSegments(nsWriteSegmentFun writer,
                                                void *writer_arg,
                                                PRUint32 num_bytes_to_read,
                                                PRUint32 *out_num_bytes_read) {
  nsresult result = input_stream_->ReadSegments(writer,
                                                writer_arg,
                                                num_bytes_to_read,
                                                out_num_bytes_read);
  if (result == NS_OK && *out_num_bytes_read > 0) {
    position_ += *out_num_bytes_read;
    if (request_) {
      ProgressEvent::Update(request_, request_, position_, total_);
    }
  }
  return result;
}

void ProgressInputStream::OnFFHttpRequestDestroyed(FFHttpRequest *request) {
  assert(request == request_);
  request_ = NULL;
}
