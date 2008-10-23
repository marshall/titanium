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

#ifndef GLINT_INCLUDE_MESSAGE_H__
#define GLINT_INCLUDE_MESSAGE_H__

#include "glint/include/base_object.h"
#include "glint/include/point.h"

namespace glint {

// Glint messages.
enum GlintMessages {
  GL_MSG_QUIT,
  GL_MSG_IDLE,
  GL_MSG_WORK_ITEM,
  GL_MSG_LBUTTONDOWN_NOTIFY,
  GL_MSG_LBUTTONUP_NOTIFY,
  GL_MSG_RBUTTONDOWN_NOTIFY,
  GL_MSG_RBUTTONUP_NOTIFY,
  GL_MSG_MBUTTONDOWN_NOTIFY,
  GL_MSG_MBUTTONUP_NOTIFY,
  GL_MSG_MOUSEMOVE_NOTIFY,
  GL_MSG_LBUTTONDOWN,
  GL_MSG_LBUTTONUP,
  GL_MSG_RBUTTONDOWN,
  GL_MSG_RBUTTONUP,
  GL_MSG_MBUTTONDOWN,
  GL_MSG_MBUTTONUP,
  GL_MSG_MOUSEMOVE,
  GL_MSG_MOUSEMOVE_BROADCAST,
  GL_MSG_CAPTURELOST,
  GL_MSG_KEYDOWN,
  GL_MSG_KEYUP,
  GL_MSG_ANIMATION_COMPLETED,
  GL_MSG_ANIMATION_ABORTED,
  GL_MSG_MOUSELEAVE,
  GL_MSG_SETCURSOR,
  GL_MSG_SETFOCUS,
  GL_MSG_KILLFOCUS,
  // Sent when system forcefully changes position of the window
  // so we need to update the layout and margins of the root node.
  GL_MSG_WINDOW_POSITION_CHANGED,
  // Resolution, size, color depth, system fonts or some other display setting
  // has changed. Re-query the system settings/config to update the display.
  GL_MSG_DISPLAY_SETTINGS_CHANGED,
  // Following messages are typically mapped to keys in an OS-dependent way.
  // So Glint has abstract messages and the platform level is responsible for
  // proper mapping of user input into them.
  GL_MSG_CUT,
  GL_MSG_COPY,
  GL_MSG_PASTE,
  GL_MSG_SELECT_ALL,
  GL_MSG_CLOSE_DOCUMENT,  // Like Ctrl+F4 on windows
  GL_MSG_USER,  // This should be the last message in this enum.
};

// Result codes, returned from functions that return "MessageResultCode"
enum MessageResultCode {
  MESSAGE_HANDLED,
  MESSAGE_CONTINUE,
  MESSAGE_CANCEL,
};

class Node;
class RootUI;
class WorkItem;

// The Glint message - platform-independent variant of a window message
// Plain struct to allow for easier platform layer implementation.
struct Message : public BaseObject {
  Message() {
    Clear();
  }

  void Clear();
  Point GetLocalPosition(Node* node) const;

  GlintMessages code;
  RootUI *ui;
  void *platform_window;
  Vector screen_mouse_position;
  Vector mouse_position;
  bool bubble;

  int virtual_key;
  int character;
  bool shift_key_pressed;
  bool ctrl_key_pressed;
  bool alt_key_pressed;
  // Note: on 2- or 3-button mice this corresponds to the left button.
  bool mouse_button_down;

  // Work item pointer (in case of GL_MSG_WORK_ITEM)
  WorkItem *work_item;
  void *user_data;
  real64 time_stamp;
  Point window_position;
};

}  // namespace glint

#endif  // GLINT_INCLUDE_MESSAGE_H__
