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

#include "glint/include/rectangle.h"
#include "glint/include/transform.h"
#include "glint/test/test.h"

namespace glint {

TEST(RectangleBasicTests);

TEST_F(RectangleBasicTests, RectangleTests) {
  Rectangle empty;
  ASSERT_TRUE(empty.IsEmpty());
  ASSERT_TRUE(empty.left() == 0 &&
       empty.top() == 0 &&
       empty.right() == 0 &&
       empty.bottom() == 0);
  ASSERT_TRUE(!empty.Contains(Point(0, 0)));

  Rectangle rect(0, 10, 100, 120);
  ASSERT_TRUE(!rect.IsEmpty());

  // Copy.
  Rectangle another;
  another.Set(rect);
  ASSERT_TRUE(another.left() == 0 &&
       another.top() == 10 &&
       another.right() == 100 &&
       another.bottom() == 120);

  // Union
  another.Set(-10, -20, 10, 20);
  rect.Union(another);
  ASSERT_TRUE(rect.left() == -10 &&
       rect.top() == -20 &&
       rect.right() == 100 &&
       rect.bottom() == 120);

  rect.Intersect(another);
  ASSERT_TRUE(rect.left() == another.left() &&
       rect.top() == another.top() &&
       rect.right() == another.right() &&
       rect.bottom() == another.bottom());

  Rectangle save;
  save.Set(rect);
  rect.Intersect(empty);
  ASSERT_TRUE(rect.IsEmpty());
  ASSERT_TRUE(!rect.IsEqual(save));

  rect.Reset();
  another.Union(rect);
  ASSERT_TRUE(another.left() == -10 &&
       another.top() == -20 &&
       another.right() == 10 &&
       another.bottom() == 20);

  rect.SetHuge();
  ASSERT_TRUE(rect.IsHuge());
  ASSERT_TRUE(!rect.IsEmpty());
  ASSERT_TRUE(rect.Contains(Point(0, 0)));

  Transform transform;
  transform.AddScale(Vector(1E-10f, 1E-20f));
  Rectangle transformed;
  transform.TransformRectangle(rect, &transformed);
  ASSERT_TRUE(transformed.IsHuge());

  rect.SetHuge();
  another.Intersect(rect);
  ASSERT_TRUE(another.left() == -10 &&
       another.top() == -20 &&
       another.right() == 10 &&
       another.bottom() == 20);
}

}  // namespace glint



