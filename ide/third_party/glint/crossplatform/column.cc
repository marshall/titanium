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

#include "glint/include/column.h"
#include "glint/crossplatform/core_util.h"

namespace glint {

#ifdef DEBUG
static const char* kColumnNodeName = "Column";
#endif

Column::Column() {
#ifdef DEBUG
  type_name_ = kColumnNodeName;
#endif
}

// In a column, constrain children horizontally and let them compute their
// height, unconstrained. Then, combine their sizes as following: max the
// widths and sum the heights of all children.
Size Column::OnComputeRequiredSize(Size constraint) {
  Size child_constraint = Size::HugeSize();
  child_constraint.width = constraint.width;
  Size accumulated;

  for (Node *child = first_child(); child; child = child->next_sibling()) {
    Size child_size = child->ComputeRequiredSize(child_constraint);
    accumulated.width = max<int>(accumulated.width, child_size.width);
    accumulated.height += child_size.height;
  }

  return accumulated;
}

// The perent container reserved a space for us - at least the amount returned
// from OnComputeRequiredSize. Now we should iterate the children and give
// them consequent vertical locations.
Size Column::OnSetLayoutBounds(Size reserved) {
  // We should be getting at least the amount we requested in
  // OnComputeRequiredSize - so repeat the same algorithm, giving each child
  // it's required height;
#ifdef DEBUG
  const Rectangle& margin = this->margin();
  ASSERT(reserved.width >=
         required_size().width - margin.left() - margin.right());
  ASSERT(reserved.height >=
         required_size().height - margin.top() - margin.bottom());
#endif

  int current_y = 0;

  for (Node *child = first_child(); child; child = child->next_sibling()) {
    // Compute child's bounds inside the Row.
    int bottom_side = current_y + child->required_size().height;
    Rectangle child_location(0, current_y, reserved.width, bottom_side);
    // Position the child.
    child->SetLayoutBounds(child_location);
    current_y = bottom_side;
  }

  return reserved;
}

}  // namespace glint
