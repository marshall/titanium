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

#ifndef GLINT_INCLUDE_ROW_H__
#define GLINT_INCLUDE_ROW_H__

#include <string>
#include "glint/include/node.h"

namespace glint {

// A layout container that arranges its children in a horizontal line.
class Row : public Node {
 public:
  Row();

  enum Direction {
    RIGHT_TO_LEFT,
    TOP_TO_BOTTOM,
    LEFT_TO_RIGHT,
    BOTTOM_TO_TOP,
  };

  // Sets a direction in which to 'stack' the children. In other words,
  // a direction in which Row grows.
  Direction direction() { return direction_; }
  void set_direction(Direction direction) {
    if (direction != direction_) {
      direction_ = direction;
      Invalidate();
    }
  }

  // Specifies how to distribute space across children of the Row.
  // takes a string that liiks similar to Frameset spec in HTML.
  // For example, "natural, 153, *, 2*" directs to give "natural size" to
  // the first child, 153 pixels to the second, and remaining space is
  // split in 3 equal parts, 3rd child takes one and 4th child takes two.
  //
  // The priority of space distribution is:
  //  - first the space is given to pixel-specified children
  //  - second the space is given to 'natural' children (they are
  //    simply measured and given the size they require)
  //  - third, the remaining space is distributed across '*' children
  //    proportionally to the weight coefficient used in front of the '*'.
  //
  // If after distributing the space to children that were pixel- and
  // natural-sized it was used up completely, the remaining
  // children are sized to 0.
  //
  // If the Row itself is "sized to content", as if it is in another Row
  // with 'natural' distribution specified, the '*' distributions become
  // equivalent to 'natural'.
  // A number in front of 'natural' gives multiple to the measured required
  // size of a child.
  // If there are more children than values in distribution string, the last
  // value is repeated for remaining children. If the distribution is empty
  // string, the default value of "natural" is used.
  std::string distribution() { return distribution_; }
  // This takes time and can fail, so it is a full-blown method rather then
  // a simple accessor.
  bool ReplaceDistribution(std::string distribution);

#ifdef GLINT_ENABLE_XML
  static BaseObject* CreateInstance() {
    return new Row();
  }

  static SetPropertyResult SetDirection(BaseObject* node,
                                        const std::string& value);

  static SetPropertyResult SetDistribution(BaseObject* node,
                                           const std::string& value);
#endif  // GLINT_ENABLE_XML

 protected:
  virtual Size OnComputeRequiredSize(Size constraint);
  virtual Size OnSetLayoutBounds(Size reserved);
  virtual bool LayoutChildren() { return false; }

 private:
  enum DistributionUnit {
    NATURAL,
    PIXELS,
    WEIGHT,
  };

  struct DistributionValue {
    real32 value;
    DistributionUnit unit;
  };

  bool ParseDistribution(const std::string& spec_string,
                         Array<DistributionValue>* parsed_values);
  DistributionValue* ComputeDistribution(int child_index);

  // switching of coordinates between xy <-> uv. Depends on direction.
  Size GetXYSize(int u, int v);
  int GetU(Size xy);
  int GetV(Size xy);

  Direction direction_;
  std::string distribution_;
  Array<DistributionValue> parsed_distributions_;
  DISALLOW_EVIL_CONSTRUCTORS(Row);
};

}  // namespace glint

#endif  // GLINT_INCLUDE_ROW_H__

