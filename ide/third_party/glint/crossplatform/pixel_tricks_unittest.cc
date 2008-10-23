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

// Unit tests for PixelTricks

// TODO(dimich): reuse as much of this unittest as possible. Make it verify
// basic bitmap operations rather then comparing MMX and C++ versions of
// algorithms.

#include "glint/include/color.h"
#include "glint/include/pixel_tricks.h"

namespace glint {

// We will accept pixels within one bit of each other as being the same
// Ideally we wouldn't have to do this, but there are subtle rounding error
// differences between the MMX and C++ versions.
static uint8 kPixelTolerance = 1;

class PixelTricksTest : public ::testing::Test {
 public:
  virtual ~PixelTricksTest() { }

 protected:
  PixelTricksTest() {
  }

  void InitBitmapForTest(Bitmap *bitmap) {
    int width = bitmap->width();
    int height = bitmap->height();

    Color white(0xffffffff);
    Vector center(width / 2, height / 2);
    int radius = (width < height) ? width / 4 : height / 4;

    ASSERT_TRUE(bitmap->ZeroImage();
    ASSERT_TRUE(bitmap->DrawCircle(center,
                                   radius,
                                   white));
    ASSERT_TRUE(bitmap->SetAlpha(0x80));
  }

  void InitLineForTest(Color *c, int count, const Color& src) {
    for (int i = 0; i < count; i++, c++) {
      c->a = src.a * i / count;
      c->r = src.r * i / count;
      c->g = src.g * i / count;
      c->b = src.b * i / count;
    }
  }

  bool CompareBitmaps(const Bitmap& one, const Bitmap& two) {
    if (one.width() != two.width() ||
        one.height() != two.height()) {
      printf("1.w is %d\n", one.width());
      printf("1.h is %d\n", one.height());
      printf("2.w is %d\n", two.width());
      printf("2.h is %d\n", two.height());
      return false;
    }

    for (int i = 0; i < one.width(); i++) {
      for (int j = 0; j < one.height(); j++) {
        Color* one_p = one.Pixel(i, j);
        Color* two_p = two.Pixel(i, j);
        if ((one_p->argb != two_p->argb) &&
            !CheckWithinTolerance(*one_p, *two_p)) {
          printf("At %d,%d 1.argb is %x, 2.argb is %x\n",
                 i, j,
                 (unsigned int)one.Pixel(i, j)->argb,
                 (unsigned int)two.Pixel(i, j)->argb);
          return false;
        }
      }
    }

    return true;
  }

  inline bool CheckWithinTolerance(Color one, Color two) {
    return
      (one.a >= two.a && one.a <= kPixelTolerance + two.a ||
      two.a >= one.a && two.a <= kPixelTolerance + one.a) &&
      (one.r >= two.r && one.r <= kPixelTolerance + two.r ||
      two.r >= one.r && two.r <= kPixelTolerance + one.r) &&
      (one.g >= two.g && one.g <= kPixelTolerance + two.g ||
      two.g >= one.g && two.g <= kPixelTolerance + one.g) &&
      (one.b >= two.b && one.b <= kPixelTolerance + two.b ||
      two.b >= one.b && two.b <= kPixelTolerance + one.b);
  }

  inline bool CheckComponents(Color one, Color two) {
    // The non-zero components are allowed to be different, but components
    // that are zero in one should also be zero in two, and vice versa.
    return
        ((one.r != 0 && two.r != 0) || one.r == two.r) &&
        ((one.g != 0 && two.g != 0) || one.g == two.g) &&
        ((one.b != 0 && two.b != 0) || one.b == two.b);
  }

 private:
  DISALLOW_EVIL_CONSTRUCTORS(PixelTricksTest);
};

// The first three tests test the testing code itself
TEST_F(PixelTricksTest, BasicSanity) {
  // Choosing some random non-square bitmap size
  Bitmap test(371, 153);
  ASSERT_TRUE(test.ZeroImage(Rect::InvalidRect()));
  ASSERT_TRUE(CompareBitmaps(test, test));
}

TEST_F(PixelTricksTest, BasicSanity2) {
  Bitmap test1(371, 153);
  Bitmap test2(371, 153);

  InitBitmapForTest(&test1);
  InitBitmapForTest(&test2);

  ASSERT_TRUE(CompareBitmaps(test1, test2));
}

TEST_F(PixelTricksTest, BasicSanity3) {
  // If pixels are the same they should be within tolerance
  ASSERT_TRUE(CheckWithinTolerance(0x80000000, 0x80000000));
  ASSERT_TRUE(CheckWithinTolerance(0x00800000, 0x00800000));
  ASSERT_TRUE(CheckWithinTolerance(0x00008000, 0x00008000));
  ASSERT_TRUE(CheckWithinTolerance(0x00000080, 0x00000080));

  // Within one pixel of each other
  ASSERT_TRUE(CheckWithinTolerance(0x7f000000, 0x80000000));
  ASSERT_TRUE(CheckWithinTolerance(0x007f0000, 0x00800000));
  ASSERT_TRUE(CheckWithinTolerance(0x00007f00, 0x00008000));
  ASSERT_TRUE(CheckWithinTolerance(0x0000007f, 0x00000080));

  ASSERT_TRUE(CheckWithinTolerance(0x80000000, 0x7f000000));
  ASSERT_TRUE(CheckWithinTolerance(0x00800000, 0x007f0000));
  ASSERT_TRUE(CheckWithinTolerance(0x00008000, 0x00007f00));
  ASSERT_TRUE(CheckWithinTolerance(0x00000080, 0x0000007f));

  // But not within two
  ASSERT_TRUE(!CheckWithinTolerance(0x7e000000, 0x80000000));
  ASSERT_TRUE(!CheckWithinTolerance(0x007e0000, 0x00800000));
  ASSERT_TRUE(!CheckWithinTolerance(0x00007e00, 0x00008000));
  ASSERT_TRUE(!CheckWithinTolerance(0x0000007e, 0x00000080));

  ASSERT_TRUE(!CheckWithinTolerance(0x80000000, 0x7e000000));
  ASSERT_TRUE(!CheckWithinTolerance(0x00800000, 0x007e0000));
  ASSERT_TRUE(!CheckWithinTolerance(0x00008000, 0x00007e00));
  ASSERT_TRUE(!CheckWithinTolerance(0x00000080, 0x0000007e));

  // Edge cases
  ASSERT_TRUE(!CheckWithinTolerance(0x00000000, 0xffffffff));
  ASSERT_TRUE(!CheckWithinTolerance(0xffffffff, 0x00000000));
}

// The following tests check that the RGB components don't get mixed up in
// various operations.

// We test these 3 colors through all operations
Color testColors[] = {colors::kRed, colors::kGreen, colors::kBlue};

// For operations that require an alpha amount, use a middle value
uint32 amount = 128;

TEST_F(PixelTricksTest, PixelMultiply) {
  for (size_t i = 0; i < ARRAY_SIZE(testColors); i++) {
    Color destination(testColors[i]);
    PixelTricks::PixelMultiply(testColors[i], &destination);
    ASSERT_TRUE(CheckComponents(testColors[i], destination));
  }
}

TEST_F(PixelTricksTest, ScaledPixelMultiply) {
  for (size_t i = 0; i < ARRAY_SIZE(testColors); i++) {
    Color destination(testColors[i]);
    PixelTricks::ScaledPixelMultiply(amount, testColors[i], &destination);
    ASSERT_TRUE(CheckComponents(testColors[i], destination));
  }
}

TEST_F(PixelTricksTest, PixelBlend) {
  for (size_t i = 0; i < ARRAY_SIZE(testColors); i++) {
    Color destination(colors::kTransparent);
    PixelTricks::PixelBlend(testColors[i], &destination);
    ASSERT_TRUE(CheckComponents(testColors[i], destination));
  }
}

TEST_F(PixelTricksTest, PixelBlend2) {
  for (size_t i = 0; i < ARRAY_SIZE(testColors); i++) {
    Color destination(colors::kTransparent);
    PixelTricks::PixelBlend(amount, testColors[i], &destination);
    ASSERT_TRUE(CheckComponents(testColors[i], destination));
  }
}

TEST_F(PixelTricksTest, ScaledPixelBlend) {
  for (size_t i = 0; i < ARRAY_SIZE(testColors); i++) {
    Color destination(colors::kTransparent);
    PixelTricks::ScaledPixelBlend(amount, testColors[i], &destination);
    ASSERT_TRUE(CheckComponents(testColors[i], destination));
  }
}

TEST_F(PixelTricksTest, ScaledPremulPixelBlend) {
  for (size_t i = 0; i < ARRAY_SIZE(testColors); i++) {
    Color destination(colors::kTransparent);
    PixelTricks::ScaledPremulPixelBlend(amount, testColors[i], &destination);
    ASSERT_TRUE(CheckComponents(testColors[i], destination));
  }
}

#if USE_MMX
// These tests check for (approximate) equivalence between the MMX and C++
// versions of the same operations.

// Simple blending of constant color throughout bitmap
TEST_F(PixelTricksTest, BlendPLine) {
  Bitmap test1(256, 256);
  Bitmap test2(256, 256);

  InitBitmapForTest(&test1);
  InitBitmapForTest(&test2);

  // Premultiplied full red
  Color red(0x80800000);

  test1.Premultiply();
  test2.Premultiply();

  for (int y = 0; y < 256; y++) {
    PixelTricks::BlendPLineMMX(test1.Pixel(0, y), red, 256);
    __asm emms;
    for (int x = 0; x < 256; x++) {
      PixelTricks::PremulPixelBlend(red, test2.Pixel(x, y));
    }
  }

  ASSERT_TRUE(CompareBitmaps(test1, test2));
}

// Alpha-scaled blending of variable color
TEST_F(PixelTricksTest, BlendPLine2) {
  Bitmap test1(256, 256);
  Bitmap test2(256, 256);

  Color src[256];

  InitBitmapForTest(&test1);
  InitBitmapForTest(&test2);

  // Premultiplied full red
  Color red(0x80800000);

  InitLineForTest(src, 256, red);

  test1.Premultiply();
  test2.Premultiply();

  for (int y = 0; y < 256; y++) {
    // alpha = y
    PixelTricks::BlendPLineMMX(y, test1.Pixel(0, y), src, 256);
    __asm emms;
    for (int x = 0; x < 256; x++) {
      if (src[x].a != 0x00) {
        PixelTricks::ScaledPremulPixelBlend(y, src[x], test2.Pixel(x, y));
      }
    }
  }

  ASSERT_TRUE(CompareBitmaps(test1, test2));
}

// Fast blur
TEST_F(PixelTricksTest, FastBlur) {
  Bitmap test1(371, 153);
  Bitmap test2(371, 153);

  InitBitmapForTest(&test1);
  InitBitmapForTest(&test2);

  ASSERT_TRUE(PixelTricks::FastBlur(&test1, 2.0, 2.0));
  ASSERT_TRUE(PixelTricks::FastBlurMMX(&test2, 2.0, 2.0));

  ASSERT_TRUE(CompareBitmaps(test1, test2));
}

#endif  // USE_MMX

}  // namespace glint
