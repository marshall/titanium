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

#ifndef GLINT_INCLUDE_ANIMATION_TIMELINE_H__
#define GLINT_INCLUDE_ANIMATION_TIMELINE_H__

#include "glint/include/animations.h"
#include "glint/include/array.h"
#include "glint/include/base_object.h"
#include "glint/include/types.h"
#include "glint/include/point.h"

namespace glint {

class Node;

// Helper class describing a sequence of segments.
template <class SegmentType, class ValueType>
class AnimationSegments : public BaseObject {
 public:
  AnimationSegments()
    : active_segment_(0),
      playing_(false) {
  }

  bool Add(SegmentType* segment) {
    if (playing_)
      return false;
    return array_.Add(segment);
  }

  void Initialize(ValueType value_before_trigger, ValueType zero) {
    value_before_trigger_ = value_before_trigger;
    zero_ = zero;
  }

  bool Start(ValueType value_after_trigger,
             real64 start_time) {
    value_after_trigger_ = value_after_trigger;
    start_time_ = start_time;
    active_segment_ = 0;
    playing_ = (array_.length() >= 2);
    return playing_;
  }

  // Time is relative to the start of the containing timeline
  bool Advance(real64 current_time, ValueType* new_value) {
    if (!playing_ || array_.length() < 2)
      return false;

    if (active_segment_ == 0) {
      SegmentType* segment = array_[0];
      ValueType relative = GetRelativeValueForSegment(segment);
      current_ = segment->GetFinalValue(relative);
      *new_value = current_;
      active_segment_ = 1;
      return true;
    }

    bool has_active_segments = false;
    real64 time = start_time_;

    for (; active_segment_ < array_.length(); ++active_segment_) {
      SegmentType* segment = array_[active_segment_];
      ValueType relative = GetRelativeValueForSegment(segment);
      // the segment is in the past, accumulate time and value
      if (time + segment->duration() < current_time) {
        time += segment->duration();
        // Completed segment leaves the current value set to its final value.
        current_ = segment->GetFinalValue(relative);
        // start_time is a time when current active_segment started.
        // This is a time of the last completed segment.
        start_time_ = time;
      } else {
        // we found an active segment
        *new_value = segment->GetCurrentValue(current_,
                                            relative,
                                            current_time - time);
        has_active_segments = true;
        break;
      }
    }

    // If we are past all segments, then stop. Note that this method still
    // returns true indicating that the last value of animation should be
    // applied. It starts returning false on the next call.
    if (!has_active_segments) {
      playing_ = false;
      *new_value = current_;
    }

    return true;
  }

 private:
  ValueType GetRelativeValueForSegment(SegmentType* segment) {
    AnimationSegmentType type = segment->type();
    ValueType value;
    switch (type) {
      case ABSOLUTE_VALUE:    value = zero_; break;
      case RELATIVE_TO_START: value = value_before_trigger_; break;
      case RELATIVE_TO_FINAL: value = value_after_trigger_;  break;
      default:                value = zero_; break;
    }
    return value;
  }

  Array<SegmentType> array_;
  ValueType current_;
  ValueType value_before_trigger_;
  ValueType value_after_trigger_;
  ValueType zero_;
  real64 start_time_;
  int active_segment_;
  bool playing_;
};

// A timeline defines several Animations (or Animation segments) grouped
// together. For example, the timeline can contain several segments
// of Alpha animation and several segments of Transform animation. Overlapped
// in time segments are composed with each other - for example,
// TranslateAnimationSegment and ScaleAnimationSegment will transform into
// a single affine matrix.
// After timeline is created and stuffed with animation segments, it can be
// played on a node, using Node::PlayAnimation method or triggered by
// one of pre-defined events, like Show, Hide or Move - see
// Node::SetEventAnimation method for details on how to set this up.
// While playing, the timeline temporarily overrides alpha and effectively adds
// a temporary transform property to the affected node.
// Upon completion of the timeline, these temporary overrides disappear and
// the node becomes completely unaffected - so if the animation does not
// terminate at the 'natural' alpha or position of the node, the jump will
// occur.
// The timeline is immutable after it starts playing.
// If timeline is started using PlayAnimaiton, it is automatically deleted when
// it plays to the end.
// If the timeline is started using SetEventAnimation, it is owned by the Node
// and will be played every time the trigger even occurs.
// When another timeline starts on a node, it replaces the previous timeline
// if it is still playing - taking current values of animated properties
// from their current values.
// While playing, any changes to the timeline (adding animations or changing
// animation's properties) are illegal and will cause unpredictable results.
// Animation is 'IDLE' initially, after takign snapshot of pre-trigger values it
// becomes INITIALIZED and when actually started (on first call to Advance)
// it becomes PLAYING.
class AnimationTimeline : public BaseObject {
 public:
  AnimationTimeline();
  ~AnimationTimeline();

  enum State {
    IDLE,
    INITIALIZED,
    PLAYING,
  };

  bool AddAlphaSegment(AlphaAnimationSegment* segment);
  bool AddTranslationSegment(TranslationAnimationSegment* segment);
  // Fires GL_MSG_ANIMATION_COMPLETED with specified user_data in case the
  // animation completed its course and GL_MSG_ANIMATION_CANCELLED if it was
  // aborted or preempted by another one.
  bool RequestCompletionMessage(void* user_data);

  // Grabs pre-trigger values of animated properties, remembers the owner
  // and adds itself to the list of 'pending' animations to be started after
  // next layout cycle.
  void Initialize(Node* owner);

  // Advances the timeline to the specified time - typically current time.
  // Finds set of active animations, computes their values, composes values
  // into a single alpha and transform and then modifies the Node's properties.
  // If the current time is beyond the total timeline length, this will stop
  // the timeline and return 'false'.
  bool Advance(real64 current_time);

  // Finalizes the effect the animation has on its owner_ and posts
  // notification message about completion.
  void Cancel(bool completed);

  Node* owner() const { return owner_; }
  State state() const { return state_; }
  void* user_data() const { return user_data_; }
  Point animated_offset() { return animated_offset_; }
  int animated_alpha() { return animated_alpha_; }

 private:
  // Takes snapshot of "before trigger" values. 'owner_' is already set.
  void TakeInitialSnapshot();

  // Called before the animation is first time 'advanced'. Moves animation
  // from INITIALIZED state to PLAYING. Once started, the animation plays
  // out until it stops (goes to IDLE).
  bool Start(real64 start_time);

  AnimationSegments<AlphaAnimationSegment, int> alpha_segments_;
  AnimationSegments<TranslationAnimationSegment, Vector> translation_segments_;
  Node* owner_;        // the node which is playing this timeline now
  void* user_data_;
  State state_;
  Point animated_offset_;
  int animated_alpha_;
  bool send_message_;
};

}  // namespace glint

#endif  // GLINT_INCLUDE_ANIMATION_TIMELINE_H__
