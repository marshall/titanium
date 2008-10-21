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

// Implementation of bitmap related parts of Platform interface for Darwin

#import <assert.h>
#import <Cocoa/Cocoa.h>

#import "glint/include/bitmap.h"
#import "glint/mac/darwin_platform.h"

using namespace glint;

namespace glint_darwin {

static const int kBitsPerComponent = 8;
static const int kComponentsPerPixel = sizeof(Color);
static const int kBitsPerPixel =
    kBitsPerComponent * kComponentsPerPixel;

// Creates platform-specific bitmap, in a platform-dependent format
// We need alpha to be the most significant byte.
// TODO(pankaj): write unit tests, test on Leopard, and on PPC (big endian)
PlatformBitmap* DarwinPlatform::CreateBitmap(int width, int height) {
  size_t row_bytes = ((width * kBitsPerPixel) + (kBitsPerComponent - 1)) / 8;

  // We want alpha to be the MSB on all platforms. This means it should be the
  // first byte of a pixel on big endian systems, and the last byte on little
  // endian.
  int format = 0;
#if defined(__BIG_ENDIAN__)
  format |= NSAlphaFirstBitmapFormat;
#endif

  NSBitmapImageRep* ns_bitmap =
      [[NSBitmapImageRep alloc]
            initWithBitmapDataPlanes:NULL
                          pixelsWide:width
                          pixelsHigh:height
                       bitsPerSample:kBitsPerComponent
                     samplesPerPixel:kComponentsPerPixel
                            hasAlpha:YES
                            isPlanar:NO
                      colorSpaceName:NSCalibratedRGBColorSpace
                        bitmapFormat:(NSBitmapFormat)format
                         bytesPerRow:row_bytes
                        bitsPerPixel:kBitsPerPixel];

  if (ns_bitmap) {
    assert(all_bitmaps_);
    assert([all_bitmaps_ containsObject:ns_bitmap] == NO);
    [all_bitmaps_ addObject:ns_bitmap];
  }

  return reinterpret_cast<PlatformBitmap*>(ns_bitmap);
}

Color* DarwinPlatform::LockPixels(PlatformBitmap *bitmap) {
  NSBitmapImageRep* ns_bitmap = reinterpret_cast<NSBitmapImageRep*>(bitmap);
  if (!ns_bitmap || [all_bitmaps_ containsObject:ns_bitmap] == NO) {
    assert(false && "Invalid platform bitmap");
    return NULL;
  }

  Color* pixels = reinterpret_cast<Color*>([ns_bitmap bitmapData]);

  return pixels;
}

void DarwinPlatform::UnlockPixels(PlatformBitmap *bitmap, Color *pixels) {
  // Noop
}

void DarwinPlatform::DeleteBitmap(PlatformBitmap *bitmap) {
  NSBitmapImageRep* ns_bitmap = reinterpret_cast<NSBitmapImageRep*>(bitmap);
  if (!ns_bitmap || [all_bitmaps_ containsObject:ns_bitmap] == NO) {
    assert(false && "Invalid platform bitmap");
    return;
  }

  [all_bitmaps_ removeObject:ns_bitmap];

  [ns_bitmap release];
}

bool DarwinPlatform::LoadBitmapFromFile(const std::string &file_name,
                                        Bitmap **bitmap) {
  if (!bitmap) {
    return false;
  }

  NSString* file_name_str = [NSString stringWithUTF8String:file_name.c_str()];
  NSImage* image = [[[NSImage alloc] initWithContentsOfFile:file_name_str]
                     autorelease];

  if (!image) {
    // Wasn't a proper image file
    return false;
  }

  NSSize image_size = [image size];
  int width = image_size.width;
  int height = image_size.height;

  *bitmap = new Bitmap(Point(), Size(width, height));

  // bitmap->platform_bitmap() should've been created using CreateBitmap above
  NSBitmapImageRep* ns_bitmap =
      reinterpret_cast<NSBitmapImageRep*>((*bitmap)->platform_bitmap());
  if (!ns_bitmap || [all_bitmaps_ containsObject:ns_bitmap] == NO) {
    assert(false && "Invalid platform bitmap");
    return false;
  }

  AutoRestoreGraphicsContext ar(ns_bitmap);

  [image drawAtPoint:NSZeroPoint
            fromRect:NSZeroRect  // NSZeroRect => draw entire image.
           operation:NSCompositeCopy
            fraction:1.0];

  return true;
}

}  // namespace glint_darwin
