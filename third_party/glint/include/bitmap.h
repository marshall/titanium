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

#ifndef GLINT_INCLUDE_BITMAP_H__
#define GLINT_INCLUDE_BITMAP_H__

#include "glint/include/base_object.h"
#include "glint/include/color.h"
#include "glint/include/point.h"
#include "glint/include/size.h"

namespace glint {

class PlatformBitmap;
class Rectangle;
class Transform;

// Describes 3x3 grid cut from potentially larger bitmap.
// x and y indicate coordinates on the bitmap used for nine-grid
// partitioning. Picture:
//        x[0] x[1]   x[2] x[3]
//  y[0]   +----+------+----+
//         |    |      |    |
//  y[1]   +----+------+----+
//         |    |      |    |
//  y[2]   +----+------+----+
//         |    |      |    |
//  y[3]   +----+------+----+
struct GridStops {
  int x[4];
  int y[4];
};

class Bitmap : public BaseObject {
 public:
  // The actually allocated pixels cover a rect (origin, size).
  // Attempt to draw outside will cause invalid memory access and crash.
  Bitmap(const Point& origin, const Size& size);
  ~Bitmap();

  Size size() const { return size_; }

  Point origin() const { return origin_; }
  void set_origin(const Point& origin) {
    origin_ = origin;
  }

  bool GetBounds(Rectangle* bounds) const;
  // Regular way to get a pointer to a pixel. Verifies that (x, y) actually
  // is covered by underlying bitmap, returns NULL otherwise. This prevents
  // dangerous issues when arbitrary (x, y) can result in a pointer to
  // unrelated but allocated memory.
  Color* GetPixelAt(int x, int y) const;

  // Multiplies rgb values of each pixel by alpha value for the same pixel.
  // This prepares the bitmap for subsequent composition into other images.
  void PremultiplyAlpha();

  // Alpha blending of 'source' into 'this' with affine transform, clip and
  // additional alpha multiple. If the transformed and clipped bounds of
  // source do not intersect the target, nothing happens.
  bool Compose(const Bitmap& source,
               const Rectangle& source_clip,
               const Transform& to_target,
               const Rectangle& target_clip,
               uint32 alpha);

  // Fills the whole bitmap with specified color.
  bool Fill(Color color);
  bool Fill(Color color, const Rectangle& area);

  // Partitions the 'bitmap' into 3x3 'nine-grid' according to specified
  // GridStops. Stretches/shrinks the nine-grid into the 'target_destination'.
  // Corners are rendered unmodified, while sides are stretched in one
  // dimension and center area is stretched in both dimensions if needed.
  bool DrawNineGrid(const Bitmap& source,
                    const GridStops& source_stops,
                    const Rectangle& target_destination,
                    const Rectangle& target_clip,
                    uint32 alpha);

  // Does Gaussian-like Blur with kernel size 5x5, where coefficients are
  // [1 4 6 4 1]. Performs specified number of passes.
  bool GaussianBlur(int passes);

  // Transfers pixels from source covered by 'source_area' into
  // the pixels covered by target_area doing necessary scale if areas are not
  // the same size. Obeys specified clip. Uses bilinear interpolation for
  // scaling.
  bool BilinearScale(const Bitmap& source,
                     const Rectangle& source_area,
                     const Rectangle& target_area,
                     const Rectangle& clip);

  bool DrawRectangle(const Rectangle& rectangle,
                     Color color,
                     uint32 alpha);

  PlatformBitmap* platform_bitmap() { return platform_bitmap_; }

 private:
  // Offset-only version of Compose.
  bool Compose(const Bitmap& source,
               const Rectangle& source_clip,
               const Point& offset_to_target,
               const Rectangle& target_clip,
               uint32 alpha);

  // Unsafe version of GetPixelAt. Does not do any checks. Use with caution.
  Color* GetPixelAtUnsafe(int x, int y) const;

  PlatformBitmap* platform_bitmap_;
  Color* pixels_;
  Point origin_;
  Size size_;
};

}  // namespace glint

#endif  // GLINT_INCLUDE_BITMAP_H__

