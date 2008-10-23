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

#include <math.h>
#include "glint/include/transform.h"
#include "glint/crossplatform/core_util.h"
#include "glint/include/point.h"
#include "glint/include/rectangle.h"

namespace glint {

void Transform::AddOffset(const Vector& offset) {
  dx_ += offset.x;
  dy_ += offset.y;
}

void Transform::AddScale(const Vector& scale) {
  Transform scale_transform(scale.x, 0, 0, scale.y, 0, 0);
  AddPostTransform(scale_transform);
}

void Transform::AddRotation(real32 angle_radians) {
  real32 cosinus = cos(angle_radians);
  real32 sinus = sin(angle_radians);
  Transform rotation(cosinus, -sinus, sinus, cosinus, 0, 0);
  AddPostTransform(rotation);
}

void Transform::AddPreTransform(const Transform& pre_transform) {
  real32 m00 = m00_ * pre_transform.m00_ + m01_ * pre_transform.m10_;
  real32 m01 = m00_ * pre_transform.m01_ + m01_ * pre_transform.m11_;
  real32 m10 = m10_ * pre_transform.m00_ + m11_ * pre_transform.m10_;
  real32 m11 = m10_ * pre_transform.m01_ + m11_ * pre_transform.m11_;
  real32 dx = m00_ * pre_transform.dx_ + m01_ * pre_transform.dy_ + dx_;
  real32 dy = m10_ * pre_transform.dx_ + m11_ * pre_transform.dy_ + dy_;
  m00_ = m00;
  m01_ = m01;
  m10_ = m10;
  m11_ = m11;
  dx_ = dx;
  dy_ = dy;
}

void Transform::AddPostTransform(const Transform& post_transform) {
  real32 m00 = post_transform.m00_ * m00_ + post_transform.m01_ * m10_;
  real32 m01 = post_transform.m00_ * m01_ + post_transform.m01_ * m11_;
  real32 m10 = post_transform.m10_ * m00_ + post_transform.m11_ * m10_;
  real32 m11 = post_transform.m10_ * m01_ + post_transform.m11_ * m11_;
  real32 dx = post_transform.m00_ * dx_ + post_transform.m01_ * dy_ +
              post_transform.dx_;
  real32 dy = post_transform.m10_ * dx_ + post_transform.m11_ * dy_ +
              post_transform.dy_;
  m00_ = m00;
  m01_ = m01;
  m10_ = m10;
  m11_ = m11;
  dx_ = dx;
  dy_ = dy;
}

void Transform::Invert() {
  real32 determinant = m00_ * m11_ - m01_ * m10_;
  if (determinant == 0.0)
    return;  // No inverse matrix.

  real32 inv_det = 1.0f / determinant;

  // Get inverted 2x2 matrix
  real32 m00 = m11_ * inv_det;
  real32 m01 = -m01_ * inv_det;
  real32 m10 = -m10_ * inv_det;
  real32 m11 = m00_ * inv_det;

  // Move in opposite direction and then transform by inverted 2x2 amtrix.
  real32 dx = m00 * -dx_ + m01 * -dy_;
  real32 dy = m10 * -dx_ + m11 * -dy_;

  m00_ = m00;
  m01_ = m01;
  m10_ = m10;
  m11_ = m11;
  dx_ = dx;
  dy_ = dy;
}

void Transform::Set(const Transform& another) {
  m00_ = another.m00_;
  m01_ = another.m01_;
  m10_ = another.m10_;
  m11_ = another.m11_;
  dx_ = another.dx_;
  dy_ = another.dy_;
}

void Transform::Reset() {
  Transform identity;
  Set(identity);
}

Vector Transform::TransformVector(const Vector& input) const {
  Vector result;
  result.x = m00_ * input.x + m01_ * input.y + dx_;
  result.y = m10_ * input.x + m11_ * input.y + dy_;
  return result;
}

void Transform::TransformRectangle(const Rectangle& input,
                                   Rectangle* result) const {
  ASSERT(result);

  if (input.IsEmpty()) {
    result->Reset();
    return;
  }

  if (input.IsHuge()) {
    result->SetHuge();
    return;
  }

  // Transformation of 4 vertices gives, in general, 4 random points.
  // We choose min and max in each direction to produce bounds.
  Vector v1 = TransformVector(Vector(Point(input.left(), input.top())));
  Vector v2 = TransformVector(Vector(Point(input.right(), input.top())));
  Vector v3 = TransformVector(Vector(Point(input.left(), input.bottom())));
  Vector v4 = TransformVector(Vector(Point(input.right(), input.bottom())));

  int x1 = Round(v1.x);
  int x2 = Round(v2.x);
  int x3 = Round(v3.x);
  int x4 = Round(v4.x);

  result->set_left(min<int>(min<int>(min<int>(x1, x2), x3), x4));
  result->set_right(max<int>(max<int>(max<int>(x1, x2), x3), x4));

  int y1 = Round(v1.y);
  int y2 = Round(v2.y);
  int y3 = Round(v3.y);
  int y4 = Round(v4.y);

  result->set_top(min<int>(min<int>(min<int>(y1, y2), y3), y4));
  result->set_bottom(max<int>(max<int>(max<int>(y1, y2), y3), y4));
}

bool Transform::IsOffsetOnly() const {
  return Close(m00_, 1) && Close(m01_, 0) && Close(m10_, 0) && Close(m11_, 1);
}

Vector Transform::GetOffset() const {
  return Vector(dx_, dy_);
}

bool Transform::IsClose(const Transform& another) const {
  return Close(m00_, another.m00_) &&
         Close(m01_, another.m01_) &&
         Close(m10_, another.m10_) &&
         Close(m11_, another.m11_) &&
         Close(dx_, another.dx_) &&
         Close(dy_, another.dy_);
}

}  // namespace glint

