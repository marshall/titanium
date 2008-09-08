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

#ifndef GLINT_INCLUDE_BUTTON_H__
#define GLINT_INCLUDE_BUTTON_H__

#include "glint/include/cursors.h"
#include "glint/include/nine_grid.h"

namespace glint {

typedef void (*ClickHandler)(const std::string& button_id, void* user_info);

class ButtonMessageHandler;

// Button expects a nine_grid bitmap which in fact has 4 parts -
// for normal, mouseover, pressed and disabled states.
class Button : public NineGrid {
 public:
  Button();

  virtual bool GetGrid(GridStops* grid);

  bool enabled() { return enabled_; }
  void set_enabled(bool enabled) {
    if (enabled_ != enabled) {
      enabled_ = enabled;
      set_state(enabled ? NORMAL : DISABLED);
      Invalidate();
    }
  }

  enum State {
    NORMAL,
    HOVER,
    PRESSED,
    DISABLED,
  };

  State state() { return state_; }

  // Note that one can set the visible state of the button to PRESSED and
  // back to NORMAL. However, that does not generate a callback to
  // ClickHandler. Only user interaction does. It's safer and less confusing -
  // but revisit if testing or accessibility will require otherwise.
  void set_state(State state) {
    if (state_ != state) {
      state_ = state;
      Invalidate();
    }
  }

  // Sets a cursor to show when the mouse is over the button.
  bool SetCursor(Cursors cursor);

  // Reset the cursor so that there is no cursor specified
  // for the button anymore.
  bool ClearCursor();

  // Sets the handler to call back when user clicks the button.
  // New handler replaces the previously set one.
  bool SetClickHandler(ClickHandler handler, void* user_info);

#ifdef GLINT_ENABLE_XML
  static BaseObject* CreateInstance() {
    return new Button();
  }

  static SetPropertyResult SetEnabled(BaseObject* node,
                                      const std::string& value);
#endif  // GLINT_ENABLE_XML

 protected:
  virtual bool RequiresSnapshot() { return true; }
  virtual std::string TagName() {
    static const std::string button("button");
    return button;
  }

 private:
  bool enabled_;
  ButtonMessageHandler* message_handler_;
  State state_;
  DISALLOW_EVIL_CONSTRUCTORS(Button);
};

}  // namespace glint

#endif  // GLINT_INCLUDE_BUTTON_H__
