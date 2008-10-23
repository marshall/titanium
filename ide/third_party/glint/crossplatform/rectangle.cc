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

#include "glint/include/rectangle.h"

namespace glint {

const int kHugeBoundary = 0x70000000;

bool Rectangle::IsEmpty() const {
  return left_ >= right_ || top_ >= bottom_;
}

bool Rectangle::IsHuge() const {
  // Note: '||' means that as soon as one boundary becomes huge the whole
  // rectangle is considered 'huge'. Technically, we could replace those with
  // '&&' and have slightly different meaning.
  return !IsEmpty() &&
         (left_ <= -kHugeBoundary ||
          top_ <= -kHugeBoundary  ||
          right_ >= kHugeBoundary ||
          bottom_ >= kHugeBoundary);
}

bool Rectangle::Contains(const Point& point) const {
  return IsHuge() ||
         (!IsEmpty() &&
          point.x >= left_ &&
          point.x < right_ &&
          point.y >= top_ &&
          point.y < bottom_);
}

bool Rectangle::IsEqual(const Rectangle& another) const {
  return (IsEmpty() && another.IsEmpty()) ||
         (left_ == another.left_ &&
          top_ == another.top_ &&
          right_ == another.right_ &&
          bottom_ == another.bottom_) ||
         (IsHuge() && another.IsHuge());
}

void Rectangle::Set(const Rectangle& another) {
  left_ = another.left_;
  top_ = another.top_;
  right_ = another.right_;
  bottom_ = another.bottom_;
}

void Rectangle::Set(int left, int top, int right, int bottom) {
  left_ = left;
  top_ = top;
  right_ = right;
  bottom_ = bottom;
}

void Rectangle::Set(const Point& origin, const Size& size) {
  left_ = origin.x;
  top_ = origin.y;
  right_ = origin.x + size.width;
  bottom_ = origin.y + size.height;
}

void Rectangle::Reset() {
  Rectangle empty;
  Set(empty);
}

void Rectangle::Offset(const Point& offset) {
  if (IsHuge())
    return;

  left_ += offset.x;
  top_ += offset.y;
  right_ += offset.x;
  bottom_ += offset.y;
}

void Rectangle::SetHuge() {
  Set(-kHugeBoundary, -kHugeBoundary, kHugeBoundary, kHugeBoundary);
}

void Rectangle::Intersect(const Rectangle& another) {
  if (IsEmpty())
    return;

  if (left_ < another.left_) {
    left_ = another.left_;
  }

  if (top_ < another.top_) {
    top_ = another.top_;
  }

  if (right_ > another.right_) {
    right_ = another.right_;
  }

  if (bottom_ > another.bottom_) {
    bottom_ = another.bottom_;
  }
}

void Rectangle::Union(const Rectangle& another) {
  if (another.IsEmpty())
    return;

  if (IsEmpty()) {
    Set(another);
    return;
  }

  // Both rectangles are not empty, compute the union.
  if (left_ > another.left_) {
    left_ = another.left_;
  }

  if (top_ > another.top_) {
    top_ = another.top_;
  }

  if (right_ < another.right_) {
    right_ = another.right_;
  }

  if (bottom_ < another.bottom_) {
    bottom_ = another.bottom_;
  }
}

}  // namespace glint

