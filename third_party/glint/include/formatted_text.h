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

#ifndef GLINT_INCLUDE_FORMATTED_TEXT_H__
#define GLINT_INCLUDE_FORMATTED_TEXT_H__

#include <string>
#include <vector>

#include "glint/include/node.h"

namespace glint {

class FormattedText;

class FormatList {
 public:
  FormatList() {}
  virtual ~FormatList() {}
  static FormatList* Create();
  virtual void add_family(int begin, int end, const char* name) = 0;
  virtual void add_pixel_size(int begin, int end, int size) = 0;
  virtual void add_bold(int begin, int end, bool bold) = 0;
  virtual void add_italic(int begin, int end, bool italic) = 0;
  virtual void add_underline(int begin, int end, bool underline) = 0;

 private:
  DISALLOW_EVIL_CONSTRUCTORS(FormatList);
};

class FormattedText : public Node {
 public:
  FormattedText() {}
  virtual ~FormattedText() {}

#ifdef GLINT_ENABLE_XML
  static BaseObject* CreateInstance();
  static SetPropertyResult SetMarkup(BaseObject *node,
                                     const std::string& markup) {
    return reinterpret_cast<FormattedText*>(node)->set_markup(markup) ?
        PROPERTY_OK : PROPERTY_BAD_VALUE;
  }
#endif  // GLINT_ENABLE_XML

  virtual void set_text(const std::string& text) = 0;
  virtual bool set_markup(const std::string& markup) = 0;
  // This object takes ownership of format and is responsible for its deletion.
  virtual void set_format(FormatList* format) = 0;
  virtual void set_foreground_color(const Color& fg) = 0;

 protected:
  virtual Size OnComputeRequiredSize(Size constraint) = 0;
  virtual Size OnSetLayoutBounds(Size reserved) = 0;
  virtual bool OnDraw(DrawStack *stack) = 0;
  virtual void OnComputeDrawingBounds(Rectangle* bounds) = 0;

 private:
  DISALLOW_EVIL_CONSTRUCTORS(FormattedText);
};
}  // namespace glint

#endif  // GLINT_INCLUDE_FORMATTED_TEXT_H__
