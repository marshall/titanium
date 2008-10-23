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

#ifndef GEARS_MEDIA_MEDIA_DATA_H__
#define GEARS_MEDIA_MEDIA_DATA_H__

#include <vector>
#include "gears/base/common/common.h"
#include "gears/base/common/mutex.h"
#include "gears/base/common/scoped_refptr.h"
#include "gears/blob/blob.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

// A MediaData object holds the audio buffer data, and also states/properties
// that are shared by the various threads concurrently. 
// This is also the protocol object passed to the player's callback function.
class MediaData : public RefCounted {
 public:
  MediaData();

  // Buffer Methods

  // Method to clear the existing buffer and init the buffer with 
  // bytes from the blob passed.
  void LoadBlob(BlobInterface* blob);

  // Accessor/update methods
  void SetLastError(int error_code);
  int GetLastError();

 private:
  // The buffer can be populated by one of several means
  // - a network stream/blob.
  // This buffer is read by the Audio Player.
  scoped_refptr<BlobInterface> media_buffer_;

  // This variable stores the 'error' state reported by GearsMedia.
  // The player or network thread or GearsMedia class could all potentially
  // update or read this state.
  int last_error_;

  // The main thread, network thread and the player thread can all
  // access/write media_buffer_ and other shared properties.
  // The below mutex is used for synchronizing this.
  Mutex media_data_lock_;

  DISALLOW_EVIL_CONSTRUCTORS(MediaData);
};

#endif  // GEARS_MEDIA_MEDIA_DATA_H__
