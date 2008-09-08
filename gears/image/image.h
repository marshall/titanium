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

#ifndef GEARS_IMAGE_IMAGE_H__
#define GEARS_IMAGE_IMAGE_H__

#ifdef OFFICIAL_BUILD
// The Image API has not been finalized for official builds
#else

#include "gears/base/common/base_class.h"
#include "gears/base/common/common.h"
#include "gears/image/backing_image.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

class GearsImage : public ModuleImplBaseClass {
 public:
  static const std::string kModuleName;

  GearsImage() : ModuleImplBaseClass(kModuleName) {}

  // IN: int x, int y, int width, int height
  // OUT: void
  void Crop(JsCallContext *context);

  // IN: GearsImage image, int x, int y
  // OUT: void
  void DrawImage(JsCallContext *context);

  // IN: void
  // OUT: void
  void FlipHorizontal(JsCallContext *context);

  // IN: void
  // OUT: void
  void FlipVertical(JsCallContext *context);

  // IN: void
  // OUT: int
  void GetHeight(JsCallContext *context);

  // IN: void
  // OUT: int
  void GetWidth(JsCallContext *context);

  // IN: int width, int height
  // OUT: void
  void Resize(JsCallContext *context);

  // IN: int degrees
  // OUT: void
  void Rotate(JsCallContext *context);

  // IN: optional string type
  // OUT: GearsBlob blob
  void ToBlob(JsCallContext *context);

  void Init(BackingImage *image) {
    image_.reset(image);
  }

 private:
  scoped_ptr<BackingImage> image_;

  DISALLOW_EVIL_CONSTRUCTORS(GearsImage);
};

#endif  // not OFFICIAL_BUILD

#endif  // GEARS_IMAGE_IMAGE_H__
