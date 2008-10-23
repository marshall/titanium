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

#import "glint/include/bitmap.h"
#import "glint/include/simple_text.h"
#import "glint/mac/darwin_platform.h"
#import "glint/test/test.h"

namespace glint_darwin {

using namespace glint;

TEST(DarwinPlatformTest);

TEST_F(DarwinPlatformTest, CreateBitmap) {
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  int width = 10;
  int height = 20;
  scoped_PlatformBitmap bitmap(width, height);
  ASSERT_TRUE([reinterpret_cast<id>(bitmap.get())
                isKindOfClass:[NSBitmapImageRep class]] == YES);

  NSBitmapImageRep* ns_bitmap = bitmap.get();

  ASSERT_EQ([ns_bitmap pixelsWide], width);
  ASSERT_EQ([ns_bitmap pixelsHigh], height);

  [pool release];
}

static const struct {
  const char* file_name;
  unsigned char expected_pixel[4];  // a, r, g, b
} load_from_file_data[] = {
  {"red_opaque.png", {0xff, 0xff, 0, 0}},
  {"red_transparent.png", {0x7f, 0x7f, 0, 0}},
  {"green_opaque.png", {0xff, 0, 0xff, 0}},
  {"green_transparent.png", {0x7f, 0, 0x7f, 0}},
  {"blue_opaque.png", {0xff, 0, 0, 0xff}},
  {"blue_transparent.png", {0x7f, 0, 0, 0x7f}}
};

TEST_F(DarwinPlatformTest, LoadFromFile) {
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  Bitmap *temp = NULL;

  for (size_t i = 0; i < ARRAYSIZE(load_from_file_data); i++) {
    const char* file_name = load_from_file_data[i].file_name;

    NSString* resource_name = [NSString stringWithUTF8String:file_name];
    NSString* path = [[NSBundle mainBundle] pathForResource:resource_name
                                                     ofType:nil];
    const char* path_cstr = [path fileSystemRepresentation];
    ASSERT_TRUE(path_cstr);
    ASSERT_TRUE(platform()->LoadBitmapFromFile(path_cstr, &temp));

    ASSERT_EQ(load_from_file_data[i].expected_pixel[0],
              temp->GetPixelAt(0,0)->alpha());
    ASSERT_EQ(load_from_file_data[i].expected_pixel[1],
              temp->GetPixelAt(0,0)->red());
    ASSERT_EQ(load_from_file_data[i].expected_pixel[2],
              temp->GetPixelAt(0,0)->green());
    ASSERT_EQ(load_from_file_data[i].expected_pixel[3],
              temp->GetPixelAt(0,0)->blue());
  }

  [pool release];
}

TEST_F(DarwinPlatformTest, DefaultFont) {
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

  PlatformFont* default_platform_font = platform()->GetDefaultFont();

  ASSERT_TRUE([reinterpret_cast<id>(default_platform_font)
                isKindOfClass:[NSFont class]] == YES);
  NSFont* default_font = reinterpret_cast<NSFont*>(default_platform_font);

  FontDescription default_font_description;
  platform()->GetDefaultFontDescription(&default_font_description);

  PlatformFont* default_platform_font_from_description =
      platform()->CreateFontFromDescription(default_font_description);

  ASSERT_TRUE(
      [reinterpret_cast<id>(default_platform_font_from_description)
           isKindOfClass:[NSFont class]] == YES);
  NSFont* default_font_from_description = reinterpret_cast<NSFont*>(
      default_platform_font_from_description);

  ASSERT_TRUE([default_font isEqual:default_font_from_description]);

  platform()->ReleaseFont(default_platform_font_from_description);
  platform()->ReleaseFont(default_platform_font);
  [pool release];
}

}  // namespace glint_darwin
