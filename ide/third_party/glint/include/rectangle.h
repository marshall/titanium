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

#ifndef GLINT_INCLUDE_RECTANGLE_H__
#define GLINT_INCLUDE_RECTANGLE_H__

#include "glint/include/base_object.h"
#include "glint/include/point.h"
#include "glint/include/size.h"

namespace glint {

// Rectangle object. Rectangle initially created 'empty', or 'nothing'.
// The only way to become non-empty for a rectangle is to be initialized
// with all 4 values (left, top, right, bottom). This can be done via
// explicit constructor which specifies them or via one of Set methods.
// Also, Union of empty rectangle with a non-empty rectangle results in
// initialization too (when you union nothing with a rectangle, you get a
// rectangle).
// Rectangle becomes 'empty' when it is constructed by default constructor
// without parameters, after Reset method or when Intersected with a
// non-overlapping rectangle. Also, setting coordinates by one of Set methods
// or via individual setters like set_left() in a way that makes left>=right
// or top>=bottom renders the rectangle 'empty'.
// Retrieving coordinates of 'empty' rectangle yields unpredictable values.
// An empty rectangle is equal to another empty rectangle and it does not
// contain any given point.
// Also, rectangle can be 'huge'. One can obtain a huge rectangle by using
// SetHuge() or using individual setters like set_left() with values exceeding
// some big range, or applying a vastly magnifying transform on an otherwise
// normal rectangle. Once huge, the rectangle stays huge until it's explicitly
// re-set to another state via Set, Reset or Intersect methods.
class Rectangle : public BaseObject {
 public:
  Rectangle() : left_(0), top_(0), right_(0), bottom_(0) {
  }

  Rectangle(int left, int top, int right, int bottom)
    : left_(left),
      top_(top),
      right_(right),
      bottom_(bottom) {
  }

  Rectangle(const Point& origin, const Size& size)
    : left_(origin.x),
      top_(origin.y),
      right_(origin.x + size.width),
      bottom_(origin.y + size.height) {
  }

  int left() const { return left_; }
  void set_left(int left) { left_ = left; }

  int top() const { return top_; }
  void set_top(int top) { top_ = top; }

  int right() const { return right_; }
  void set_right(int right) { right_ = right; }

  int bottom() const { return bottom_; }
  void set_bottom(int bottom) { bottom_ = bottom; }

  Point origin() const { return Point(left_, top_); }
  Size size() const { return Size(right_ - left_, bottom_ - top_); }

  bool IsEmpty() const;
  bool IsHuge() const;
  bool Contains(const Point& point) const;
  bool IsEqual(const Rectangle& another) const;

  void Set(const Rectangle& another);
  void Set(int left, int top, int right, int bottom);
  void Set(const Point& origin, const Size& size);
  void Reset();
  void Offset(const Point& offset);
  void SetHuge();

  void Intersect(const Rectangle& another);
  void Union(const Rectangle& another);

 private:
  int left_;
  int top_;
  int right_;
  int bottom_;
  DISALLOW_EVIL_CONSTRUCTORS(Rectangle);
};

}  // namespace glint

#endif  // GLINT_INCLUDE_RECTANGLE_H__
