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

#ifndef GLINT_INCLUDE_TRANSFORM_H__
#define GLINT_INCLUDE_TRANSFORM_H__

#include "glint/include/types.h"

namespace glint {

class Rectangle;
struct Vector;

class Transform {
 public:
  Transform() : m00_(1), m01_(0), m10_(0), m11_(1), dx_(0), dy_(0) {
  }

  Transform(real32 m00,
            real32 m01,
            real32 m10,
            real32 m11,
            real32 dx,
            real32 dy)
    : m00_(m00),
      m01_(m01),
      m10_(m10),
      m11_(m11),
      dx_(dx),
      dy_(dy) {
  }

  // These functions add simple post-transforms.
  void AddOffset(const Vector& offset);
  void AddScale(const Vector& scale);
  void AddRotation(real32 angle_radians);

  void AddPostTransform(const Transform& post_transform);
  void AddPreTransform(const Transform& pre_transform);
  void Invert();
  // Sets the transform to identity.
  void Reset();
  void Set(const Transform& another);

  Vector TransformVector(const Vector& input) const;

  // Returns bounds of transformed rectangle
  void TransformRectangle(const Rectangle& input, Rectangle* result) const;

  // No scale, rotation or shear - (1, 0, 0, 1, dx dy)
  bool IsOffsetOnly() const;
  Vector GetOffset() const;

  bool IsClose(const Transform& another) const;

 private:
  real32 m00_, m01_, m10_, m11_, dx_, dy_;
  DISALLOW_EVIL_CONSTRUCTORS(Transform);
};

}  // namespace glint

#endif  // GLINT_INCLUDE_TRANSFORM_H__

