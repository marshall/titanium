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

#ifndef GLINT_INCLUDE_POINT_H__
#define GLINT_INCLUDE_POINT_H__

#include "glint/include/types.h"

namespace glint {

// Simple Point struct
struct Point {
  Point() : x(0), y(0) {
  }
  Point(int initial_x, int initial_y) : x(initial_x), y(initial_y) {
  }

  bool operator==(const Point& p) { return (p.x == x && p.y == y); }
  bool operator!=(const Point& p) { return !(*this == p); }

  int x;
  int y;
};

struct Vector {
  Vector() : x(0), y(0) {}
  Vector(real32 initial_x, real32 initial_y) : x(initial_x), y(initial_y) {
  }
  Vector(int initial_x, int initial_y) : x(static_cast<real32>(initial_x)),
                                         y(static_cast<real32>(initial_y)) {
  }
  explicit Vector(Point point) : x(static_cast<real32>(point.x)),
                                 y(static_cast<real32>(point.y)) {
  }

  real32 x;
  real32 y;
};

}  // namespace glint

#endif  // GLINT_INCLUDE_POINT_H__
