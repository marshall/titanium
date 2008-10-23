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

#include "glint/include/simple_text.h"
#include "glint/crossplatform/core_util.h"
#include "glint/include/bitmap.h"
#include "glint/include/draw_stack.h"
#include "glint/include/xml_parser.h"

namespace glint {

#ifdef DEBUG
static const char* kSimpleTextNodeName = "SimpleText";
#endif

SimpleText::SimpleText()
    : platform_font_(NULL),
      foreground_(Color(0xFF000000)),
      drop_shadow_(false),
      platform_font_loaded_(false) {
#ifdef DEBUG
  type_name_ = kSimpleTextNodeName;
#endif

  // TODO(dimich): cache this in win32
  platform()->GetDefaultFontDescription(&font_description_);
}

SimpleText::~SimpleText() {
  if (platform_font_)
    platform()->ReleaseFont(platform_font_);
  platform_font_ = NULL;
}

bool SimpleText::EnsureFont() {
  // Don't try again and again if initialization didn't work
  // from the first time.
  if (platform_font_loaded_)
    return platform_font_ != NULL;

  if (platform_font_) {
    platform()->ReleaseFont(platform_font_);
    platform_font_ = NULL;
  }

  platform_font_loaded_ = true;

  // If the family name was not set, retrieve default OS font
  if (font_description_.family_name.empty()) {
    if (!platform()->GetDefaultFontDescription(&font_description_))
      return false;
  }

  platform_font_ = platform()->CreateFontFromDescription(font_description_);

  return platform_font_ != NULL;
}

Size SimpleText::OnComputeRequiredSize(Size constraint) {
  if (!EnsureFont())
    return Size(0, 0);

  Rectangle bounds;
  if (!platform()->MeasureSimpleText(platform_font_,
                                     text_,
                                     constraint.width,
                                     false,  // single line?
                                     true,   // use ellipsis?
                                     &bounds)) {
    return Size(0, 0);
  }

  return bounds.size();
}

// repeat measuring, now at reserved wrapping width.
// TODO(dimich): skip measurement if there were no wrapping during
// ComputeRequiredSize - since layout bounds are at least as big as
// required size, if the text did not wrap then it will not wrap now.
Size SimpleText::OnSetLayoutBounds(Size reserved) {
  if (!EnsureFont())
    return Size(0, 0);

  Rectangle bounds;
  if (!platform()->MeasureSimpleText(platform_font_,
                                     text_,
                                     reserved.width,
                                     false,  // single line?
                                     true,   // use ellipsis?
                                     &bounds)) {
    return Size(0, 0);
  }

  return bounds.size();
}

void SimpleText::OnComputeDrawingBounds(Rectangle* bounds) {
  ASSERT(bounds);
  bounds->Set(Point(), final_size());
}

bool SimpleText::OnDraw(DrawStack *stack) {
  if (!EnsureFont())
    return false;
//  Rectangle clip = stack.clip();
  ASSERT(stack && stack->target() && stack->Top());
  if (!stack || !stack->target() || !stack->Top())
    return false;

  Bitmap *target = stack->target();
  DrawContext *draw_context = stack->Top();
  Size final_size = this->final_size();
  Rectangle bounds(Point(), final_size);

  // empty string or rendering will be clipped anyways, return.
  if (bounds.IsEmpty())
    return true;

  Bitmap local_copy(Point(), final_size);
  if (!platform()->DrawSimpleText(platform_font_,
                                  text_,
                                  &local_copy,
                                  bounds,
                                  foreground_,
                                  false,    // singleline?
                                  true)) {  // use ellipsis?
    return false;
  }

  Rectangle source_clip;
  source_clip.SetHuge();
  target->Compose(local_copy,
                  source_clip,
                  draw_context->transform_to_global,
                  draw_context->clip,
                  draw_context->alpha);

  return true;
}

#ifdef GLINT_ENABLE_XML
SetPropertyResult SimpleText::SetText(BaseObject *node,
                                      const std::string& value) {
  ASSERT(node);
  std::string text;
  if (!XmlParser::StringToText(value, &text))
    return PROPERTY_HAS_INCORRECT_FORMAT;
  static_cast<SimpleText*>(node)->set_text(text);
  return PROPERTY_OK;
}

SetPropertyResult SimpleText::SetFontSize(BaseObject *node,
                                          const std::string& value) {
  ASSERT(node);
  int size;
  if (!XmlParser::StringToInt(value, &size))
    return PROPERTY_HAS_INCORRECT_FORMAT;
  if (size <= 0)
    return PROPERTY_BAD_VALUE;
  static_cast<SimpleText*>(node)->set_font_size(size);
  return PROPERTY_OK;
}

SetPropertyResult SimpleText::SetForeground(BaseObject *node,
                                            const std::string& value) {
  ASSERT(node);
  Color color(0x00000000);
  if (!XmlParser::StringToColor(value, &color))
    return PROPERTY_HAS_INCORRECT_FORMAT;
  static_cast<SimpleText*>(node)->set_foreground(color);
  return PROPERTY_OK;
}

SetPropertyResult SimpleText::SetBold(BaseObject *node,
                                      const std::string& value) {
  ASSERT(node);
  bool bool_value;
  if (!XmlParser::StringToBool(value, &bool_value))
    return PROPERTY_HAS_INCORRECT_FORMAT;
  static_cast<SimpleText*>(node)->set_bold(bool_value);
  return PROPERTY_OK;
}

SetPropertyResult SimpleText::SetItalic(BaseObject *node,
                                        const std::string& value) {
  ASSERT(node);
  bool bool_value;
  if (!XmlParser::StringToBool(value, &bool_value))
    return PROPERTY_HAS_INCORRECT_FORMAT;
  static_cast<SimpleText*>(node)->set_italic(bool_value);
  return PROPERTY_OK;
}

SetPropertyResult SimpleText::SetFontFamily(BaseObject *node,
                                            const std::string& value) {
  ASSERT(node);
  if (value.empty())
    return PROPERTY_BAD_VALUE;
  static_cast<SimpleText*>(node)->set_font_family(value);
  return PROPERTY_OK;
}
#endif  // GLINT_ENABLE_XML


}  // namespace glint
