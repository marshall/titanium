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

#include "glint/include/message.h"
#include "glint/include/current_time.h"
#include "glint/include/node.h"
#include "glint/include/transform.h"

namespace glint {

void Message::Clear() {
  code = GL_MSG_IDLE;
  virtual_key = 0;
  ui = NULL;
  platform_window = NULL;
  work_item = NULL;
  user_data = NULL;
  bubble = false;
  time_stamp = CurrentTime::Seconds();
  shift_key_pressed = false;
  ctrl_key_pressed = false;
  alt_key_pressed = false;
  mouse_button_down = false;
  character = 0;
  window_position = Point();
}

Point Message::GetLocalPosition(Node* node) const {
  Transform to_local;
  while (node) {
    Transform transform;
    node->GetTransformToLocal(&transform);
    to_local.AddPreTransform(transform);
    node = node->parent();
  }
  Vector local_mouse = to_local.TransformVector(screen_mouse_position);
  return Point(static_cast<int>(local_mouse.x),
               static_cast<int>(local_mouse.y));
}

}  // namespace glint

