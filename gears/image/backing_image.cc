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

#ifdef OFFICIAL_BUILD
// The Image API has not been finalized for official builds.
#else

#include <cstring>
#include <vector>
#include "gears/base/common/scoped_refptr.h"
#include "gears/base/common/string_utils.h"
#include "gears/base/common/string16.h"
#include "gears/blob/buffer_blob.h"
#include "gears/image/backing_image.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

struct BlobReader {
  gdIOCtx io_ctx;
  scoped_refptr<BlobInterface> blob;
  int64 pos;
};

struct BlobWriter {
  gdIOCtx io_ctx;
  std::vector<uint8> buffer;
};

// The resultant BlobReader satisfies libGD's gdIOCtx structure for reading
// from blob.
static BlobReader *NewBlobReader(BlobInterface *blob);

// The resultant BlobWriter satisfies libGD's gdIOCtx structure for writing.
static BlobWriter *NewBlobWriter();

// The following are functions suitable as members of a gdIOCtx which has an
// actual type of BlobReader.
// Reads and returns a single byte from the BlobReader ctx, or EOF.
int blobGetC(gdIOCtx *ctx);

// Copies wanted bytes from the BlobReader ctx into dest, returning the number
// of bytes actually read
int blobGetBuf(gdIOCtx *ctx, void *dest, int wanted);

// The following are functions suitable as members of a gdIOCtx which has an
// actual type of BlobWriter.
// Writes a single byte to the BlobWriter ctx.
void blobPutC(gdIOCtx *ctx, int byte);

// Copies wanted bytes from src onto the end of the BlobWriter ctx and returns
// the number of bytes actually written.
int blobPutBuf(gdIOCtx *ctx, const void *src, int wanted);

BlobReader *NewBlobReader(BlobInterface *blob) {
  assert(blob);
  BlobReader *reader = new BlobReader;
  reinterpret_cast<gdIOCtx*>(reader)->getC = blobGetC;
  reinterpret_cast<gdIOCtx*>(reader)->getBuf = blobGetBuf;
  reinterpret_cast<gdIOCtx*>(reader)->putC = NULL;
  reinterpret_cast<gdIOCtx*>(reader)->putBuf = NULL;
  reinterpret_cast<gdIOCtx*>(reader)->seek = NULL;
  reinterpret_cast<gdIOCtx*>(reader)->tell = NULL;
  reinterpret_cast<gdIOCtx*>(reader)->gd_free = NULL;
  reader->blob = blob;
  reader->pos = 0;
  return reader;
}

BlobWriter *NewBlobWriter() {
  BlobWriter *writer = new BlobWriter;
  reinterpret_cast<gdIOCtx*>(writer)->getC = NULL;
  reinterpret_cast<gdIOCtx*>(writer)->getBuf = NULL;
  reinterpret_cast<gdIOCtx*>(writer)->putC = blobPutC;
  reinterpret_cast<gdIOCtx*>(writer)->putBuf = blobPutBuf;
  reinterpret_cast<gdIOCtx*>(writer)->seek = NULL;
  reinterpret_cast<gdIOCtx*>(writer)->tell = NULL;
  reinterpret_cast<gdIOCtx*>(writer)->gd_free = NULL;
  return writer;
}

// Returns (via the out parameter) an immutable BlobInterface made from this
// BlobWriter's contents.
// After calling this function, blob_writer should no longer be used.
void BlobWriterToBlob(BlobWriter *blob_writer,
                      scoped_refptr<BlobInterface> *out) {
  scoped_refptr<BufferBlob> blob(new BufferBlob(&(blob_writer->buffer)));
  *out = blob;
}

int blobGetC(gdIOCtx *ctx) {
  BlobReader *blob_reader = reinterpret_cast<BlobReader*>(ctx);
  if (blob_reader->pos >= blob_reader->blob->Length()) {
    return EOF;
  }
  uint8 c;
  blob_reader->blob->Read(&c, blob_reader->pos++, 1);
  return c;
}

int blobGetBuf(gdIOCtx *ctx, void *dest, int wanted) {
  BlobReader *blob_reader = reinterpret_cast<BlobReader*>(ctx);
  int64 bytes_read = blob_reader->blob->Read(reinterpret_cast<uint8*>(dest),
                                             blob_reader->pos,
                                             wanted);
  blob_reader->pos += bytes_read;
  return static_cast<int>(bytes_read);
}

void blobPutC(gdIOCtx *ctx, int byte) {
  BlobWriter *blob_writer = reinterpret_cast<BlobWriter*>(ctx);
  blob_writer->buffer.push_back(static_cast<uint8>(byte));
}

int blobPutBuf(gdIOCtx *ctx, const void *src, int wanted) {
  BlobWriter *blob_writer = reinterpret_cast<BlobWriter*>(ctx);
  const char *begin = static_cast<const char*>(src);
  const char *end = begin + wanted;
  blob_writer->buffer.insert(blob_writer->buffer.end(), begin, end);
  return wanted;
}

// Start Image class implementation

BackingImage::BackingImage() : img_ptr_(NULL) {}

bool BackingImage::Init(BlobInterface *blob, std::string16 *error) {
  const int kHeaderSize = 4;  // At least the length of the longest header below
  const char *kPngHeader = "\x89PNG";
  const char *kGifHeader = "GIF8";
  const char *kJpegHeader = "\xFF\xD8\xFF";

  if (img_ptr_) {
    gdFree(img_ptr_);
    img_ptr_ = NULL;
  }

  // Sniff the first four bytes and load the image
  uint8 header[kHeaderSize];
  if (blob->Read(header, 0, kHeaderSize) != kHeaderSize) {
    *error = STRING16(L"The blob is not an image");
    return false;
  }
  if (memcmp(header, kPngHeader, strlen(kPngHeader)) == 0) {  // PNG
    BlobReader *blob_reader = NewBlobReader(blob);
    img_ptr_ = gdImageCreateFromPngCtx(reinterpret_cast<gdIOCtx*>(blob_reader));
    delete blob_reader;
    if (!img_ptr_) {
      *error = STRING16(L"Failed to open the blob as a PNG");
      return false;
    }
    gdImageSaveAlpha(img_ptr_, 1);
    original_format_ = FORMAT_PNG;
  } else if (memcmp(header, kGifHeader, strlen(kGifHeader)) == 0) {  // GIF
    BlobReader *blob_reader = NewBlobReader(blob);
    gdImagePtr tmp_img = gdImageCreateFromGifCtx(
        reinterpret_cast<gdIOCtx*>(blob_reader));
    delete blob_reader;
    if (!tmp_img) {
      *error = STRING16(L"Failed to open the blob as a GIF");
      return false;
    }

    // Because GIF is palette coloured, we need to make a truecolor version
    // by creating a blank black image and copying it in:
    img_ptr_ = gdImageCreateTrueColor(gdImageSX(tmp_img), gdImageSY(tmp_img));
    if (!img_ptr_) {
      gdFree(tmp_img);
      *error = GET_INTERNAL_ERROR_MESSAGE();
      return false;
    }
    gdImageSaveAlpha(img_ptr_, 1);
    gdImageAlphaBlending(img_ptr_, 0);
    gdImageCopy(img_ptr_, tmp_img, 0, 0, 0, 0,
                gdImageSX(tmp_img), gdImageSY(tmp_img));

    gdFree(tmp_img);
    original_format_ = FORMAT_GIF;
  } else if (memcmp(header, kJpegHeader, strlen(kJpegHeader)) == 0) {  // JPEG
    BlobReader *blob_reader = NewBlobReader(blob);
    img_ptr_ = gdImageCreateFromJpegCtx(
        reinterpret_cast<gdIOCtx*>(blob_reader));
    delete blob_reader;
    if (!img_ptr_) {
      *error = STRING16(L"Failed to open the blob as a JPEG");
      return false;
    }
    original_format_ = FORMAT_JPEG;
  } else {
    *error = STRING16(L"Unsupported image format");
    return false;
  }

  return true;
}

bool BackingImage::Resize(int width, int height, std::string16 *error) {
  assert(img_ptr_);
  if (width <= 0 || height <= 0) {
    *error = STRING16(L"width and height must not be negative");
    return false;
  }
  if (width > kMaxImageDimension || height > kMaxImageDimension) {
    *error = std::string16(STRING16(L"width and height must be less than ")) +
        IntegerToString16(kMaxImageDimension);
    return false;
  }
  gdImagePtr new_img = gdImageCreateTrueColor(width, height);
  gdImageSaveAlpha(new_img, 1);
  gdImageAlphaBlending(new_img, 0);
  gdImageCopyResampled(new_img, img_ptr_, 0, 0, 0, 0,
                     width, height, gdImageSX(img_ptr_), gdImageSY(img_ptr_));
  gdImageDestroy(img_ptr_);
  img_ptr_ = new_img;
  return true;
}

bool BackingImage::Crop(int x, int y, int width, int height, std::string16 *error) {
  assert(img_ptr_);
  if (width <= 0 || height <= 0 ||
      x < 0 || y < 0 ||
      x + width > gdImageSX(img_ptr_) ||
      y + height > gdImageSY(img_ptr_)) {
    *error = STRING16(L"The bounding box must be within the original boundary");
    return false;
  }
  gdImagePtr new_img = gdImageCreateTrueColor(width, height);
  gdImageSaveAlpha(new_img, 1);
  gdImageAlphaBlending(new_img, 0);
  gdImageCopy(new_img, img_ptr_, 0, 0, x, y, width, height);
  gdImageDestroy(img_ptr_);
  img_ptr_ = new_img;
  return true;
}

bool BackingImage::Rotate(int degrees, std::string16 *error) {
  assert(img_ptr_);
  if (degrees % 90 != 0) {
    *error = STRING16(L"Rotations must be orthogonal");
    return false;
  }
  int width;
  int height;
  if (degrees % 180 == 0) {
    width = gdImageSX(img_ptr_);
    height = gdImageSY(img_ptr_);
  } else {
    width = gdImageSY(img_ptr_);
    height = gdImageSX(img_ptr_);
  }
  gdImagePtr new_img = gdImageCreateTrueColor(width, height);
  gdImageSaveAlpha(new_img, 1);
  gdImageAlphaBlending(new_img, 0);
  gdImageCopyRotated(new_img,
                     img_ptr_,
                     width * 0.5, height * 0.5,  // destination center
                     0, 0,                       // source top-left
                     gdImageSX(img_ptr_), gdImageSY(img_ptr_),
                     degrees % 360);
  gdImageDestroy(img_ptr_);
  img_ptr_ = new_img;
  return true;
}

bool BackingImage::DrawImage(const BackingImage *img, int x, int y,
                             std::string16 *error) {
  assert(img_ptr_);
  if (img == this) {
    *error = STRING16(L"Cannot draw an image onto itself");
    return false;
  }
  gdImageAlphaBlending(img_ptr_, 1);
  gdImageCopy(img_ptr_, img->img_ptr_, x, y, 0, 0,
              gdImageSX(img->img_ptr_), gdImageSY(img->img_ptr_));
  return true;
}

bool BackingImage::FlipHorizontal(std::string16 *error) {
  assert(img_ptr_);
  int width = gdImageSX(img_ptr_);
  int height = gdImageSY(img_ptr_);
  gdImagePtr new_img = gdImageCreateTrueColor(width, height);
  if (!new_img) {
    *error = GET_INTERNAL_ERROR_MESSAGE();
    return false;
  }
  gdImageSaveAlpha(new_img, 1);
  gdImageAlphaBlending(new_img, 0);
  for (int x = 0; x < width; x++) {
    gdImageCopy(new_img, img_ptr_, x, 0, width - x - 1, 0, 1, height);
  }
  gdImageDestroy(img_ptr_);
  img_ptr_ = new_img;
  return true;
}

bool BackingImage::FlipVertical(std::string16 *error) {
  assert(img_ptr_);
  int width = gdImageSX(img_ptr_);
  int height = gdImageSY(img_ptr_);
  gdImagePtr new_img = gdImageCreateTrueColor(width, height);
  if (!new_img) {
    *error = GET_INTERNAL_ERROR_MESSAGE();
    return false;
  }
  gdImageSaveAlpha(new_img, 1);
  gdImageAlphaBlending(new_img, 0);
  for (int y = 0; y < height; y++) {
    gdImageCopy(new_img, img_ptr_, 0, y, 0, height - y - 1, width, 1);
  }
  gdImageDestroy(img_ptr_);
  img_ptr_ = new_img;
  return true;
}

int BackingImage::Width() const {
  return gdImageSX(img_ptr_);
}

int BackingImage::Height() const {
  return gdImageSY(img_ptr_);
}

void BackingImage::ToBlob(scoped_refptr<BlobInterface> *out,
                          ImageFormat format) {
  assert(img_ptr_);
  scoped_ptr<BlobWriter> blob_writer(NewBlobWriter());
  if (format == FORMAT_PNG) {
    gdImagePngCtx(img_ptr_, reinterpret_cast<gdIOCtx*>(blob_writer.get()));
  } else if (format == FORMAT_GIF) {
    gdImageGifCtx(img_ptr_, reinterpret_cast<gdIOCtx*>(blob_writer.get()));
  } else if (format == FORMAT_JPEG) {
    gdImageJpegCtx(img_ptr_, reinterpret_cast<gdIOCtx*>(blob_writer.get()),
                   kJpegQuality);
  } else {
    assert(false);
  }
  BlobWriterToBlob(blob_writer.get(), out);
}

void BackingImage::ToBlob(scoped_refptr<BlobInterface> *out) {
  return ToBlob(out, original_format_);
}

BackingImage::~BackingImage() {
  if (img_ptr_) {
    gdImageDestroy(img_ptr_);
  }
  img_ptr_ = NULL;
}

#endif  // not OFFICIAL_BUILD
