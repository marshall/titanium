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

#include "glint/include/animation_timeline.h"
#include "glint/crossplatform/core_util.h"
#include "glint/include/message.h"
#include "glint/include/node.h"
#include "glint/include/root_ui.h"
#include "glint/include/work_item.h"

namespace glint {

class AnimationMessageWorkItem : public WorkItem {
 public:
  AnimationMessageWorkItem(Node *node, void *user_data, bool completed)
    : node_(node),
      user_data_(user_data),
      completed_(completed) {
  }

  virtual void Run() {
    if (!node_)
      return;
    Message message;
    message.code = completed_ ? GL_MSG_ANIMATION_COMPLETED :
                                GL_MSG_ANIMATION_ABORTED;
    message.user_data = user_data_;
    node_->HandleMessage(message);
  }

 private:
  Node *node_;
  void *user_data_;
  bool completed_;
  DISALLOW_EVIL_CONSTRUCTORS(AnimationMessageWorkItem);
};

AnimationTimeline::AnimationTimeline()
  : owner_(NULL),
    user_data_(NULL),
    state_(IDLE),
    animated_alpha_(-1),
    send_message_(false) {
}

AnimationTimeline::~AnimationTimeline() {
}

bool AnimationTimeline::AddAlphaSegment(
    AlphaAnimationSegment *segment) {
  if (owner_)
    return false;
  return alpha_segments_.Add(segment);
}

bool AnimationTimeline::AddTranslationSegment(
    TranslationAnimationSegment *segment) {
  if (owner_)
    return false;
  return translation_segments_.Add(segment);
}

// First step of starting an animation - grab "before trigger" values.
void AnimationTimeline::Initialize(Node* owner) {
  ASSERT(owner);
  owner_ = owner;

  // Snapshot "current", or "before trigger" values - they are a sum of
  // layout and animation offsets, momentous values, perhaps under
  // control of another or the same animation timeline.
  alpha_segments_.Initialize(owner_->effective_alpha(), 0);
  Point offset = owner_->effective_offset();
  translation_segments_.Initialize(Vector(offset.x, offset.y), Vector());

  RootUI* root_ui = owner_->GetRootUI();
  if (!root_ui)
    return;
  root_ui->AddPendingAnimation(this);

  // Need a [re]start.
  state_ = INITIALIZED;
}

// Second step of starting an animation - grab "after trigger" values.
bool AnimationTimeline::Start(real64 start_time) {
  if (!owner_ || state_ != INITIALIZED)
    return false;

  // Store "final", "after trigger" values
  alpha_segments_.Start(owner_->alpha(), start_time);
  translation_segments_.Start(Vector(owner_->local_offset()), start_time);

  state_ = PLAYING;
  return true;
}

bool AnimationTimeline::Advance(real64 current_time) {
  if (!owner_)
    return false;

  ASSERT(state_ != IDLE);

  // If it's the first time the animation is being advanced, let it
  // snapshot the property values.
  if (state_ == INITIALIZED && !Start(current_time))
    return false;

  int alpha = 255;
  bool alpha_active = alpha_segments_.Advance(current_time, &alpha);
  if (alpha_active && (animated_alpha_ != alpha)) {
    animated_alpha_ = alpha;
    owner_->Invalidate();
  }

  Vector translation;
  bool translation_active = translation_segments_.Advance(current_time,
                                                          &translation);
  if (translation_active) {
    Point local_offset = owner_->local_offset();
    Point animated_offset(Round(translation.x) - local_offset.x,
                          Round(translation.y) - local_offset.y);
    if (animated_offset_ != animated_offset) {
      animated_offset_ = animated_offset;
      owner_->Invalidate();
    }
  }

  if (!alpha_active && !translation_active) {
    state_ = IDLE;
    Cancel(true);
  }

  return (state_ == PLAYING);
}

bool AnimationTimeline::RequestCompletionMessage(void *user_data) {
  user_data_ = user_data;
  send_message_ = true;
  return true;
}

void AnimationTimeline::Cancel(bool completed) {
  if (!owner_)
    return;

  if (send_message_) {
    RootUI *root_ui = owner_->GetRootUI();
    if (root_ui) {
      AnimationMessageWorkItem *item =
          new AnimationMessageWorkItem(owner_, user_data_, completed);
      platform()->PostWorkItem(root_ui, item);
    }
  }
  animated_alpha_ = -1;
  animated_offset_ = Point();
  // Invalidate, since the finish values of animation might not be the same
  // as current values on the node.
  owner_->Invalidate();

}

}  // namespace glint
