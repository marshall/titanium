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

#ifndef GEARS_BLOB_BLOB_UTILS_H_
#define GEARS_BLOB_BLOB_UTILS_H_

#include <vector>
#include "gears/base/common/string16.h"

class BlobInterface;

// Converts the blob's contents to a UTF16 string.
// If charset is empty, assumes UTF8.
// The length of the blob must be smaller or equal to kuint32max,
// otherwise an assertion is triggered.
bool BlobToString16(BlobInterface *blob, const std::string16 &charset,
                    std::string16 *text);

// Copy the blob's contents to a string. No assumptions are made about the data
// type of the content, it is copied verbatim into the string.
// The length of the blob must be smaller or equal to largest possible size
// of the string, otherwise an assertion is triggered.
bool BlobToString(BlobInterface *blob, std::string *string_out);

// Convert the blob's contents to a vector.
// The length of the blob must be smaller or equal to largest possible size
// of the vector, otherwise an assertion is triggered.
bool BlobToVector(BlobInterface *blob, std::vector<uint8> *vector_out);

#endif  // GEARS_BLOB_BLOB_UTILS_H_
