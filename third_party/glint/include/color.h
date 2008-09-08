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

#ifndef GLINT_INCLUDE_COLOR_H__
#define GLINT_INCLUDE_COLOR_H__

#include <string>
#include "glint/include/types.h"

namespace glint {

// On OSX/x86 and Linux the bitmaps we use are ABGR (r,g,b,a in byte order)
// instead of ARGB.
#if (defined(OSX) && defined(__LITTLE_ENDIAN__)) || defined(LINUX)
#define SWITCH_B_R(x) \
  (((x) & 0xff000000) | (((x) & 0x00ff0000) >> 16) | \
   ((x) & 0x0000ff00) | (((x) & 0x000000ff) << 16))

#define MAKE_COLOR(x) SWITCH_B_R(x)
#define MAKE_UINT32(x) SWITCH_B_R(x)
#else
#define MAKE_COLOR(x) x
#define MAKE_UINT32(x) x
#endif

class Color {
 public:
  explicit Color() : color_(0x00000000) {
  }

  explicit Color(uint32 argb) : color_(MAKE_COLOR(argb)) {
  }

  Color(const Color& another) {
    color_ = another.color_;
  }

  bool operator==(const Color &s) const {
    return (s.color_ == color_);
  }

  bool operator!=(const Color &s) const {
    return !(*this == s);
  }

  uint8 red()   const { return red_; }
  uint8 green() const { return green_; }
  uint8 blue()  const { return blue_; }
  uint8 alpha() const { return alpha_; }

  void set_red(uint8 red)     { red_ = red; }
  void set_green(uint8 green) { green_ = green; }
  void set_blue(uint8 blue)   { blue_ = blue; }
  void set_alpha(uint8 alpha) { alpha_ = alpha; }

  uint32 GetARGB() const { return MAKE_UINT32(color_); }
  void SetARGB(uint32 argb) { color_ = MAKE_COLOR(argb); }

  void Set(uint32 a, uint32 r, uint32 g, uint32 b) {
    color_ = MAKE_COLOR((a << 24) | (r << 16) | (g << 8) | b);
  }

  // Returns internal representation of color which can be RGBA or BGRA or
  // anything else depending on the platform we are running. The only guarantee
  // is that it is 32 bits filled with 4 bytes representing 3 colors and alpha.
  // Which is which is not specified.
  uint32 GetInternalRepresentation() {
    return color_;
  }

  // Constructs the Color from internal representation.
  static Color FromInternalRepresentation(uint32 data) {
    Color result;
    result.color_ = data;
    return result;
  }

  void Premultiply() {
    int alpha = alpha_ + 1;
    red_   = (red_   * alpha) >> 8;
    green_ = (green_ * alpha) >> 8;
    blue_  = (blue_  * alpha) >> 8;
  }

  void Modulate(int alpha) {
    // Move it in 1..256 range so the subsequent multiplications are correct.
    alpha = alpha + 1;

    alpha_ = (alpha_ * alpha) >> 8;
    red_   = (red_ * alpha)   >> 8;
    green_ = (green_ * alpha) >> 8;
    blue_  = (blue_  * alpha) >> 8;
  }

  static bool FromName(const std::string& name, Color* color);

 private:
  union {
#if (defined(OSX) && defined(__LITTLE_ENDIAN__)) || defined(LINUX)
    // This case applies to OSX/x86 and Linux
    struct {
      uint8 red_, green_, blue_, alpha_;
    };
#elif defined(OSX) && defined(__BIG_ENDIAN__)
    // This case applies to OSX/PPC
    struct {
      uint8 alpha_, red_, green_, blue_;
    };
#else
    // This case applies to Win32
    struct {
      uint8 blue_, green_, red_, alpha_;
    };
#endif
    uint32 color_;
  };
};

// Colors known by their names.
// TODO(dimich): use this to implement Color::FromName and
// extend the XML parser for colors to accept color names in addition
// to numberic notation: background="red"
namespace colors {
  static const Color kTransparent = Color(0x00000000);
  static const Color kWhite       = Color(0xFFFFFFFF);
  static const Color kBlack       = Color(0xFF000000);
  static const Color kRed         = Color(0xFFFF0000);
  static const Color kYellow      = Color(0xFFFFFF00);
  static const Color kGreen       = Color(0xFF00FF00);
  static const Color kBlue        = Color(0xFF0000FF);

  static const uint8 kOpaqueAlpha       = 0xFF;
  static const uint8 kTransparentAlpha  = 0x00;
}  // namespace colors


}  // namespace glint

#endif  // GLINT_INCLUDE_COLOR_H__
