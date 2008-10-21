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

#include "glint/include/button.h"
#include "glint/crossplatform/core_util.h"
#include "glint/include/bitmap.h"
#include "glint/include/root_ui.h"
#include "glint/include/xml_parser.h"

namespace glint {

#ifdef DEBUG
static const char* kButtonNodeName = "Button";
#endif

class ButtonMessageHandler : public MessageHandler {
 public:
  ButtonMessageHandler(Button* owner)
    : owner_(owner),
      callback_(NULL),
      user_info_(NULL),
      capture_(false),
      set_cursor_(false),
      cursor_(CURSOR_POINTER) {
  }

  MessageResultCode HandleMessage(const Message& message) {
    if (!owner_->enabled())
      return MESSAGE_CONTINUE;

    // return quickly from broadcasted message, we only need it
    // to see if we are in MouseLeave situation.
    if (message.code == GL_MSG_MOUSEMOVE_BROADCAST) {
      if (capture_ ||
          owner_->state() == Button::NORMAL)
        return MESSAGE_CONTINUE;
    }

    RootUI* root_ui = owner_->GetRootUI();
    if (!root_ui)
      return MESSAGE_CONTINUE;

    switch (message.code) {
      case GL_MSG_MOUSEMOVE:
        if (DirectlyOverButton(message)) {
          if (capture_) {
            // Check for Mouse location - if Button has capture, it gets
            // all mouse messages but should be NORMAL if they fall outside.
            owner_->set_state(IsMouseWithin(message) ? Button::PRESSED :
                                                       Button::NORMAL);
          } else {
            owner_->set_state(Button::HOVER);
          }

          if (set_cursor_) {
            // TODO(levin): Glint should probably have a pass just to do
            // set cursor changes and then if no one sets the cursor, it
            // could be set to a default (pointer).
            // Right now, we rely on the
            // fact that the window is created with a default window cursor
            // of a pointer to change the cursor back after it leaves the
            // button.
            root_ui->SetCursor(cursor_);
            // After setting the cursor, don't let other nodes have
            // a crack at it or they may change it to something else.
            // This is mildly ugly -- stopping the mouse move processing.
            // It would be fixed by having the pass just to set the
            // cursor as mentioned above.
            return MESSAGE_HANDLED;
          }
        } else {
          owner_->set_state(Button::NORMAL);
        }
        break;
      case GL_MSG_MOUSEMOVE_BROADCAST: {
          // Check for MouseLeave condition
          Rectangle bounds(Point(), owner_->final_size());
          // GL_MSG_MOUSEMOVE_BROADCAST comes with screen coordinate of the
          // mouse, so transform it first.
          Point mouse = message.GetLocalPosition(owner_);
          if (!bounds.Contains(mouse)) {
            owner_->set_state(Button::NORMAL);
          }
        }
        break;
      case GL_MSG_LBUTTONDOWN:
        if (DirectlyOverButton(message)) {
          capture_ = root_ui->StartMouseCapture(owner_);
          owner_->set_state(Button::PRESSED);
          return MESSAGE_HANDLED;
        }
      case GL_MSG_CAPTURELOST:
      case GL_MSG_MOUSELEAVE:
        owner_->set_state(Button::NORMAL);
        break;
      case GL_MSG_LBUTTONUP:
        if (capture_ && DirectlyOverButton(message)) {
          root_ui->EndMouseCapture();
          capture_ = false;
          if (callback_ && IsMouseWithin(message)) {
            owner_->set_state(Button::NORMAL);
            callback_(owner_->id(), user_info_);
            // Note: after callback (which is a user-defined reaction to a
            // button click) the button itself and this message handler can
            // be already deleted. Don't access any instance variables.
          }
        }
        return MESSAGE_HANDLED;
      default:
        return MESSAGE_CONTINUE;
    }
    return MESSAGE_CONTINUE;
  }

  void set_callback(ClickHandler callback) {
    callback_ = callback;
  }

  void set_user_info(void* user_info) {
    user_info_ = user_info;
  }

  void SetCursor(Cursors cursor) {
    set_cursor_ = true;
    cursor_ = cursor;
  }

  void ClearCursor() {
    set_cursor_ = false;
  }

 private:
  bool IsMouseWithin(const Message& message) {
    Rectangle bounds(Point(), owner_->final_size());
    Point mouse = Point(static_cast<int>(message.mouse_position.x),
                        static_cast<int>(message.mouse_position.y));
    return bounds.Contains(mouse);
  }

  // Message.user_data is a top-hit-tested node in mouse messages.
  // The mouse is on top of the button if it is the button itself
  // or its descendants.
  bool DirectlyOverButton(const Message& message) {
    Node* node = reinterpret_cast<Node*>(message.user_data);
    while (node) {
      if (node == owner_)
        return true;
      node = node->parent();
    }
    return false;
  }

  Button* owner_;
  ClickHandler callback_;
  void* user_info_;
  bool capture_;
  bool set_cursor_;
  Cursors cursor_;
  DISALLOW_EVIL_CONSTRUCTORS(ButtonMessageHandler);
};

Button::Button()
    : enabled_(true),
      message_handler_(NULL),
      state_(NORMAL) {
#ifdef DEBUG
  type_name_ = kButtonNodeName;
#endif

  message_handler_ = new ButtonMessageHandler(this);
  if (message_handler_)
    AddHandler(message_handler_);
}

// For Button, we get a 1x4 image strip with 4 equally-sized fragments
// for different states. Chop the strip according to state.
bool Button::GetGrid(GridStops* grid) {
  if (!grid)
    return false;

  int tile_width = nine_grid()->size().width / 4;
  int left = 0;

  switch (state_) {
    case HOVER:
      left = tile_width;
      break;
    case PRESSED:
      left = 2 * tile_width;
      break;
    case DISABLED:
      left = 3 * tile_width;
      break;
    default:
      break;
  }

  if (!NineGrid::GetGrid(grid))
    return false;

  // Move horizontal stops to correspond to the button's state.
  int right = left + tile_width;
  grid->x[0] = left;
  grid->x[1] = max<int>(left, left + (tile_width - center_width()) / 2);
  grid->x[2] = min<int>(right, grid->x[1] + center_width());
  grid->x[3] = right;

  return true;
}

bool Button::SetClickHandler(ClickHandler handler, void* user_info) {
  if (!message_handler_)
    return false;
  message_handler_->set_callback(handler);
  message_handler_->set_user_info(user_info);
  return true;
}

bool Button::SetCursor(Cursors cursor) {
  if (!message_handler_)
    return false;
  message_handler_->SetCursor(cursor);
  return true;
}

bool Button::ClearCursor() {
  if (!message_handler_)
    return false;
  message_handler_->ClearCursor();
  return true;
}

#ifdef GLINT_ENABLE_XML
SetPropertyResult Button::SetEnabled(BaseObject* node,
                                     const std::string& value) {
  ASSERT(node);
  bool bool_value;
  if (!XmlParser::StringToBool(value, &bool_value))
    return PROPERTY_HAS_INCORRECT_FORMAT;
  static_cast<Button*>(node)->enabled_ = bool_value;
  return PROPERTY_OK;
}
#endif  // GLINT_ENABLE_XML

}  // namespace glint
