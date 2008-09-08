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

#include "glint/include/image_node.h"
#include "glint/crossplatform/core_util.h"
#include "glint/include/bitmap.h"
#include "glint/include/draw_stack.h"

namespace glint {

#ifdef DEBUG
static const char* kImageNodeName = "ImageNode";
#endif

ImageNode::ImageNode() : bitmap_(NULL) {
#ifdef DEBUG
  type_name_ = kImageNodeName;
#endif
}

ImageNode::~ImageNode() {
  ReleaseBitmap();
}

void ImageNode::ReleaseBitmap() {
  delete bitmap_;
  bitmap_ = NULL;
}

bool ImageNode::ReplaceImage(const std::string& file_name) {
  Bitmap* new_bitmap = NULL;
  if (!platform()->LoadBitmapFromFile(file_name, &new_bitmap))
    return false;

  ReleaseBitmap();
  bitmap_ = new_bitmap;

  Invalidate();
  return true;
}

void ImageNode::ReplaceBitmap(int width, int height, const void* data) {
  ASSERT(width >= 0 && height >= 0);
  ASSERT(width == 0 || height == 0 || data);

  ReleaseBitmap();
  bitmap_ = new Bitmap(Point(), Size(width, height));
  if (width > 0 && height > 0) {
    memcpy(bitmap_->GetPixelAt(0, 0), data, width * height * 4);
  }
  bitmap_->PremultiplyAlpha();
  Invalidate();
}

Size ImageNode::OnComputeRequiredSize(Size constraint) {
  // No bitmap was loaded into this ImageNode.
  // Make sure there is at least something.
  if (!bitmap_) {
    bitmap_ = new Bitmap(Point(), Size());
  }
  Size image_size = bitmap_->size();
  return image_size;
}

Size ImageNode::OnSetLayoutBounds(Size reserved) {
  Size image_size = bitmap_->size();
  return image_size;
}

void ImageNode::OnComputeDrawingBounds(Rectangle* bounds) {
  ASSERT(bounds);
  bounds->Set(Point(), bitmap_->size());
}

bool ImageNode::OnDraw(DrawStack *stack) {
//  Rectangle clip = stack.clip();
  ASSERT(stack && stack->target() && stack->Top());
  if (!stack || !stack->target() || !stack->Top())
    return false;

  Bitmap *target = stack->target();
  DrawContext *draw_context = stack->Top();
  Rectangle source_clip(bitmap_->origin(), bitmap_->size());

  target->Compose(*bitmap_,
                  source_clip,
                  draw_context->transform_to_global,
                  draw_context->clip,
                  draw_context->alpha);

  return true;
}

#ifdef GLINT_ENABLE_XML
SetPropertyResult ImageNode::SetSource(BaseObject *node,
                                       const std::string& value) {
  ASSERT(node);
  if (!static_cast<ImageNode*>(node)->ReplaceImage(value)) {
    return PROPERTY_NOT_SET;
  }
  return PROPERTY_OK;
}
#endif  // GLINT_ENABLE_XML

}  // namespace glint

