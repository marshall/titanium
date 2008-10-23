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

#include <gtk/gtk.h>

#include <vector>

#include "glint/include/formatted_text.h"

#include "glint/crossplatform/core_util.h"
#include "glint/include/bitmap.h"
#include "glint/include/draw_stack.h"
#include "glint/linux/linux.h"

using namespace glint;

namespace glint_linux {

#ifdef DEBUG
static const char* kFormattedTextNodeName = "FormattedText";
#endif

static PangoAttribute* extend_attr(PangoAttribute* attr) {
  attr->start_index = 0;
  attr->end_index = G_MAXUINT;
  return attr;
}

static PangoAttribute* position_attr(PangoAttribute* attr, int start, int end) {
  attr->start_index = start;
  attr->end_index = end;
  return attr;
}

class LinuxFormatList : public FormatList {
 public:
  LinuxFormatList() : attr_list_(pango_attr_list_new()) {
  }
  explicit LinuxFormatList(PangoAttrList* list) : attr_list_(list) {
  }
  ~LinuxFormatList() {}
  virtual void add_family(int begin, int end, const char* name) {
    add_attribute(pango_attr_family_new(name), begin, end);
  }
  virtual void add_pixel_size(int begin, int end, int size) {
    add_attribute(pango_attr_size_new_absolute(PANGO_SCALE * size), begin, end);
  }
  virtual void add_bold(int begin, int end, bool bold) {
    add_attribute(pango_attr_weight_new(
        bold ? PANGO_WEIGHT_BOLD : PANGO_WEIGHT_NORMAL), begin, end);
  }
  virtual void add_italic(int begin, int end, bool italic) {
    add_attribute(pango_attr_style_new(
        italic ? PANGO_STYLE_ITALIC : PANGO_STYLE_NORMAL), begin, end);
  }
  virtual void add_underline(int begin, int end, bool underline) {
    add_attribute(pango_attr_underline_new(
        underline ? PANGO_UNDERLINE_SINGLE : PANGO_UNDERLINE_NONE), begin, end);
  }
  PangoAttrList* get_pango_attr_list_copy() {
    return pango_attr_list_copy(attr_list_.get());
  }
 private:
  void add_attribute(PangoAttribute* attribute, int begin, int end) {
    attribute->start_index = begin;
    attribute->end_index = end;
    pango_attr_list_change(attr_list_.get(), attribute);
  }
  ScopedAttrList attr_list_;
  DISALLOW_EVIL_CONSTRUCTORS(LinuxFormatList);
};

class LinuxFormattedText : public FormattedText {
 public:
  LinuxFormattedText()
    : foreground_(0xFF000000) {
  #ifdef DEBUG
    type_name_ = kFormattedTextNodeName;
  #endif
  }
  virtual ~LinuxFormattedText() {}
  virtual void set_text(const std::string& text) {
    text_ = text;
  }
  virtual bool set_markup(const std::string& markup) {
    PangoAttrList* attr_list = NULL;
    char* text = NULL;
    if (!pango_parse_markup(markup.data(),
                            markup.length(),
                            0,
                            &attr_list,
                            &text,
                            NULL,
                            NULL))
      return false;
    ASSERT(attr_list && text);
    text_.assign(text);
    custom_attrs_.reset(attr_list);
    return true;
  }
  virtual void set_format(FormatList* format) {
    custom_attrs_.reset(reinterpret_cast<LinuxFormatList*>(format)->
        get_pango_attr_list_copy());
  }
  virtual void set_foreground_color(const Color& fg) {
    foreground_ = fg;
  }

 protected:
  // Converts width and height from Pango units to device (pixel) units.
  static Size PangoSizeToDeviceSize(int width, int height) {
    return Size((width + PANGO_SCALE - 1) / PANGO_SCALE,
                (height + PANGO_SCALE - 1) / PANGO_SCALE);
  }
  // Get combined pango attribute list from default attributes,
  // custom attributes and if applicable, ellipsis attributes.
  void make_combined_attrs(int ellipsis_position) {
    PangoAttrList* attrs = pango_attr_list_new();
    // Apply default attributes globally.
    pango_attr_list_insert(attrs, extend_attr(pango_attr_family_new("Sans")));
    pango_attr_list_insert(attrs, extend_attr(
        pango_attr_size_new_absolute(14 * PANGO_SCALE)));
    // Apply custom attributes.
    pango_attr_list_splice(attrs,
                           custom_attrs_.get(),
                           0,
                           rendered_text_.length());
    // Apply ellipsis attributes if necessary.
    if (ellipsis_position >= 0) {
     pango_attr_list_change(
         attrs,
         position_attr(pango_attr_family_new("Sans"),
                       ellipsis_position,
                       ellipsis_position + 3)); // ellipsis UTF-8 code is 3-byte
     pango_attr_list_change(
         attrs,
         position_attr(pango_attr_size_new_absolute(14 * PANGO_SCALE),
                       ellipsis_position,
                       ellipsis_position + 3));
     pango_attr_list_change(
         attrs,
         position_attr(pango_attr_weight_new(PANGO_WEIGHT_NORMAL),
                       ellipsis_position,
                       ellipsis_position + 3));
     pango_attr_list_change(
         attrs,
         position_attr(pango_attr_style_new(PANGO_STYLE_NORMAL),
                       ellipsis_position,
                       ellipsis_position + 3));
     pango_attr_list_change(
         attrs,
         position_attr(pango_attr_underline_new(PANGO_UNDERLINE_NONE),
                       ellipsis_position,
                       ellipsis_position + 3));
    }
    combined_attrs_.reset(attrs);
  }
  virtual Size OnComputeRequiredSize(Size constraint) {
    ellipsis_position_ = -1;  // -1 indicates that no ellipsis is used
    if (text_.length() == 0)  // nothing to draw, so we take no space
      return Size(0, 0);
    constraint.width *= PANGO_SCALE;
    constraint.height *= PANGO_SCALE;
    rendered_text_ = text_;
    make_layout(constraint.width, -1);
    ASSERT(layout_.get() != NULL);
    PangoRectangle rect;

    // Get height of first line.
    ScopedLayoutIter iterator(pango_layout_get_iter(layout_.get()));
    pango_layout_iter_get_line_extents(iterator.get(), NULL, &rect);
    int first_line_height = rect.height;

    if (first_line_height > constraint.height) {
      // First line is too tall to display--try displaying just an ellipsis
      rendered_text_.clear();
      make_layout(constraint.width, 0);
      ASSERT(layout_.get() != NULL);
      // Get dimensions of ellipsis-only layout
      pango_layout_get_extents(layout_.get(), NULL, &rect);
      if (rect.height > constraint.height ||
         rect.width > constraint.width) {
        // Even ellipsis doesn't fit -- display nothing.
        return Size(0, 0);
      }
      return PangoSizeToDeviceSize(rect.width, rect.height);
    }
    // Search for last line that fully fits within constraint
    int last_line_idx = 0;
    while (true) {
      if (!pango_layout_iter_next_line(iterator.get()))
        break;
      pango_layout_iter_get_line_extents(iterator.get(), NULL, &rect);
      if (rect.y + rect.height >= constraint.height)
        break;
      ++last_line_idx;
    }
    PangoLayoutLine* last_line = pango_layout_get_line(layout_.get(),
                                                       last_line_idx);
    ASSERT(last_line != NULL);
    int text_end = last_line->start_index + last_line->length;
    ASSERT(text_end > 0 &&
           static_cast<size_t>(text_end) <= rendered_text_.length());
    if (static_cast<size_t>(text_end) < rendered_text_.length()) {
      // Not all text fits -- truncate and add ellipsis.
      int line_byte_count = last_line->length;
      int min_text_end = text_end - line_byte_count;
      // Try truncating and adding ellipsis with a countdown from last character
      // on last line.
      while (text_end >= min_text_end) {
        rendered_text_.resize(text_end);  // truncate
        rendered_text_.push_back('\xE2');  // insert UTF-8 code for ellipsis
        rendered_text_.push_back('\x80');
        rendered_text_.push_back('\xA6');
        make_layout(constraint.width, text_end);
        pango_layout_get_extents(layout_.get(), NULL, &rect);
        if (rect.height <= constraint.height &&
            rect.width <= constraint.width) {
          // layout with current tested ellipsis position is small enough
          ellipsis_position_ = text_end;
          break;
        }
        // Step back one UTF-8 character.
        text_end = text_end > min_text_end ?
            g_utf8_prev_char(rendered_text_.data() + text_end) -
                rendered_text_ .data() :
            -1;
      }
      if (text_end < min_text_end) {
        // Even ellipsis doesn't fit (horizontally) -- display nothing.
        return Size(0, 0);
      }
    }
    pango_layout_get_extents(layout_.get(), NULL, &rect);
    ASSERT(rect.width <= constraint.width &&
           rect.height <= constraint.height);
    return PangoSizeToDeviceSize(rect.width, rect.height);
  }
  virtual Size OnSetLayoutBounds(Size reserved) {
    return OnComputeRequiredSize(reserved);
  }
  virtual void OnComputeDrawingBounds(Rectangle* bounds) {
    ASSERT(bounds);
    bounds->Set(Point(), final_size());
  }
  virtual bool OnDraw(DrawStack *stack) {
    Size final_size = this->final_size();
    if (final_size.height == 0)
      return true;
    ASSERT(text_.length() != 0);
    make_layout(final_size.width * PANGO_SCALE, ellipsis_position_);
    ASSERT(stack && stack->target() && stack->Top());
    if (!stack || !stack->target() || !stack->Top())
      return false;

    Bitmap *target = stack->target();
    DrawContext *draw_context = stack->Top();
    Rectangle bounds(Point(), final_size);

    // empty string or rendering will be clipped anyways, return.
    if (bounds.IsEmpty())
      return true;

    Bitmap local_copy(Point(), Size(final_size.width, final_size.height));
    pango_layout_set_width(layout_.get(), final_size.width * PANGO_SCALE);
    draw_text_layout(layout_.get(),
                     foreground_,
                     final_size.width,
                     final_size.height,
                     &local_copy);
    Rectangle source_clip(local_copy.origin(), local_copy.size());
    target->Compose(local_copy,
                    source_clip,
                    draw_context->transform_to_local,
                    draw_context->clip,
                    draw_context->alpha);
    return true;
  }

 private:
  void make_layout(int width, int ellipsis_position) {
    // width in Pango units, not device units
    if (!layout_.get()) {
      ScopedContext pango_context(gdk_pango_context_get());
      layout_.reset(pango_layout_new(pango_context.get()));
    }
    pango_layout_set_single_paragraph_mode(layout_.get(), FALSE);
    pango_layout_set_ellipsize(layout_.get(), PANGO_ELLIPSIZE_NONE);
    pango_layout_set_width(layout_.get(), width);
    pango_layout_set_wrap(layout_.get(), PANGO_WRAP_WORD_CHAR);
    pango_layout_set_text(layout_.get(),
                          rendered_text_.data(),
                          rendered_text_.length());
    make_combined_attrs(ellipsis_position);
    pango_layout_set_attributes(layout_.get(), combined_attrs_.get());
  }

  int width_, height_;
  ScopedAttrList custom_attrs_;

  /* Attribute list obtained from overlaying ellipsis attributes over custom
     attributes over default attributes.
  */
  ScopedAttrList combined_attrs_;

  // Byte index of ellipsis within rendered_text_ (-1 means no ellipsis)
  int ellipsis_position_;
  Color foreground_;
  ScopedLayout layout_;
  std::string text_;

  // Text that is actually rendered (different from text_ if ellipsis is shown)
  std::string rendered_text_;

  DISALLOW_EVIL_CONSTRUCTORS(LinuxFormattedText);
};
}

// Linux-specific definition of base classes' static creation functions
namespace glint {
FormatList* FormatList::Create() {
  return new glint_linux::LinuxFormatList();
}


#ifdef GLINT_ENABLE_XML
glint::BaseObject* glint::FormattedText::CreateInstance() {
  return new glint_linux::LinuxFormattedText();
}
#endif  // GLINT_ENABLE_XML
}  // namespace glint_linux
