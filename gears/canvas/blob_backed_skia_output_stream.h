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

#ifndef GEARS_CANVAS_BLOB_BACKED_SKIA_OUTPUT_STREAM_H__
#define GEARS_CANVAS_BLOB_BACKED_SKIA_OUTPUT_STREAM_H__

#include "gears/blob/blob_builder.h"
#include "third_party/skia/include/SkStream.h"

class BlobBackedSkiaOutputStream : public SkWStream {
 public:
  BlobBackedSkiaOutputStream();
  virtual ~BlobBackedSkiaOutputStream();

  // Writes `size` bytes to stream, returning true on success.
  virtual bool write(const void* buffer, size_t size);

  // Returns a blob containing the data written so far.
  void CreateBlob(scoped_refptr<BlobInterface> *blob);

 private:
  BlobBuilder blob_builder_;
  DISALLOW_EVIL_CONSTRUCTORS(BlobBackedSkiaOutputStream);
};

#endif  // GEARS_CANVAS_BLOB_BACKED_SKIA_OUTPUT_STREAM_H__
