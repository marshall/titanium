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
#include "glint/crossplatform/core_util.h"
#include "glint/include/allocator.h"

namespace glint {

static const int PreallocatedTiles = 256;

Tile* Plane::tile_cache_ = NULL;

void Plane::AllocateTileCache() {
  tile_cache_ = reinterpret_cast<Tile*>(
      Allocator::Allocate(PreallocatedTiles * sizeof(Tile)));

  Tile* tile = tile_cache_;

  for (int i = 1; i < PreallocatedTiles ; ++i) {
    tile->bottom = tile + 1;
    tile = tile->bottom;
  }

  tile->bottom = NULL;
}

Tile* Plane::NewTile(int type, int x, int y) {
  if (tile_cache_ == NULL) {
    AllocateTileCache();
  }
  Tile* tile = tile_cache_;
  tile_cache_ = tile->bottom;
  tile->x = x;
  tile->y = y;
  tile->type = type;
  tile->top = tile->bottom = tile->left = tile->right = NULL;
  tile->marked = false;
  return tile;
}

void Plane::ReleaseTile(Tile* tile) {
  if (last_found_ == tile) {
    last_found_ = tile->left;
  }
  tile->bottom = tile_cache_;
  tile_cache_ = tile;
}

// Creates an initial tile structure (2D ordered list) with special "border"
// tiles that allow us not to check for NULL at the ends of the lists.
Plane::Plane() {
  Tile* tile = NewTile(Tile::EMPTY, -Max_Coordinate, -Max_Coordinate);
  tile->top = NewTile(Tile::BORDER_1, -Max_Coordinate - 1, -Max_Coordinate - 1);
  tile->bottom = NewTile(Tile::BORDER_1, -Max_Coordinate - 1, Max_Coordinate);
  tile->left = NewTile(Tile::BORDER_1, -Max_Coordinate - 1, -Max_Coordinate);
  tile->right = NewTile(Tile::BORDER_1, Max_Coordinate, -Max_Coordinate);

  tile->top->bottom = tile->right;
  tile->left->top = tile->right->top = tile->top;
  tile->left->right = tile->right->left = tile;
  tile->bottom->top = tile->left;
  tile->left->bottom = tile->right->bottom = tile->bottom;

  tile->top->top =
      NewTile(Tile::BORDER_2, -Max_Coordinate - 2, -Max_Coordinate - 2);
  tile->bottom->bottom =
      NewTile(Tile::BORDER_2, -Max_Coordinate - 2, Max_Coordinate + 1);
  tile->left->left = tile->top->left = tile->bottom->left =
      NewTile(Tile::BORDER_2, -Max_Coordinate - 2, -Max_Coordinate - 1);
  tile->right->right = tile->bottom->right = tile->top->right =
      NewTile(Tile::BORDER_2, Max_Coordinate + 1, -Max_Coordinate - 1);

  tile->left->left->top = tile->top->right->top = tile->top->top;
  tile->right->right->bottom = tile->left->left->bottom = tile->bottom->bottom;
  tile->bottom->bottom->top = tile->left->left;
  tile->top->top->bottom = tile->right->right;
  tile->left->left->right = tile->bottom;
  tile->right->right->left = tile->top;

  last_found_ = tile;
}

Tile* Plane::FromPoint(int x, int y) {
  Tile* tile = last_found_;
  ASSERT(tile);

  while (tile->x > x ||
         tile->right->x <= x ||
         tile->y > y ||
         tile->bottom->y <= y) {
    while (tile->x > x) {
      tile = tile->left;
    }
    while (tile->y > y) {
      tile = tile->top;
    }
    while (tile->right->x <= x) {
      tile = tile->right;
    }
    while (tile->bottom->y <= y) {
      tile = tile->bottom;
    }
  }
  last_found_ = tile;
  return tile;
}

AreaIterator::AreaIterator(Plane* plane, const Rectangle& area)
  : plane_(plane),
    current_(NULL),
    state_(FIRST_ROOT),
    next_(NULL) {
  ASSERT(plane);
  area_.Set(area);
}

Tile* AreaIterator::GetNextTile() {
  // Note: the set of tiles overlapping the area is basically a forest
  // with roots overlapping left boundary. Iterate those trees, depth first,
  // bottom root first.
  if (state_ == FINISHED) {
    current_ = NULL;
    return NULL;
  }

  if (state_ == FIRST_ROOT) {
    next_ = plane_->FromPoint(area_.left(), area_.bottom() - 1);
    state_ = FIRST_CHILD;
  }

  while (true) {
    if (state_ != NEXT_SIBLING) {
      // find lowest child
      while (true) {
        Tile* s = next_->right;
        if (s->x >= area_.right())
          break;  // no children in the area

        // get back into area if step right threw us out
        while (s->y >= area_.bottom()) {
          s = s->top;
        }

        if ((s->y <= area_.top()) || (s->left != next_))
          break;  // no children in the area

        next_ = s;  // go further right (to grandchildren)
      }
      // next_ is leftmost child (may be root)
    }

    while (true) {
      current_ = next_;
      state_ = FIRST_CHILD;

      if (next_->y <= area_.top()) {  // top roots
        if (next_->right->x >= area_.right()) {  // last root
          state_ = FINISHED;
          return current_;
        }

        next_ = next_->right;

        while (next_->y > area_.top()) {
          next_ = next_->top;
        }

        return current_;
      }

      if (next_->x <= area_.left()) {  // left roots
        next_ = next_->top;
        while (next_->right->x <= area_.left()) {
          next_ = next_->right;
        }
        return current_;
      }

      if ((next_->top->left == next_->left) &&
          (next_->top->y > area_.top())) {  // sibling
        next_ = next_->top;
        return current_;
      }

      next_ = next_->left;
      state_ = NEXT_SIBLING;
      return current_;
    }
  }
}

void AreaIterator::Reset() {
  current_ = NULL;
  state_ = FIRST_ROOT;
}

static void UpdateLeftSide(Tile* to) {
  Tile* tile = to->left;
  int limit = to->bottom->y;
  while (tile->bottom->y <= limit) {
    tile->right = to;
    tile = tile->bottom;
  }
}

static void UpdateRightSide(Tile* to) {
  Tile* tile = to->right;
  int limit = to->y;
  while (tile->y >= limit) {
    tile->left = to;
    tile = tile->top;
  }
}

static void UpdateTopSide(Tile* to) {
  Tile* tile = to->top;
  int limit = to->right->x;
  while (tile->right->x <= limit) {
    tile->bottom = to;
    tile = tile->right;
  }
}

static void UpdateBottomSide(Tile* to) {
  Tile* tile = to->bottom;
  int limit = to->x;
  while (tile->x >= limit) {
    tile->top = to;
    tile = tile->left;
  }
}

void Plane::SplitVertically(Tile* tile, int xline) {
  Tile* new_tile = NewTile(tile->type, xline, tile->y);
  Tile* pt = tile->top;
  while (pt->right->x <= xline) {
    pt = pt->right;
  }
  new_tile->top = pt;
  new_tile->right = tile->right;
  new_tile->bottom = tile->bottom;
  new_tile->left = tile;
  pt = tile->bottom;
  while (pt->x >= xline) {
    pt = pt->left;
  }
  tile->bottom = pt;
  tile->right = new_tile;
  UpdateRightSide(new_tile);
  UpdateBottomSide(new_tile);
  UpdateTopSide(new_tile);
}

void Plane::SplitHorizontally(Tile* tile, int yline) {
  if ((yline >= tile->bottom->y) || (yline <= tile->y))
    return;
  Tile* new_tile = NewTile(tile->type, tile->x, yline);
  Tile* pt = tile->left;
  while (pt->bottom->y <= yline) {
    pt = pt->bottom;
  }
  new_tile->left = pt;
  new_tile->bottom = tile->bottom;
  new_tile->right = tile->right;
  new_tile->top = tile;
  pt = tile->right;
  while (pt->y >= yline) {
    pt = pt->top;
  }
  tile->right = pt;
  tile->bottom = new_tile;
  UpdateRightSide(new_tile);
  UpdateBottomSide(new_tile);
  UpdateLeftSide(new_tile);
}

bool Plane::MergeVertically(Tile* pt) {
  Tile* pt1 = pt->bottom;
  if (pt->type   == pt1->type &&
      !pt->marked &&
      !pt1->marked &&
      pt->x == pt1->x &&
      pt->right->x == pt1->right->x) {
    pt->bottom = pt1->bottom;
    pt->right = pt1->right;
    UpdateLeftSide(pt);
    UpdateBottomSide(pt);
    UpdateRightSide(pt);
    ReleaseTile(pt1);
    return true;
  }
  return false;
}

bool Plane::MergeHorizontally(Tile* pt) {
  Tile* pt1 = pt->right;
  if (pt->type   == pt1->type &&
      !pt->marked &&
      !pt1->marked &&
      pt->y == pt1->y &&
      pt->bottom->y == pt1->bottom->y) {
    pt->bottom = pt1->bottom;
    pt->right = pt1->right;
    UpdateBottomSide(pt);
    UpdateRightSide(pt);
    UpdateTopSide(pt);
    ReleaseTile(pt1);
    return true;
  }
  return false;
}

static bool SameType(Tile* tile, int type) {
  return (!tile->marked) && (tile->type == type);
}

void Plane::SplitNeighboursHorizontally(Tile* tile, int tp, int y, int ymax) {
  Tile* pt = tile->left;
  Tile* s = tile->right;
  if (SameType(pt, tp) && (pt->y < y)) {
    SplitHorizontally(pt, y);
    pt = pt->bottom;
  }

  while (true) {
    if (pt->bottom->y == ymax)
      break;

    if (pt->bottom->y > ymax) {
      if (SameType(pt, tp)) {
          SplitHorizontally(pt, ymax);
      }
      break;
    } else {
      if (SameType(pt, tp) || SameType(pt->bottom, tp)) {
        SplitHorizontally(pt->right, pt->bottom->y);
        if (SameType(pt->right->right, tp)) {
            SplitHorizontally(pt->right->right, pt->bottom->y);
        }
      }
    }
    pt = pt->bottom;
  }

  pt = s;

  while (pt->bottom->y < ymax) {
    pt = pt->bottom;
  }

  if ((pt->bottom->y > ymax) && SameType(pt, tp)) {
    SplitHorizontally(pt, ymax);
  }

  while (pt->y > y) {
    if (SameType(pt, tp) || SameType(pt->top, tp)) {
      SplitHorizontally(pt->left, pt->y);
      if (SameType(pt->left->left, tp)) {
          SplitHorizontally(pt->left->left, pt->y);
      }
    }
    pt = pt->top;
  }

  if ((pt->y < y) && SameType(pt, tp)) {
    SplitHorizontally(pt, y);
  }
}

void Plane::MergeNeighboursVertically(Tile* tile) {
  Tile* s = NULL;
  Tile* pt = tile->left;

  if (pt->top->bottom == pt) {
    MergeVertically(pt->top);
    pt = tile->left;
  }

  int limit = tile->bottom->y;
  while (pt->bottom->y <= limit) {
    s = pt->bottom;
    // If no merge happen, advance, otherwise try to merge with next down.
    if (!MergeVertically(pt))
      pt = s;
  }

  pt = tile->right;

  while (pt->bottom->y >= tile->y) {
    MergeVertically(pt);
    pt = pt->top;
  }
}

void Plane::MergeNeighboursHorizontally(Tile* tile, int ymax) {
  Tile* upt = tile->top;
  while (tile->y < ymax) {
    Tile* s = tile->bottom;
    MergeHorizontally(tile);
    MergeHorizontally(tile->left);
    tile = s;
  }
  Tile* dwt = tile->top;
  MergeVertically(dwt);
  MergeVertically(upt);
}

void Plane::CanonicalizeAround(Tile* tile) {
  int y = tile->y;

  // Try to merge with tile below.
  MergeVertically(tile);

  // Try to merge with tile above.
  Tile* s = tile;
  tile = tile->top;
  MergeVertically(tile);

  // If we couldn't merge with tile above, restore 'tile'.
  if (tile->bottom->y == y) {
    tile = s;
  } else {
  // Otherwise, update our lower boundary from the new merged tile.
    y = tile->y;
  }
  int bottom = tile->bottom->y;
  int tp = tile->type;

  MergeNeighboursVertically(tile);
  SplitNeighboursHorizontally(tile, tp, y, bottom);
  MergeNeighboursHorizontally(tile, bottom);
}

// Cuts the tile using 'cutter', returns central tile.
// Produces max 4 new tiles.
Tile* Plane::Cutout(Tile* tile, const Rectangle& cutter) {
  if (tile->bottom->y > cutter.bottom()) {
    SplitHorizontally(tile, cutter.bottom());
  }

  if (tile->y < cutter.top()) {
    SplitHorizontally(tile, cutter.top());
    tile = tile->bottom;
  }

  if (tile->x < cutter.left()) {
    SplitVertically(tile, cutter.left());
    tile = tile->right;
  }

  if (tile->right->x > cutter.right()) {
    SplitVertically(tile, cutter.right());
  }

  return tile;
}

static int painter(int old_type, int new_type) {
  return new_type;
}

void Plane::CreateTile(const Rectangle& bounds, int type) {
  CreateTile(bounds, type, painter);
}

void Plane::CreateTile(const Rectangle& bounds,
                       int type,
                       ResolveTypeCallback resolve_type) {
  if (bounds.IsEmpty())
    return;

  AreaIterator iterator(this, bounds);

  // Mark all tiles overlapping with bounds so we can
  // process them one-by-one - merging works with non-marked tiles only,
  // which makes it possible to iterate and modify the area at the same time.
  while (iterator.GetNextTile()) {
    iterator.current()->marked = true;
  }

  iterator.Reset();
  while (iterator.GetNextTile()) {
    Tile* tile = iterator.current();
    tile->marked = false;

    int new_type = resolve_type(tile->type, type);

    // If the current tile is of target type already, canonicalize around.
    if (new_type == tile->type) {
      CanonicalizeAround(tile);
    } else {
      // Tile is not of the target type - cut a hole in it if needed.
      tile = Cutout(iterator.current(), bounds);
      // Fill the hole with target type.
      tile->type = new_type;
      // Canonicalize the result.
      CanonicalizeAround(tile);
    }
  }
}


Plane::~Plane() {
  Tile* top = (FromPoint(-Max_Coordinate, -Max_Coordinate))->top;
  Tile* right = top->bottom;
  Tile* bottom = right->bottom;
  Tile* left = bottom->top;

  Rectangle whole_plane(-Max_Coordinate,
                        -Max_Coordinate,
                        Max_Coordinate,
                        Max_Coordinate);

  AreaIterator iterator(this, whole_plane);

  while (iterator.GetNextTile()) {
    iterator.current()->bottom = tile_cache_;
    tile_cache_ = iterator.current();
  }

  last_found_ = NULL;

  ReleaseTile(top->top);
  ReleaseTile(top->right);
  ReleaseTile(bottom->bottom);
  ReleaseTile(bottom->left);
  ReleaseTile(top);
  ReleaseTile(bottom);
  ReleaseTile(right);
  ReleaseTile(left);
}


}  // namespace glint
