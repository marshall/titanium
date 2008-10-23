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

#include "glint/include/bitmap.h"
#include "glint/crossplatform/core_util.h"
#include "glint/include/rectangle.h"
#include "glint/include/transform.h"

namespace glint {

// Used for hi-precision integer math in interpolation routines.
// Defining it inside 'glint/local' namespace should avoid interference with
// possible outside definitions, even if the code that uses Glint is placing
// itself into 'glint' namespace for brievity.
namespace local {
#ifdef _MSC_VER
typedef __int64 int64;
#else
typedef long long int64;
#endif  // _MSC_VER
}  // namespace local

// Combines ARGB pixels with additional alpha.
// Formula (source is already premultiplied by its own alpha):
// target = source * alpha + target * (1 - source.alpha * alpha)
// 'alpha' is pre-shifted into 1..256 range (0..255 + 1) so the operations
// like this: 'dst = (src * alpha) >> 8' yield dst == src when original alpha
// is 255 and dst == 0 when original alpha is 0.
// (Ex Michael Herf http://www.stereopsis.com/doubleblend.html)
static void AlphaBlend(Color* target, Color source, uint32 alpha) {
  ASSERT(alpha <= 255);
  ASSERT(target);

  Color result;
  alpha += 1;
  uint32 target_alpha = 256 - ((source.alpha() * alpha) >> 8);

  result.set_red(
      static_cast<uint8>(((source.red() * alpha) >> 8) +
                         ((target->red() * target_alpha) >> 8)));

  result.set_green(
      static_cast<uint8>(((source.green() * alpha) >> 8) +
                         ((target->green() * target_alpha) >> 8)));

  result.set_blue(
      static_cast<uint8>(((source.blue() * alpha) >> 8) +
                         ((target->blue() * target_alpha) >> 8)));

  result.set_alpha(
      static_cast<uint8>(((source.alpha() * alpha) >> 8) +
                         ((target->alpha() * target_alpha) >> 8)));

  *target = result;
}

// Linear 1-D interpolation.
// Computes color between pixels a and b at a distance x:
//         <----x---->
//        a---------result---b
// We assume x is relative to distance between a and b and is in range 0..255.
// We move it to 1..256 range to facilitate easy multiplications.
// This also means that 'distance' between a and b is assumed to be 255.
// Formula: result = a * (1 - x) + b * x
static Color LinearInterpolation(Color a, Color b, uint32 x) {
  ASSERT(x <= 255);
  // Obvious optimization.
  if (a == b) {
    return a;
  }

  Color result;
  x += 1;
  uint32 one_less_x = 257 - x;  // also in 1..256 range

  result.set_red(
      static_cast<uint8>(((a.red() * one_less_x) + (b.red() * x)) >> 8));
  result.set_green(
      static_cast<uint8>(((a.green() * one_less_x) + (b.green() * x)) >> 8));
  result.set_blue(
      static_cast<uint8>(((a.blue() * one_less_x) + (b.blue() * x)) >> 8));
  result.set_alpha(
      static_cast<uint8>(((a.alpha() * one_less_x) + (b.alpha() * x)) >> 8));

  return result;
}

// Bilinear 2D interpolation. Computes color of a pixel between 4 pixels on
// a plane:
//
//    a<---x--->    b
//             |
//             y
//             |
//          result
//
//    c              d
//
// x and y are assumed in range 0..255 and are fractional distance (255 = 100%).
static Color BilinearInterpolation(Color a, Color b, Color c, Color d,
                                   int x, int y) {
  // Simple optimization - if a == b, c == d or all 4 are the same, skip
  // corresponding interpolations.
  Color top = (a == b ? a : LinearInterpolation(a, b, x));
  Color bottom = (c == d ? c : LinearInterpolation(c, d, x));
  Color result = (top == bottom ? top : LinearInterpolation(top, bottom, y));
  return result;
}

// Fills pixels between target_clip_start and target_clip_end of target
// with interpolated pixels from source. source and target can be of different
// length. For horizontal line, specify pixel_steps equal to 1, for
// vertical column - specify them to be corresponding stride.
// 'source' and 'target' are the beginning of line segments of corresponding
// bitmaps - for example, source + source_start gives the first pixel from
// source, and target + max(target_start, target_clip_start) gives the first
// actually filled pixel in target bitmap.
//              s_start   s_end            max_source_offset
//  source-> [-----|-------|----------------]
//                /        \.
//               /          \.
//  target-> [--|------------|----------------------------]
//            t_start       t_end
static void InterpolateLine(Color* source,
                            int source_start,
                            int source_end,
                            int source_pixel_step,
                            int max_source_offset,
                            Color* target,
                            int target_start,
                            int target_end,
                            int target_pixel_step,
                            int target_clip_start,
                            int target_clip_end,
                            uint32 alpha) {
  ASSERT(source_start >= 0 &&
         source_start < 0x8FFF &&
         source_end >= 0 &&
         source_end < 0x8FFF &&
         target_start >= 0 &&
         target_start < 0x8FFF &&
         target_end >= 0 &&
         target_end < 0x8FFF &&
         target_clip_start >= 0 &&
         target_clip_start < 0x8FFF &&
         target_clip_end >= 0 &&
         target_clip_end < 0x8FFF);
  ASSERT(alpha <= 255);

  // Check if nothing to draw.
  // First, intersect clip with size of target.
  if (target_clip_start < target_start) {
    target_clip_start = target_start;
  }
  if (target_clip_end > target_end) {
    target_clip_end = target_end;
  }
  // Now see if the intersection is not empty.
  if (target_clip_start >= target_clip_end)
    return;

  // Convert values to hi-resolution by multiplying by 256
  int source_length_hi = (source_end - source_start) << 8;
  int target_length_hi = (target_end - target_start) << 8;
  int source_start_hi = source_start << 8;

  for (int x = target_clip_start; x < target_clip_end; ++x) {
    int x_hi = (x - target_start) << 8;
    // Use 64 bit here since multiplication may overflow 32-bit int.
    local::int64 source_x_offset_64 =
        (static_cast<local::int64>(x_hi) * source_length_hi) / target_length_hi;
    int source_x_hi = source_start_hi + static_cast<int>(source_x_offset_64);
    int dx = source_x_hi & 0xFF;       // fractional location between pixels
    int src_index = (source_x_hi >> 8) * source_pixel_step;  // back in pixels
    Color* source_a = source + src_index;
    src_index += source_pixel_step;
    Color* source_b = (src_index > max_source_offset ?
                       source_a :
                       source + src_index);
    Color interpolated = LinearInterpolation(*source_a, *source_b, dx);
    AlphaBlend(target + (x * target_pixel_step), interpolated, alpha);
  }
}

Bitmap::Bitmap(const Point& origin, const Size& size)
  : platform_bitmap_(NULL),
    pixels_(NULL),
    origin_(origin),
    size_(size) {
  ASSERT(size_.width >= 0 && size_.height >= 0);

  if (size_.width <= 0 || size_.height <= 0)
    return;

  platform_bitmap_ = platform()->CreateBitmap(size_.width, size_.height);

  if (platform_bitmap_) {
    pixels_ = platform()->LockPixels(platform_bitmap_);
  }
}

Bitmap::~Bitmap() {
  if (platform_bitmap_)  {
    if (pixels_) {
      platform()->UnlockPixels(platform_bitmap_, pixels_);
      pixels_ = NULL;
    }
    platform()->DeleteBitmap(platform_bitmap_);
    platform_bitmap_ = NULL;
  }
}

bool Bitmap::GetBounds(Rectangle* bounds) const {
  ASSERT(bounds);
  if (!pixels_)
    return false;
  bounds->Set(origin_, size_);
  return true;
}

Color* Bitmap::GetPixelAt(int x, int y) const {
  if (!pixels_ ||
      x < origin_.x ||
      y < origin_.y ||
      x >= origin_.x + size_.width ||
      y >= origin_.y + size_.height)
    return NULL;

  return GetPixelAtUnsafe(x, y);
}

Color* Bitmap::GetPixelAtUnsafe(int x, int y) const {
  return pixels_ + (x - origin_.x) + size_.width * (y - origin_.y);
}

void Bitmap::PremultiplyAlpha() {
  if (!pixels_)
    return;
  int pixel_count = size_.width * size_.height;
  Color* pixel = pixels_;
  while (pixel_count > 0) {
    --pixel_count;
    pixel->Premultiply();
    ++pixel;
  }
}

bool Bitmap::Compose(const Bitmap& source,
                     const Rectangle& source_clip,
                     const Transform& to_target,
                     const Rectangle& target_clip,
                     uint32 alpha) {
  if (to_target.IsOffsetOnly()) {
    // If transform is just an offset
    Vector offset = to_target.GetOffset();
    Compose(source,
            source_clip,
            Point(Round(offset.x), Round(offset.y)),
            target_clip,
            alpha);
  } else {
    // TODO(dimich): Implement bilinear blend.
  }
  return true;
}

bool Bitmap::Compose(const Bitmap& source,
                     const Rectangle& source_clip,
                     const Point& offset_to_target,
                     const Rectangle& target_clip,
                     uint32 alpha) {
  // Find intersection of 4 rectangles - target area, target clip,
  // source area and source clip translated into target coordinate space.
  // The resulting rectangle covers the affected target pixels.
  Rectangle clip(origin_, size_);
  clip.Intersect(target_clip);
  Rectangle source_area(source.origin(), source.size());
  source_area.Intersect(source_clip);
  source_area.Offset(offset_to_target);
  clip.Intersect(source_area);

  if (clip.IsEmpty()) {
    return true;  // Nothing to compose, intersection is empty.
  }

  for (int y = clip.top(); y < clip.bottom(); ++y) {
    Color* target_pixel = GetPixelAt(clip.left(), y);
    Color* source_pixel =
        source.GetPixelAt(clip.left() - offset_to_target.x,
                          y - offset_to_target.y);
    for (int x = clip.left(); x < clip.right(); ++x) {
      AlphaBlend(target_pixel, *source_pixel, alpha);
      ++target_pixel;
      ++source_pixel;
    }
  }
  return true;
}

bool Bitmap::Fill(Color color) {
  Rectangle area(origin_, size_);
  return Fill(color, area);
}

bool Bitmap::Fill(Color color, const Rectangle& area) {
  if (!pixels_)
    return false;

  Rectangle clip(origin_, size_);
  clip.Intersect(area);
  if (clip.IsEmpty())
    return true;

  for (int y = clip.top(); y < clip.bottom(); ++y) {
    Color* pixel = GetPixelAt(clip.left(), y);
    for (int x = clip.left(); x < clip.right(); ++x) {
      *pixel = color;
      ++pixel;
    }
  }
  return true;
}

bool Bitmap::DrawNineGrid(const Bitmap& source,
                          const GridStops& source_stops,
                          const Rectangle& target_destination,
                          const Rectangle& target_clip,
                          uint32 alpha) {
  if (!pixels_)
    return false;

  // First, check if we have to render at all.
  Rectangle clip;
  clip.Set(target_clip);
  clip.Intersect(target_destination);
  if (clip.IsEmpty())
    return true;

  // 'stops' are in coordinates of the 'source'.
  // They should be entirely covered by source - it is an error if they don't.
  Rectangle source_rect(source.origin(), source.size());
  if (source_rect.left() > source_stops.x[0] ||
      source_rect.right() < source_stops.x[3] ||
      source_rect.top() > source_stops.y[0] ||
      source_rect.bottom() < source_stops.y[3]) {
#ifdef DEBUG
      Fill(colors::kRed, clip);
#endif
      return false;
  }

  // Destination stops (points that source stops will be mapped to).
  GridStops destination_stops;
  int middle = target_destination.size().width / 2;
  destination_stops.x[0] = target_destination.left();
  destination_stops.x[1] = target_destination.left() +
      min<int>(
          source_stops.x[1] - source_stops.x[0],
          middle);
  destination_stops.x[2] = target_destination.right() -
      min<int>(
          source_stops.x[3] - source_stops.x[2],
          target_destination.size().width - middle);
  destination_stops.x[3] = target_destination.right();

  middle = target_destination.size().height / 2;
  destination_stops.y[0] = target_destination.top();
  destination_stops.y[1] = target_destination.top() +
      min<int>(
          source_stops.y[1] - source_stops.y[0],
          middle);
  destination_stops.y[2] = target_destination.bottom() -
      min<int>(
          source_stops.y[3] - source_stops.y[2],
          target_destination.size().height - middle);
  destination_stops.y[3] = target_destination.bottom();

  // Transfer 4 corners to the target.
  for (int start_x = 0; start_x <= 2; start_x += 2) {
    for (int start_y = 0; start_y <= 2; start_y += 2) {
      Rectangle corner(source_stops.x[start_x],
                       source_stops.y[start_y],
                       source_stops.x[start_x + 1],
                       source_stops.y[start_y + 1]);
      Compose(source,
              corner,
              Point(destination_stops.x[start_x] - corner.left(),
                    destination_stops.y[start_y] - corner.top()),
              clip,
              alpha);
    }
  }

  // Edges should be scaled in one dimension.
  // Horizontal edges - pixel_step is 1.
  // Top edge goes from x[1],y[0] -> x[2],y[1]
  // Bottom edge goes from x[1],y[2] -> x[2],y[3]
  // Run loop 2 times with start_y = 0 and 2.
  for (int start_y = 0; start_y <= 2; start_y += 2) {
    Rectangle edge_clipped(destination_stops.x[1],
                           destination_stops.y[start_y],
                           destination_stops.x[2],
                           destination_stops.y[start_y + 1]);
    edge_clipped.Intersect(clip);
    for (int y = edge_clipped.top(); y < edge_clipped.bottom(); ++y) {
      int dy = y - destination_stops.y[start_y];
      Color* source_line = source.GetPixelAt(0, source_stops.y[start_y] + dy);
      // It's ok to use unsafe getter here because InterpolateLine only
      // writes between edge_clipped.left and edge_clipped.right.
      Color* target_line = GetPixelAtUnsafe(0,
                                            destination_stops.y[start_y] + dy);
      InterpolateLine(source_line,
                      source_stops.x[1],
                      source_stops.x[2],
                      // horizontal line - so the next pixel is 1 px away
                      1,
                      source.size().width,
                      target_line,
                      destination_stops.x[1],
                      destination_stops.x[2],
                      // horizontal line - so the next pixel is 1 px away
                      1,
                      edge_clipped.left(),
                      edge_clipped.right(),
                      alpha);
    }
  }

  // Vertical edges - pixel_step is width of a bitmap.
  // Left edge goes from x[0],y[1] -> x[1],y[2]
  // Right edge goes from x[2],y[1] -> x[3],y[2]
  // Run loop 2 times with start_x = 0 and 2.
  for (int start_x = 0; start_x <= 2; start_x += 2) {
    Rectangle edge_clipped(destination_stops.x[start_x],
                           destination_stops.y[1],
                           destination_stops.x[start_x + 1],
                           destination_stops.y[2]);
    edge_clipped.Intersect(clip);
    for (int x = edge_clipped.left(); x < edge_clipped.right(); ++x) {
      int dx = x - destination_stops.x[start_x];
      Color* source_line = source.GetPixelAt(source_stops.x[start_x] + dx, 0);
      // It's ok to use unsafe getter here because InterpolateLine only
      // writes between edge_clipped.top and edge_clipped.bottom.
      Color* target_line = GetPixelAtUnsafe(destination_stops.x[start_x] + dx,
                                            0);
      InterpolateLine(source_line,
                      source_stops.y[1],
                      source_stops.y[2],
                      // vertical line - so the next pixel is a width away
                      source.size().width,
                      // max offset in source
                      source.size().height * source.size().width,
                      target_line,
                      destination_stops.y[1],
                      destination_stops.y[2],
                      // vertical line - so the next pixel is a width away
                      size().width,
                      edge_clipped.top(),
                      edge_clipped.bottom(),
                      alpha);
    }
  }

  // Center portion of nine grid. One special case is when its source is
  // exactly 1 pixel. Then we optimize by using Fill rather then
  // bilinear-interpolated scale.
  Rectangle source_center_area(source_stops.x[1],
                               source_stops.y[1],
                               source_stops.x[2],
                               source_stops.y[2]);

  Rectangle target_center_area(destination_stops.x[1],
                               destination_stops.y[1],
                               destination_stops.x[2],
                               destination_stops.y[2]);

  if (source_center_area.size().width == 1 &&
      source_center_area.size().height == 1) {
    target_center_area.Intersect(clip);
    Fill(*source.GetPixelAt(source_center_area.left(),
                            source_center_area.top()),
         target_center_area);
  } else {
    BilinearScale(source, source_center_area, target_center_area, clip);
  }
  return true;
}

bool Bitmap::BilinearScale(const Bitmap& source,
                           const Rectangle& source_area,
                           const Rectangle& target_area,
                           const Rectangle& clip) {
  if (!pixels_)
    return false;

  Rectangle target_clip;
  target_clip.Set(target_area);
  target_clip.Intersect(clip);
  if (clip.IsEmpty() || source_area.IsEmpty())
    return true;

  int target_width_hi = target_area.size().width << 8;
  int target_height_hi = target_area.size().height << 8;
  int source_width_hi = source_area.size().width << 8;
  int source_height_hi = source_area.size().height << 8;
  int source_area_top_hi = source_area.top() << 8;
  int source_area_left_hi = source_area.left() << 8;

  for (int y = target_clip.top(); y < target_clip.bottom(); ++y) {
    int y_hi = (y - target_area.top()) << 8;
    local::int64 source_y_offset_64 =
        (static_cast<local::int64>(y_hi) * source_height_hi) / target_height_hi;
    int source_y_hi = source_area_top_hi +
                      static_cast<int>(source_y_offset_64);
    int source_y = source_y_hi >> 8;
    // If at the bottom boundary, use the same pixel as next in y direction.
    ASSERT(source_y < source.size().height);
    int offset_to_next_y =
        (source_y + 1 == source.size().height ? 0 : source.size().width);

    int dy = source_y_hi & 0xFF;
    Color* target_pixel = GetPixelAt(target_clip.left(), y);
    for (int x = target_clip.left(); x < target_clip.right(); ++x) {
      int x_hi = (x - target_area.left()) << 8;
      local::int64 source_x_offset_64 =
          (static_cast<local::int64>(x_hi) * source_width_hi) / target_width_hi;
      int source_x_hi = source_area_left_hi +
                        static_cast<int>(source_x_offset_64);
      int source_x = source_x_hi >> 8;
      int dx = source_x_hi & 0xFF;

      Color* source_pixel_a = source.GetPixelAt(source_x, source_y);
      Color* source_pixel_c = source_pixel_a + offset_to_next_y;
      // If at the right boundary, use the same pixel as next in x direction.
      ASSERT(source_x < source.size().width);
      int offset_to_next_x = (source_x + 1 == source.size().width ? 0 : 1);

      *target_pixel = BilinearInterpolation(
          *source_pixel_a,
          *(source_pixel_a + offset_to_next_x),
          *source_pixel_c,
          *(source_pixel_c + offset_to_next_x),
          dx,
          dy);

      ++target_pixel;
    }
  }
  return true;
}

bool Bitmap::DrawRectangle(const Rectangle& rectangle,
                           Color color,
                           uint32 alpha) {
  if (!pixels_)
    return false;

  Rectangle clip(origin_, size_);
  clip.Intersect(rectangle);
  if (clip.IsEmpty())
    return true;

  for (int y = clip.top(); y < clip.bottom(); ++y) {
    Color* pixel = GetPixelAt(clip.left(), y);
    for (int x = clip.left(); x < clip.right(); ++x) {
      AlphaBlend(pixel, color, alpha);
      ++pixel;
    }
  }
  return true;
}

// Helper class for doing some math with colors - allows accumulate (add)
// several ARGB color values and then convert accumulated value back to
// Color with normalization. See GaussianBlur below for usage.
class WideColor {
 public:
  WideColor() : color_low_(0), color_high_(0) {
  }

  WideColor(int constant) {
    constant &= 0xFF;
    color_low_ = color_high_ = constant + (constant << 16);
  }

  WideColor(Color color) {
    uint32 data = color.GetInternalRepresentation();
    color_low_  = data & 0xFF00FF;
    color_high_ = (data >> 8) & 0xFF00FF;
  }

  WideColor operator+(const WideColor& another) {
    WideColor result;
    result.color_low_ = color_low_ + another.color_low_;
    result.color_high_ = color_high_ + another.color_high_;
    return result;
  }

  void operator=(const WideColor& another) {
    color_low_ = another.color_low_;
    color_high_ = another.color_high_;
  }

  Color ToColor(uint32 scale_shift) {
    uint32 result = (color_low_ >> scale_shift) & 0xFF00FF;
    result |= ((color_high_ >> scale_shift) & 0xFF00FF) << 8;
    return Color::FromInternalRepresentation(result);
  }

  void Clear() {
    color_low_ = color_high_ = 0;
  }

private:
  uint32 color_low_;
  uint32 color_high_;
};

// Helper function for GaussianBlur below.
static Color* fetchColor(Bitmap* b,
                        int x,
                        int y) {
  Rectangle clip(b->origin(), b->size());
  if (x < clip.left()) {
    x = clip.left();
  }
  if (x >= clip.right()) {
    x = clip.right() - 1;
  }
  if (y < clip.top()) {
    y = clip.top();
  }
  if (y >= clip.bottom()) {
    y = clip.bottom() - 1;
  }
  return b->GetPixelAt(x, y);
}

// Follows "An efficient algorithm for Gaussian blur using finite-state
// machines" by Frederick M. Waltza and John W. V. Miller. See
// http://www-personal.engin.umd.umich.edu/~jwvm/ece581/21_GBlur.pdf.
// Implementation uses Pascal triangle as approximation of Gaussian kernel
// which allows having sliding buffers for accumulated values in vertical
// (column_state) and horizontal (row_state) directions. Because of this, each
// pass reads a pixel once and stores a pixel once in the original image.
bool Bitmap::GaussianBlur(int passes) {
  Rectangle clip(origin_, size_);
  if (clip.IsEmpty())
    return true;

  int width = size_.width;
  WideColor* column_state_0 = new WideColor[width];
  WideColor* column_state_1 = new WideColor[width];
  WideColor* column_state_2 = new WideColor[width];
  WideColor* column_state_3 = new WideColor[width];

  for (int pass = 0; pass < passes; pass++) {
    if (pass > 0) {
      for (int i = 0; i < width; ++i) {
        column_state_0[i].Clear();
        column_state_1[i].Clear();
        column_state_2[i].Clear();
        column_state_3[i].Clear();
      }
    }
    for(int y = clip.top(); y < clip.bottom(); ++y) {
      WideColor row_state_0, row_state_1, row_state_2, row_state_3;
      Color* target = GetPixelAt(clip.left(), y);
      for (int x = 0; x < width; ++x) {
        WideColor temp_1(*(fetchColor(this, x + clip.left() + 2, y + 2)));
        WideColor temp_2 = row_state_0 + temp_1;
        row_state_0 = temp_1;
        temp_1 = row_state_1 + temp_2;
        row_state_1 = temp_2;
        temp_2 = row_state_2 + temp_1;
        row_state_2 = temp_1;
        temp_1 = row_state_3 + temp_2;
        row_state_3 = temp_2;
        temp_2 = column_state_0[x] + temp_1;
        column_state_0[x] = temp_1;
        temp_1 = column_state_1[x] + temp_2;
        column_state_1[x] = temp_2;
        temp_2 = column_state_2[x] + temp_1;
        column_state_2[x] = temp_1;
        WideColor result(column_state_3[x] + temp_2 + 128);
        *target = result.ToColor(8);
        column_state_3[x] = temp_2;
        ++target;
      }
    }
  }
  delete [] column_state_0;
  delete [] column_state_1;
  delete [] column_state_2;
  delete [] column_state_3;
  return true;
}

}  // namespace glint

