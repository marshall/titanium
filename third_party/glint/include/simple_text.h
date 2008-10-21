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

#ifndef GLINT_INCLUDE_SIMPLE_TEXT_H__
#define GLINT_INCLUDE_SIMPLE_TEXT_H__

#include "glint/include/node.h"

namespace glint {

class PlatformFont;

// Holds the value needed to create a platform-specific font.
struct FontDescription {
  FontDescription()
    : height(0),
      bold(false),
      italic(false)
#ifndef OSX
      , underline(false),
      strike_out(false)
#endif
  {
  }

  std::string family_name;
  int height;  // in points
  bool bold;
  bool italic;
#ifndef OSX
  // OSX Fonts don't contain these traits, and they aren't being used
  // anywhere by Glint anyway - no way to set these properties from a Glint
  // XML file. The only way they might currently matter is if the default
  // font on Win32 uses underline or strikethrough.
  // If we give Glint XML files the capability to set these properties,
  // the way to use them on OSX will be through NSParagraphStyle.
  bool underline;
  bool strike_out;
#endif
};

class SimpleText : public Node {
 public:
  SimpleText();
  ~SimpleText();

  std::string text() { return text_;}
  void set_text(const std::string &text) {
    if (text != text_) {
      text_ = text;
      Invalidate();
    }
  }

  std::string font_family() { return font_description_.family_name;}
  void set_font_family(const std::string &font_family) {
    if (font_description_.family_name != font_family) {
      font_description_.family_name = font_family;
      platform_font_loaded_ = false;
      Invalidate();
    }
  }

  int font_size() { return font_description_.height; }
  void set_font_size(int font_size) {
    if (font_description_.height != font_size) {
      font_description_.height = font_size;
      platform_font_loaded_ = false;
      Invalidate();
    }
  }

  bool italic() { return font_description_.italic; }
  void set_italic(bool italic) {
    if (font_description_.italic != italic) {
      font_description_.italic = italic;
      platform_font_loaded_ = false;
      Invalidate();
    }
  }

  bool bold() { return font_description_.bold; }
  void set_bold(bool bold) {
    if (font_description_.bold != bold) {
      font_description_.bold = bold;
      platform_font_loaded_ = false;
      Invalidate();
    }
  }

  Color foreground() { return foreground_; }
  void set_foreground(Color foreground) {
    if (foreground_ != foreground) {
      foreground_ = foreground;
      platform_font_loaded_ = false;
      Invalidate();
    }
  }

  // TODO(dimich): implement this
  bool drop_shadow() { return drop_shadow_; }
  void set_drop_shadow(bool drop_shadow) { drop_shadow_ = drop_shadow; }

#ifdef GLINT_ENABLE_XML
  static BaseObject* CreateInstance() {
    return new SimpleText();
  }
  static SetPropertyResult SetText(BaseObject *node,
                                   const std::string& value);
  static SetPropertyResult SetFontSize(BaseObject *node,
                                       const std::string& value);
  static SetPropertyResult SetForeground(BaseObject *node,
                                         const std::string& value);
  static SetPropertyResult SetBold(BaseObject *node,
                                   const std::string& value);
  static SetPropertyResult SetItalic(BaseObject *node,
                                     const std::string& value);
  static SetPropertyResult SetFontFamily(BaseObject *node,
                                         const std::string& value);
#endif  // GLINT_ENABLE_XML

 protected:
  virtual Size OnComputeRequiredSize(Size constraint);
  virtual Size OnSetLayoutBounds(Size reserved);
  virtual bool OnDraw(DrawStack *stack);
  virtual void OnComputeDrawingBounds(Rectangle* bounds);

 private:
  bool EnsureFont();

  PlatformFont *platform_font_;
  FontDescription font_description_;
  std::string text_;
  Color foreground_;
  bool drop_shadow_;
  bool platform_font_loaded_;

  DISALLOW_EVIL_CONSTRUCTORS(SimpleText);
};

}  // namespace glint

#endif  // GLINT_INCLUDE_SIMPLE_TEXT_H__
