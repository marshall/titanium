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

#include "glint/crossplatform/tile_plane.h"
#include "glint/test/test.h"

namespace glint {

const int kInfinity = Plane::Max_Coordinate;
const int kEmpty = Tile::EMPTY;
const int kSolid = Tile::FIRST_USER_TYPE;

void CheckTile(Tile* tile, int type, int left, int top, int right, int bottom) {
  ASSERT_TRUE(tile != NULL);
  ASSERT_EQ(tile->type, type);
  ASSERT_EQ(tile->x, left);
  ASSERT_EQ(tile->y, top);
  ASSERT_EQ(tile->right->x, right);
  ASSERT_EQ(tile->bottom->y, bottom);
}

TEST(TilePlaneTest);

TEST_F(TilePlaneTest, CreateEmptyPlane) {
  Plane* plane = new Plane();
  Tile *space_tile = plane->FromPoint(0, 0);
  CheckTile(space_tile, kEmpty, -kInfinity, -kInfinity, kInfinity, kInfinity);
  delete plane;
}

TEST_F(TilePlaneTest, CreateOneTile) {
  Plane* plane = new Plane();
  // This creates 5 tiles.
  Rectangle test(10, 10, 20, 20);
  plane->CreateTile(test, kSolid);
  Rectangle area(0, 0, 100, 100);
  AreaIterator iterator(plane, area);

  ASSERT_TRUE(iterator.GetNextTile());
  CheckTile(iterator.current(), kEmpty, -kInfinity, 20, kInfinity, kInfinity);

  ASSERT_TRUE(iterator.GetNextTile());
  CheckTile(iterator.current(), kEmpty, 20, 10, kInfinity, 20);


  ASSERT_TRUE(iterator.GetNextTile());
  CheckTile(iterator.current(), kSolid, 10, 10, 20, 20);

  ASSERT_TRUE(iterator.GetNextTile());
  CheckTile(iterator.current(), kEmpty, -kInfinity, 10, 10, 20);

  ASSERT_TRUE(iterator.GetNextTile());
  CheckTile(iterator.current(), kEmpty, -kInfinity, -kInfinity, kInfinity, 10);

  // No more tiles in this area.
  ASSERT_FALSE(iterator.GetNextTile());
  delete plane;
}

TEST_F(TilePlaneTest, CreateOverlappedTilesSimple) {
  Plane* plane = new Plane();
  // This creates 5 tiles.
  Rectangle test(10, 10, 40, 40);
  plane->CreateTile(test, kSolid);
  // This splits the first tile to stretch the overlappign area horizontally.
  // Result is 3 solid tiles.
  test.Set(0, 20, 15, 30);
  plane->CreateTile(test, kSolid);

  Rectangle area(0, 0, 100, 100);
  AreaIterator iterator(plane, area);

  ASSERT_TRUE(iterator.GetNextTile());
  CheckTile(iterator.current(), kEmpty, -kInfinity, 40, kInfinity, kInfinity);

  ASSERT_TRUE(iterator.GetNextTile());
  CheckTile(iterator.current(), kSolid, 10, 30, 40, 40);

  ASSERT_TRUE(iterator.GetNextTile());
  CheckTile(iterator.current(), kEmpty, -kInfinity, 30, 10, 40);

  ASSERT_TRUE(iterator.GetNextTile());
  CheckTile(iterator.current(), kSolid, 0, 20, 40, 30);

  ASSERT_TRUE(iterator.GetNextTile());
  CheckTile(iterator.current(), kEmpty, 40, 10, kInfinity, 40);

  ASSERT_TRUE(iterator.GetNextTile());
  CheckTile(iterator.current(), kSolid, 10, 10, 40, 20);

  ASSERT_TRUE(iterator.GetNextTile());
  CheckTile(iterator.current(), kEmpty, -kInfinity, 10, 10, 20);

  ASSERT_TRUE(iterator.GetNextTile());
  CheckTile(iterator.current(), kEmpty, -kInfinity, -kInfinity, kInfinity, 10);

  // No more tiles in this area.
  ASSERT_FALSE(iterator.GetNextTile());
  delete plane;
}

}  // namespace glint

