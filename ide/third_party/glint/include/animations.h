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

#ifndef GLINT_INCLUDE_ANIMATIONS_H__
#define GLINT_INCLUDE_ANIMATIONS_H__

#include "glint/include/base_object.h"
#include "glint/include/interpolation.h"
#include "glint/include/types.h"
#include "glint/include/point.h"

namespace glint {

enum AnimationSegmentType {
  // specifies absolute value
  ABSOLUTE_VALUE,
  // relative to current (snapshot of "property before trigger" for the
  // first frame and current animation value for the subsequent ones)
  RELATIVE_TO_START,
  // is relative to final ("property after trigger")
  RELATIVE_TO_FINAL,
};

class AnimationSegment : public BaseObject {
 public:
  AnimationSegment()
    : type_(ABSOLUTE_VALUE),
      duration_(0),
      interpolation_(Interpolation::Smooth) {
  }

  AnimationSegment(AnimationSegmentType type,
                   real64 duration,
                   InterpolationFunction* interpolation)
    : type_(type),
      duration_(duration),
      interpolation_(interpolation) {
  }

  // gets the interpolated time in the range 0..1
  real64 GetRelativeTime(real64 current_time) {
    real64 relative_time = duration_ > 0.0 ?
                           current_time / duration_ :
                           1.0;
    if (relative_time >= 1.0)
      relative_time = 1.0;
    else if (relative_time <= 0.0)
      relative_time = 0.0;

    return interpolation_(relative_time);
  }

  real64 duration() const { return duration_; }
  void set_duration(real64 value) { duration_ = value; }

  InterpolationFunction * interpolation() const { return interpolation_; }
  void set_interpolation(InterpolationFunction value) {
    interpolation_ = value;
  }

  AnimationSegmentType type() const { return type_;  }
  void set_type(AnimationSegmentType type) { type_ = type; }

 private:
  AnimationSegmentType type_;
  real64 duration_;  // in seconds
  InterpolationFunction *interpolation_;
};

class AlphaAnimationSegment : public AnimationSegment {
 public:
  AlphaAnimationSegment() : value_(0) {
  }

  AlphaAnimationSegment(AnimationSegmentType type,
                        int value,
                        real64 duration,
                        InterpolationFunction* interpolation)
    : AnimationSegment(type, duration, interpolation), value_(value) {
  }

  int GetFinalValue(int base) {
    return Clamp(base + value_);
  }

  // time is relative to the segment's start
  int GetCurrentValue(int start,
                      int base,
                      real64 current_time) {
    real64 time = GetRelativeTime(current_time);
//    real64 start = static_cast<real64>(start_value);
    real64 final = static_cast<real64>(GetFinalValue(base));
    real64 result = start + (final - start) * time;
    return Clamp(static_cast<int>(result));
  }

  int value() const { return value_; }
  void set_value(int value) {
    value_ = Clamp(value);
  }

 private:
  int Clamp(int value) {
    if (value < 0)
      return 0;
    else if (value > 255)
      return 255;
    else
      return value;
  }

  int value_;
};

class TranslationAnimationSegment : public AnimationSegment {
 public:
  Vector GetFinalValue(Vector base) {
    return Vector(base.x + value_.x,
                  base.y + value_.y);
  }

  Vector GetCurrentValue(Vector start, Vector base, real64 current_time) {
    real32 time = static_cast<real32>(GetRelativeTime(current_time));
    Vector final = GetFinalValue(base);
    return Vector(start.x + (final.x - start.x) * time,
                  start.y + (final.y - start.y) * time);
  }

  Vector value() const { return value_; }
  void set_value(Vector value) { value_ = value; }

 private:
  Vector value_;
};

}  // namespace glint

#endif  // GLINT_INCLUDE_ANIMATIONS_H__
