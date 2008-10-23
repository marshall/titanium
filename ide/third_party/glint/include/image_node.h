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

#ifndef GLINT_INCLUDE_IMAGE_NODE_H__
#define GLINT_INCLUDE_IMAGE_NODE_H__

#include <string>
#include "glint/include/node.h"
#include "glint/include/size.h"

namespace glint {

class Bitmap;
class Rectangle;

class ImageNode : public Node {
 public:
  ImageNode();
  ~ImageNode();

  bool ReplaceImage(const std::string& file_name);

  void ReplaceBitmap(int width, int height, const void* data);

  Bitmap* bitmap() {
    return bitmap_;
  }

#ifdef GLINT_ENABLE_XML
  static BaseObject* CreateInstance() {
    return new ImageNode();
  }
  static SetPropertyResult SetSource(BaseObject* node,
                                     const std::string& value);
#endif  // GLINT_ENABLE_XML

 protected:
  virtual Size OnComputeRequiredSize(Size constraint);
  virtual Size OnSetLayoutBounds(Size reserved);
  virtual bool OnDraw(DrawStack* stack);
  virtual void OnComputeDrawingBounds(Rectangle* bounds);

 private:
  void ReleaseBitmap();
  Bitmap* bitmap_;
  DISALLOW_EVIL_CONSTRUCTORS(ImageNode);
};

}  // namespace glint

#endif  // GLINT_INCLUDE_IMAGE_NODE_H__
