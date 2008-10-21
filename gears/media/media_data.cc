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
//
// A MediaData object holds the audio buffer data, and also states/properties
// that are shared by the various threads concurrently. This is also the
// protocol object passed to the player's callback function.

#include "gears/media/media_data.h"
#include "gears/media/media_constants.h"

MediaData::MediaData()
     : media_buffer_(NULL),
       last_error_(MediaConstants::MEDIA_NO_ERROR) {
}

void MediaData::LoadBlob(BlobInterface* blob) {
  assert(blob != NULL);
  MutexLock lock(&media_data_lock_);
  media_buffer_.reset(blob);
}

void MediaData::SetLastError(int error_code) {
  MutexLock lock(&media_data_lock_);
  last_error_ = error_code;
}

int MediaData::GetLastError() {
  // TODO(aprasath): Are 'int' updates atomic? Can this lock be removed?
  MutexLock lock(&media_data_lock_);
  return last_error_;
}
