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

#include "glint/include/color.h"
#include "glint/crossplatform/tile_plane.h"

namespace glint {

TEST_GLINT_F(ColorTest);

static const struct {
  uint32 color_value;
  uint8 components[4];
} test_inputs[] = {
  {colors::kTransparent, {0, 0, 0, 0}},
  {colors::kWhite, {0xff, 0xff, 0xff, 0xff}},
  {colors::kBlack, {0xff, 0, 0, 0}},
  {colors::kRed, {0xff, 0xff, 0, 0}},
  {colors::kYellow, {0xff, 0xff, 0xff, 0}},
  {colors::kGreen, {0xff, 0, 0xff, 0}},
  {colors::kBlue, {0xff, 0, 0, 0xff}}
};

// This test checks that the order of components in the glint::Color
// struct matches the transform used in it's constructor.

TEST_F(ColorTest, CheckPredefined) {
  for (size_t i = 0; i < ARRAY_SIZE(test_inputs); i++) {
    // This assignment uses the transform (MAKE_COLOR).
    Color color = test_inputs[i].color_value;

    // Now check that all components got the expected values.
    ASSERT_EQ(color.a, test_inputs[i].components[0]);
    ASSERT_EQ(color.r, test_inputs[i].components[1]);
    ASSERT_EQ(color.g, test_inputs[i].components[2]);
    ASSERT_EQ(color.b, test_inputs[i].components[3]);
  }
}

}  // namespace glint
