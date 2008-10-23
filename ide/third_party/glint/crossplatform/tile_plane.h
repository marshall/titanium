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

#ifndef GLINT_CROSSPLATFORM_TILE_PLANE_H__
#define GLINT_CROSSPLATFORM_TILE_PLANE_H__

#include "glint/include/point.h"
#include "glint/include/rectangle.h"
#include "glint/include/size.h"

// Plane is a flat surface filled with Tiles. Tiles never overlap.
// Also, every point is always covered by some tile (even if it is
// "EMPTY" tile). Efficiently-suported operations include finding tile by (x, y)
// create a new tile overlapping some other tiles and iterating tiles in
// rectangular area.

namespace glint {

struct Tile {
 public:
  enum TileTypes {
    EMPTY = 0,
    BORDER_1,
    BORDER_2,
    FIRST_USER_TYPE,
  };

  Point origin() {
    return Point(x, y);
  }

  Size size() {
    return Size(right->x - x, bottom->y - y);
  }

  // 'type' of the tile - some are predefined (see TileTypes).
  // Tiles of the same type are getting merged with each other.
  int type;

  // Coordinates of top-left corner of the tile.
  int x;
  int y;

  // Pointers to tiles joining this one at top-left corner and at right-bottom
  // corner. For example, 'left' points to a leftmost tile at the top
  // of this one, 'top' points to a topmost tile at the left, etc.
  Tile* left;
  Tile* top;
  Tile* right;
  Tile* bottom;

  // Used by Plane::CreateTile.
  bool marked;
 private:
  DISALLOW_EVIL_CONSTRUCTORS(Tile);
};

// When tiles with different types overlap, this callback is used to obtain
// a resulting type.
typedef int (*ResolveTypeCallback)(int oldType, int newType);

class Plane {
 public:
  static const int Max_Coordinate = 100000000;

  Plane();
  ~Plane();

  // Finds a tile which covers point (x, y). Always returns non-NULL.
  Tile* FromPoint(int x, int y);

  // Creates a new tile, potentially canonicalizes the result by
  // merging tiles of the same type and stretching them in horizontal direction.
  // In overlapping areas, the plane assumes new tile type.
  void CreateTile(const Rectangle& bounds, int type);

  // Creates a new tile, potentially canonicalizes the result by
  // merging tiles of the same type and stretching them in horizontal direction.
  // In overlapping areas, the resolve_type callback is called to
  // determine the resulting tile type.
  void CreateTile(const Rectangle& bounds,
                  int type,
                  ResolveTypeCallback resolve_type);

 private:
  void AllocateTileCache();
  Tile* NewTile(int type, int x, int y);
  void ReleaseTile(Tile* t);
  bool MergeVertically(Tile* t);
  bool MergeHorizontally(Tile* t);
  void MergeNeighboursVertically(Tile* t);
  void MergeNeighboursHorizontally(Tile* t, int ymax);
  void CanonicalizeAround(Tile* t);
  Tile* Cutout(Tile* tile, const Rectangle& cutter);
  void SplitVertically(Tile* t, int xline);
  void SplitHorizontally(Tile* t, int yline);
  void SplitNeighboursHorizontally(Tile* t, int tp, int y, int ymax);

  Tile* last_found_;
  static Tile* tile_cache_;
  DISALLOW_EVIL_CONSTRUCTORS(Plane);
};

// Iterates a rectangular area and reports all tiles that intersect with it.
class AreaIterator {
 public:
  AreaIterator(Plane* plane, const Rectangle& area);
  Tile* GetNextTile();
  void Reset();

  Tile* current() const {
    return current_;
  }

 private:
  enum IterationState {
    FIRST_ROOT,
    FIRST_CHILD,
    NEXT_SIBLING,
    FINISHED,
  };

  Plane* plane_;
  Rectangle area_;
  Tile* current_;
  IterationState state_;
  Tile* next_;
  DISALLOW_EVIL_CONSTRUCTORS(AreaIterator);
};

}  // namespace glint

#endif  // GLINT_CROSSPLATFORM_TILE_PLANE_H__



