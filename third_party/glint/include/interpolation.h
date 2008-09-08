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

#ifndef GLINT_INCLUDE_INTERPOLATION_H__
#define GLINT_INCLUDE_INTERPOLATION_H__

#include "glint/include/types.h"

namespace glint {

typedef real64 InterpolationFunction(real64 time);

// Static class defining various interpolation functions for using in
// animations. All functions take a value from 0 to 1 and produce a value
// from 0 to 1. The input parameter is usually a relative time of the animation
// step in progress, and the return value indicates the progress of animation.
class Interpolation {
 public:
  // See http://en.wikipedia.org/wiki/Cubic_Hermite_spline for the formula.
  // We use 0-1 interval and both tangents are 0 at the ends.
  static real64 Smooth(real64 time) {
    return time * time * (3 - 2 * time);
  }

  static real64 Linear(real64 time) {
    return time;
  }

 private:
  Interpolation() {  // Static class -> private ctor.
  }
};

}  // namespace glint

#endif  // GLINT_INCLUDE_INTERPOLATION_H__
