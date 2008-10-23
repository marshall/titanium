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

#include "glint/crossplatform/core_util.h"
#include "glint/include/point.h"
#include "glint/include/transform.h"
#include "glint/test/test.h"

namespace glint {

TEST(TransformBasicTests);

TEST_F(TransformBasicTests, TransformTests) {
  Transform identity;
  ASSERT_TRUE(identity.IsOffsetOnly());

  Transform manual_identity(1, 0, 0, 1, 0, 0);
  ASSERT_TRUE(identity.IsOffsetOnly());
  ASSERT_TRUE(identity.IsClose(manual_identity));
  ASSERT_TRUE(manual_identity.IsClose(identity));

  Vector test_vector(1.0f, 2.0f);
  Vector result = identity.TransformVector(test_vector);
  ASSERT_TRUE(Close(result.x, 1.0f));
  ASSERT_TRUE(Close(result.y, 2.0f));

  identity.Invert();
  result = identity.TransformVector(test_vector);
  ASSERT_TRUE(Close(result.x, 1.0f));
  ASSERT_TRUE(Close(result.y, 2.0f));

  Transform matrix;
  matrix.AddOffset(Vector(10.0f, 20.0f));
  ASSERT_TRUE(!identity.IsClose(matrix));
  ASSERT_TRUE(matrix.IsOffsetOnly());
  Vector offset = matrix.GetOffset();
  ASSERT_TRUE(offset.x == 10.0f && offset.y == 20.0f);

  result = matrix.TransformVector(test_vector);
  ASSERT_TRUE(Close(result.x, 1.0f + 10.0f));
  ASSERT_TRUE(Close(result.y, 2.0f + 20.0f));

  matrix.AddScale(Vector(3.0f, 4.0f));
  ASSERT_TRUE(!identity.IsClose(matrix));
  result = matrix.TransformVector(test_vector);
  ASSERT_TRUE(Close(result.x, (1.0f + 10.0f) * 3.0f));
  ASSERT_TRUE(Close(result.y, (2.0f + 20.0f) * 4.0f));

  matrix.AddRotation(3.1415926f / 2);  // 90 degrees
  Vector result_rotated = matrix.TransformVector(test_vector);
  ASSERT_TRUE(Close(result.x, result_rotated.y));
  ASSERT_TRUE(Close(result.y, -result_rotated.x));

  Transform inverted;
  inverted.Set(matrix);
  inverted.Invert();
  Vector original = inverted.TransformVector(result_rotated);
  ASSERT_TRUE(Close(original.x, test_vector.x));
  ASSERT_TRUE(Close(original.y, test_vector.y));

  matrix.AddPostTransform(inverted);
  ASSERT_TRUE(matrix.IsClose(identity));
}

}  // namespace glint



