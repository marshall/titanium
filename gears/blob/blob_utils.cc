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

#include "gears/blob/blob_utils.h"

#include "gears/base/common/basictypes.h"
#include "gears/base/common/string16.h"
#include "gears/blob/blob_interface.h"
#include "third_party/convert_utf/ConvertUTF.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

namespace {

class UTF8ToUTF16Reader : public BlobInterface::Reader {
 public:
  explicit UTF8ToUTF16Reader(std::string16 *out_string)
      : target_start_original_(reinterpret_cast<UTF16*>(&(*out_string)[0])),
        target_start_(target_start_original_),
        target_end_(target_start_ + out_string->size()),
        partial_pos_(0),
        result_(true) {
  }

  // NOTE: this will update source_start to skip over bytes that it used to
  // complete the partial read.  It will also update target_start_.
  bool ReadPartial(const UTF8 **source_start, const UTF8 *source_end) {
    int length(std::min(4, partial_pos_ + (source_end - *source_start)));
    memcpy(partial_ + partial_pos_, *source_start, length - partial_pos_);
    const UTF8 *start(partial_);
    const UTF8 *end(partial_ + length);
    ConversionResult result = ConvertUTF8toUTF16(&start, end,
                                                 &target_start_, target_end_,
                                                 strictConversion);
    if (result == sourceIllegal || result == targetExhausted) {
      return false;
    }
    // We handle both sourceExhausted and conversionOK results.
    // start will now point to the first non-converted byte.
    if (start == partial_) {
      // We still couldn't complete a character.  Leave partial_pos_ as-is.
      // Allow our caller to (re-)copy the characters to partial_.
      assert(length < 4);
    } else {
      // We've completed at least one character.  Adjust source_start to skip
      // over the converted bytes, and reset partial_pos_.
      int offset((start - partial_) - partial_pos_);
      assert(offset > 0 && offset < 4);
      *source_start += offset;
      partial_pos_ = 0;
    }
    return true;
  }

  virtual int64 ReadFromBuffer(const uint8 *buffer, int64 max_bytes) {
    assert(buffer && max_bytes >= 0);
    if (result_ == false) return 0;  // If we failed to convert, don't continue.
    const UTF8 *source_start_original(reinterpret_cast<const UTF8*>(buffer));
    const UTF8 *source_start(source_start_original);
    const UTF8 *source_end(source_start + max_bytes);
    // If we have saved bytes from the last call, see if they're complete now.
    if (partial_pos_ && !ReadPartial(&source_start, source_end)) {
      result_ = false;
      return 0;
    }
    ConversionResult result = ConvertUTF8toUTF16(&source_start, source_end,
                                                 &target_start_, target_end_,
                                                 strictConversion);
    if (result == targetExhausted ||
        (result == sourceIllegal && partial_pos_ == 0)) {
      result_ = false;
      return 0;
    }
    if (result == sourceIllegal) {
      // This happens when a character is split across 3 or more buffers, and
      // we're examining a middle buffer.  Treat as sourceExhausted and add
      // the characters to partial_.
      assert(partial_pos_ > 0);
      result = sourceExhausted;
    }
    if (result == sourceExhausted) {
      // Save extra bytes for next call.
      int length(source_end - source_start);
      assert(length < 4);
      memcpy(partial_ + partial_pos_, source_start, length);
      partial_pos_ += length;
      assert(partial_pos_ > 0 && partial_pos_ < 4);
    }
    // This function always reads all bytes upon success, even if we have a
    // partial character left over.
    return max_bytes;
  }

  size_t FinalSize() const { return target_start_ - target_start_original_; }
  bool Success() const { return result_ && (partial_pos_ == 0); }

 private:
  UTF16 *target_start_original_;
  UTF16 *target_start_;
  UTF16 *target_end_;
  UTF8 partial_[4];
  int partial_pos_;
  bool result_;
  DISALLOW_EVIL_CONSTRUCTORS(UTF8ToUTF16Reader);
};
}  // namespace

bool BlobToString16(BlobInterface* blob, const std::string16 &charset,
                    std::string16 *text) {
  assert(blob);
  int64 blob_length(blob->Length());
  assert(blob_length >= 0);
  assert(blob_length <= static_cast<int64>(kuint32max));
  if (blob_length == 0) {
    text->clear();
    return true;
  }
#if 0
  if (charset && !charset.empty()) {
    // TODO(michaeln): decode charset using nsICharsetConverterManager
    // for firefox and MLang for ie.

    // TODO(bgarcia): Safari used to do:
    // ConvertToString16FromCharset(utf8_text.get(), length, charset, text);
    // But this will need to be updated to use blob ReadDirect().

    // TODO(bgarcia): To fix firefox & ie, also need to update
    // localserver/<platform>/http_request_XX.cc, GetResponseCharset().
  }
#endif
  // If no charset is provided, assume UTF8.
  // A UTF-16 string has at most as many 'characters' as the equiv. UTF8 string.
  text->resize(static_cast<size_t>(blob_length));
  UTF8ToUTF16Reader reader(text);
  int64 length = blob->ReadDirect(&reader, 0, blob_length);
  if (!reader.Success()) {
    text->clear();
    return false;
  }
  text->resize(reader.FinalSize());
  assert(length == blob_length);
  return (length == blob_length);
}

bool BlobToString(BlobInterface *blob, std::string *string_out) {
  assert(blob);
  int64 blob_length(blob->Length());
  assert(blob_length >= 0);
  assert(string_out);
  assert(blob_length <= static_cast<int64>(string_out->max_size()));
  if (blob_length == 0) {
    string_out->clear();
    return true;
  }
  string_out->resize(static_cast<size_t>(blob_length));
  uint8 *destination(reinterpret_cast<uint8*>(&(*string_out)[0]));
  int64 length = blob->Read(destination, 0, blob_length);
  assert(length == blob_length);
  return (length == blob_length);
}

// Convert the blob's contents to a vector.
bool BlobToVector(BlobInterface *blob, std::vector<uint8> *vector_out) {
  assert(blob);
  int64 blob_length(blob->Length());
  assert(blob_length >= 0);
  assert(vector_out);
  assert(blob_length <= static_cast<int64>(vector_out->max_size()));
  if (blob_length == 0) {
    vector_out->clear();
    return true;
  }
  vector_out->resize(static_cast<std::vector<uint8>::size_type>(
      blob_length));
  int64 length = blob->Read(&((*vector_out)[0]), 0, blob_length);
  assert(length == blob_length);
  return (length == blob_length);
}
