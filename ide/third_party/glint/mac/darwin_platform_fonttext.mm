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

// Implementation of font/text related parts of Platform interface for Darwin

#import <assert.h>
#import <Cocoa/Cocoa.h>

#import "glint/include/bitmap.h"
#import "glint/include/simple_text.h"
#import "glint/mac/darwin_platform.h"

namespace glint_darwin {

const int kMaxWrappingWidth = 10000;  // Arbitrary, until change is required.

// Check whether the passed in pointer is an NSFont we created/returned
NSFont* DarwinPlatform::ValidateFont(PlatformFont* platform_font) {
  NSFont* ns_font = reinterpret_cast<NSFont*>(platform_font);
  if (!ns_font || [all_fonts_ countForObject:ns_font] == 0) {
    assert(false && "Invalid platform font");
    return nil;
  }
  return ns_font;
}

// Fills in FontDescription struct with the info about current default font.
// Normally, it corresponds to the font used in the default OS message boxes.
bool DarwinPlatform::GetDefaultFontDescription(FontDescription* font) {
  if (!font)
    return false;

  float system_font_size =
      [NSFont systemFontSizeForControlSize:NSRegularControlSize];
  NSFont* system_font = [NSFont systemFontOfSize:system_font_size];

  font->family_name = [[system_font familyName] UTF8String];

  font->height = [system_font pointSize];

  NSFontTraitMask system_font_traits =
      [[NSFontManager sharedFontManager] traitsOfFont:system_font];
  font->bold = system_font_traits & NSBoldFontMask;
  font->italic = system_font_traits & NSItalicFontMask;

  return true;
}

// Returns a default font.
// Normally, it corresponds to the font used in the default OS message boxes.
// Use ReleaseFont to free the internal resources.
PlatformFont* DarwinPlatform::GetDefaultFont() {
  float system_font_size =
      [NSFont systemFontSizeForControlSize:NSRegularControlSize];
  NSFont* system_font = [NSFont systemFontOfSize:system_font_size];

  if (system_font) {
    [all_fonts_ addObject:system_font];
  }

  return reinterpret_cast<PlatformFont*>(system_font);
}

// Creates a platform-specific font object and returns it as an opaque
// pointer. Use ReleaseFont to free the internal resources.
PlatformFont* DarwinPlatform::CreateFontFromDescription(
    const FontDescription &font) {
  NSFontManager* font_manager = [NSFontManager sharedFontManager];

  NSString* family_name =
      [NSString stringWithUTF8String:font.family_name.c_str()];

  NSFontTraitMask traits =
      (font.bold ? NSBoldFontMask : 0) |
      (font.italic ? NSItalicFontMask : 0);

  int weight = font.bold ? bold_weight_ : normal_weight_;

  NSFont* ns_font = [font_manager fontWithFamily:family_name
                                          traits:traits
                                          weight:weight
                                            size:font.height];

  if (ns_font) {
    [all_fonts_ addObject:ns_font];
  }

  return reinterpret_cast<PlatformFont*>(ns_font);
}

// Releases the font previously created/obtained by GetDefaultFont or
// CreateFontFromDescription.
void DarwinPlatform::ReleaseFont(PlatformFont* font) {
  NSFont* ns_font = ValidateFont(font);
  [all_fonts_ removeObject:ns_font];
}

// Returns a dictionary with NSAttributedString style attributes that match the
// arguments.
NSDictionary* DarwinPlatform::GetAttributeDictionary(
    PlatformFont* platform_font,
    bool single_line,
    bool use_ellipsis,
    Color* foreground) {
  NSFont* ns_font = ValidateFont(platform_font);
  if (!ns_font) {
    return nil;
  }

  NSMutableDictionary* dict = [NSMutableDictionary dictionaryWithCapacity:3];
  NSMutableParagraphStyle* style =
      [[[NSParagraphStyle defaultParagraphStyle] mutableCopy] autorelease];

  NSLineBreakMode mode;
  if (single_line && use_ellipsis) {
    mode = NSLineBreakByTruncatingTail;
  } else if (single_line) {
    mode = NSLineBreakByClipping;
  } else {
    mode = NSLineBreakByWordWrapping;
  }

  [style setLineBreakMode:mode];
  [dict setObject:style forKey:NSParagraphStyleAttributeName];
  [dict setObject:ns_font forKey:NSFontAttributeName];

  if (foreground) {
    float a = foreground->alpha();
    float r = foreground->red();
    float g = foreground->green();
    float b = foreground->blue();

    a = a / 255.0f;
    r = r / 255.0f;
    g = g / 255.0f;
    b = b / 255.0f;

    NSColor* color = [NSColor colorWithCalibratedRed:r
                                               green:g
                                                blue:b
                                               alpha:a];
    [dict setObject:color forKey:NSForegroundColorAttributeName];
  }

  return dict;
}

// Measures the simple (single-font) text. Returns a bounding rectangle.
bool DarwinPlatform::MeasureSimpleText(PlatformFont* platform_font,
                                       const std::string &text,
                                       int wrapping_width,
                                       bool single_line,
                                       bool use_ellipsis,
                                       glint::Rectangle* bounds) {
  NSDictionary* attrs = GetAttributeDictionary(platform_font, single_line,
                                               use_ellipsis, NULL);
  if (!attrs || !bounds) {
    return false;
  }

  if (wrapping_width > kMaxWrappingWidth) {
    wrapping_width = kMaxWrappingWidth;
  }

  NSSize size = NSMakeSize(wrapping_width, MAXFLOAT);
  NSString* ns_text = [NSString stringWithUTF8String:text.c_str()];

  // boundingRectWithSize will return a rect valid in the current graphics
  // context, and the current context may not be compatible with the bitmap
  // that we will ultimately draw in, so create a temporary bitmap to set the
  // current context to.
  scoped_PlatformBitmap ns_bitmap(wrapping_width, 1);
  if (!ns_bitmap.get() || [all_bitmaps_ containsObject:ns_bitmap.get()] == NO) {
    assert(false && "Invalid platform bitmap");
    return false;
  }

  AutoRestoreGraphicsContext ar(ns_bitmap.get());

  int options = single_line ? 0 : NSStringDrawingUsesLineFragmentOrigin;

  NSRect rect = [ns_text boundingRectWithSize:size
                                      options:(NSStringDrawingOptions)options
                                   attributes:attrs];

  // HACK:
  // It seems when using NSStringDrawingUsesLineFragmentOrigin that drawing
  // into a rect of dimensions returned by boundingRectWithSize always wraps
  // the last character into another line; we get around this by padding the
  // width of the rect we return by 1.
  bounds->Set(0, 0, rect.size.width + 1, rect.size.height);

  return true;
}

// Draws the simple (single-font) text into specified bitmap on a transparent
// background. Clips the output to specified rectangle.
bool DarwinPlatform::DrawSimpleText(PlatformFont* platform_font,
                                    const std::string &text,
                                    Bitmap* target,
                                    const glint::Rectangle& clip,
                                    Color foreground,
                                    bool single_line,
                                    bool use_ellipsis) {
  NSDictionary* attrs = GetAttributeDictionary(platform_font, single_line,
                                               use_ellipsis, &foreground);

  if (!attrs || !target) {
    return false;
  }

  NSString* ns_text = [NSString stringWithUTF8String:text.c_str()];

  int width = clip.size().width;
  int height = clip.size().height;
  NSRect ns_rect = NSMakeRect(clip.left(), clip.top(), width, height);

  target->Fill(colors::kTransparent);

  NSBitmapImageRep* ns_bitmap =
      reinterpret_cast<NSBitmapImageRep*>(target->platform_bitmap());
  if (!ns_bitmap || [all_bitmaps_ containsObject:ns_bitmap] == NO) {
    assert(false && "Invalid platform bitmap");
    return false;
  }

  AutoRestoreGraphicsContext ar(ns_bitmap);

  int options = single_line ? 0 : NSStringDrawingUsesLineFragmentOrigin;

  [ns_text drawWithRect:ns_rect
                options:(NSStringDrawingOptions)options
             attributes:attrs];

  return true;
}

}  // namespace glint_darwin
