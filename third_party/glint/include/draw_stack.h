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

// DrawStack is a stack of DrawContexts - when drawing, Glint
// walks the tree of Nodes and pushes their transforms/clips to
// the drawing stack. So the top of the stack is always an
// accumulated drawing context, translating global coordinate
// system of screen into local coordinate system of the Node.

#ifndef GLINT_INCLUDE_DRAW_STACK_H__
#define GLINT_INCLUDE_DRAW_STACK_H__

#include "glint/include/array.h"
#include "glint/include/base_object.h"
#include "glint/include/rectangle.h"
#include "glint/include/transform.h"

namespace glint {

class Bitmap;

// This is an element of DrawStack. Members represent the accumulated values.
class DrawContext : public BaseObject {
 public:
  DrawContext() : alpha(255) {}
  void Set(const DrawContext& another) {
    transform_to_local.Set(another.transform_to_local);
    transform_to_global.Set(another.transform_to_global);
    alpha = another.alpha;
    clip.Set(another.clip);
  }

  Transform transform_to_local;
  Transform transform_to_global;
  int alpha;
  Rectangle clip;
 private:
  DISALLOW_EVIL_CONSTRUCTORS(DrawContext);
};

class DrawStack : public BaseObject {
 public:
  // DrawStack does not take ownership of the target bitmap.
  // Target bitmap can be NULL if we only use transform/clip stack - for
  // example, when simulating drawing while computing drawing bounds.
  explicit DrawStack(Bitmap *target);

  // 'remove_clip' removes accumulated clip from the stack so the display
  // subtree can render anywhere.
  bool Push(const Transform &transform,
            const Rectangle &clip,
            bool remove_clip,
            int alpha);
  void Pop();

  DrawContext *Top() {
    return array_[array_.length() - 1];
  }

  Bitmap *target() {
    return target_;
  }

 private:
  Array<DrawContext> array_;
  Bitmap *target_;
  DISALLOW_EVIL_CONSTRUCTORS(DrawStack);
};


}  // namespace glint

#endif  // GLINT_INCLUDE_DRAW_STACK_H__
