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

#include "glint/include/draw_stack.h"
#include "glint/crossplatform/core_util.h"
#include "glint/include/bitmap.h"

namespace glint {

void GetInitialClip(Bitmap* target, Rectangle* clip) {
  ASSERT(clip);
  if (target) {
    target->GetBounds(clip);
  } else {
    clip->SetHuge();
  }
}

DrawStack::DrawStack(Bitmap *target) {
  target_ = target;
  DrawContext *context = new DrawContext();
  if (!context)
    return;
  GetInitialClip(target, &context->clip);
  context->alpha = 255;
  array_.Add(context);
}

bool DrawStack::Push(const Transform &transform,
                     const Rectangle &clip,
                     bool reset_clip,
                     int alpha) {
  DrawContext *context = new DrawContext();
  if (!context)
    return false;

  if (array_.length() > 0)
    context->Set(*(array_[array_.length() - 1]));

  Transform inverse;
  inverse.Set(transform);
  inverse.Invert();
  context->transform_to_local.AddPostTransform(inverse);
  context->transform_to_global.AddPreTransform(transform);
  // Note - clip is local and when we apply TransformBounds,
  // this will expand the clip area in case of rotation, so the rotated content
  // will be clipped incorrectly.
  // One solution for that is to create a temporary bitmap
  // for a rotated subtree and compose the whole rotated subtree into
  // that bitmap. After that, pass both local and global clips to bilinear
  // blend procedure. Currently, Bitmap::TransformBilinear
  // used the whole size of the source bitmap - so if it will get "source clip",
  // it will be able to compare transformed pixels against it, effectively
  // affecting pixels that belong to intersection of source and target clips
  // without need to actually ever intersect non-rectangular clip regions.
  // Since typical nodes (bitmaps, text) use intermediate bitmaps anyways,
  // in most cases there is already an "temporary bitmap" pre-allocated.
  // TODO(dimich): add intermediate compositing surfaces if necessary.

  if (reset_clip) {
    GetInitialClip(target_, &context->clip);
  } else {
    Rectangle global_clip;
    context->transform_to_global.TransformRectangle(clip, &global_clip);
    context->clip.Intersect(global_clip);
  }

  context->alpha = (context->alpha * (alpha + 1)) >> 8;
  array_.Add(context);
  return true;
}

void DrawStack::Pop() {
  int last = array_.length() - 1;
  ASSERT(last > 0);
  // Note we always keep the last element in the array - since
  // it is pushed in construction time, it should not be ever removed.
  if (last > 0)
    array_.RemoveAt(last);
}
}  // namespace glint
