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

#include <string>
#include "glint/include/row.h"
#include "glint/crossplatform/core_util.h"
#include "glint/crossplatform/lexer.h"

namespace glint {

#ifdef DEBUG
static const char* kRowNodeName = "Row";
#endif

// TODO(dimich): think about better way to pass "unconstrained". May be -1?
static int kHugeSize = 1000000;

Row::Row() : direction_(LEFT_TO_RIGHT) {
#ifdef DEBUG
  type_name_ = kRowNodeName;
#endif
}

Size Row::GetXYSize(int u, int v) {
  if (direction_ == TOP_TO_BOTTOM || direction_ == BOTTOM_TO_TOP) {
    return Size(u, v);
  } else {
    return Size(v, u);
  }
}

// U and V are coordinates are used as "logical" ones. U is measured
// perpendicular to the direction of the row, and V is in the direction
// of growing. The first child starts in (U, V) = (0, 0), then second child
// is appended in the direction of increasing V. Later, (U,V) coordinates
// are converted into actual (X,Y) depending on direction of the row.
// For left_to_right row the U is Y and V is X, while for
// top_to_bottom row the U is X and V is Y.
int Row::GetU(Size xy) {
  if (direction_ == TOP_TO_BOTTOM || direction_ == BOTTOM_TO_TOP) {
    return xy.width;
  } else {
    return xy.height;
  }
}

int Row::GetV(Size xy) {
  if (direction_ == TOP_TO_BOTTOM || direction_ == BOTTOM_TO_TOP) {
    return xy.height;
  } else {
    return xy.width;
  }
}

Row::DistributionValue* Row::ComputeDistribution(int child_index) {
  DistributionValue* distribution = new DistributionValue();
  if (parsed_distributions_.length() == 0) {
    distribution->value = static_cast<real32>(1);
    distribution->unit = NATURAL;
  } else {
    int index = min<int>(child_index, parsed_distributions_.length() - 1);
    distribution->value = parsed_distributions_[index]->value;
    distribution->unit = parsed_distributions_[index]->unit;
  }

  ASSERT(distribution->value >= 0);
  return distribution;
}

// In a row, constrain children in U direction and let them compute their V,
// unconstrained or constrained depending on distribution specification.
// Then, combine their sizes as following: max the U and sum the V
// of all children.
Size Row::OnComputeRequiredSize(Size constraint) {
  // The Row is always constrained in U direction and grows in V direction.
  // Depending on horizontal/vertical orientation, those coordinates may mean
  // X or Y.
  int constraint_u = GetU(constraint);
  int constraint_v = GetV(constraint);
  bool constrained = (constraint_v < kHugeSize);
  int consumed_u = 0;
  int consumed_v = 0;
  // Filled with a DV for each child.
  Array<DistributionValue> distributions;
  real32 sum_of_weights = 0;
  int child_index = 0;
  for (Node *child = first_child();
       child;
       child = child->next_sibling(), ++child_index) {
    // find out constraint_v from distribution specification
    DistributionValue* distribution =
        ComputeDistribution(child_index);
    distributions.Add(distribution);

    // Accumulate the sum of weights - we will need
    if (distribution->unit == WEIGHT) {
      sum_of_weights += distribution->value;
    }

    // We can already count the space taken by PIXELS children.
    if (distribution->unit != PIXELS)
      continue;

    Size constraint_xy = GetXYSize(constraint_u, Round(distribution->value));
    Size size_xy = child->ComputeRequiredSize(constraint_xy);
    consumed_u = max<int>(consumed_u, GetU(size_xy));
    consumed_v += Round(distribution->value);
  }

  // Now, process all children with "NATURAL" distribution.
  int max_weight_slice = 0;
  child_index = 0;
  for (Node *child = first_child();
       child;
       child = child->next_sibling(), ++child_index) {
    DistributionValue* distribution = distributions[child_index];

    // WEIGHT cells are measured as NATURAL if there is no constraint, ie
    // if we are sized "to the natural size of content". In this case,
    // there is no "given space" to distribute actross cells with
    // '*'-specified size, so the '*' gets treated as 'natural'.
    // For cells that are treated like this, also accumulate
    // "maximum weight slice" which will be used to compute their aggregate
    // contribution to the required size. For example, if there are 2 cells
    // specified like "*,*" and one of them is measuring 10px and another 100px,
    // we report required size back as 200px and then split the reserved space
    // equally so they end up being equal, as specification implies.
    // TODO(dimich): consider changing the algorithm to report
    // actual minimum and then re-distribute extra space in the SetSize phase
    // treating the amounts that cells can accept to bring them up to
    // exact specification as weights.
    bool weight_as_natural = (!constrained && distribution->unit == WEIGHT);

    bool measure_natural_size = distribution->unit == NATURAL ||
                                weight_as_natural;

    if (!measure_natural_size)
      continue;

    // For measuring the natural size, remove constraint on V.
    Size constraint_xy = GetXYSize(constraint_u, kHugeSize);
    Size size_xy = child->ComputeRequiredSize(constraint_xy);
    consumed_u = max<int>(consumed_u, GetU(size_xy));
    if (!weight_as_natural) {
      consumed_v += Round(GetV(size_xy) * distribution->value);
    } else {
      max_weight_slice = max<int>(max_weight_slice,
                                  Round(GetV(size_xy) / distribution->value));
    }
  }

  // Finally, process all children with "WEIGHT" distribution.
  // They use the rest of the remaining constraint.
  // Also, if we are not constrained in v direction, we should have
  // measured all the WEIGHT cells as NATURAL in the previous loop.
  if (constraint_v < kHugeSize) {  // if constrained
    int remaining_v = max<int>(0, constraint_v - consumed_v);
    int weight_slice = Round(remaining_v / sum_of_weights);
    child_index = 0;
    for (Node *child = first_child();
         child;
         child = child->next_sibling(), ++child_index) {
      DistributionValue* distribution = distributions[child_index];
      if (distribution->unit != WEIGHT)
        continue;

      int weighted_v = Round(weight_slice * distribution->value);
      Size constraint_xy = GetXYSize(constraint_u, weighted_v);
      Size size_xy = child->ComputeRequiredSize(constraint_xy);
      consumed_u = max<int>(consumed_u, GetU(size_xy));
      consumed_v += weighted_v;
    }
  } else {
    consumed_v += Round(max_weight_slice * sum_of_weights);
  }

  return GetXYSize(consumed_u, consumed_v);
}

// The perent container reserved a space for us - at least the amount returned
// from OnComputeRequiredSize. Now we should iterate the children and give
// them consequent locations.
Size Row::OnSetLayoutBounds(Size reserved) {
  // We should be getting at least the amount we requested in
  // OnComputeRequiredSize - so repeat the same algorithm, giving each child
  // it's required width;
#ifdef DEBUG
  Rectangle margin;
  margin.Set(this->margin());
  ASSERT(reserved.width >=
         required_size().width - margin.left() - margin.right());
  ASSERT(reserved.height >=
         required_size().height - margin.top() - margin.bottom());
#endif

  int reserved_v = GetV(reserved);

  Array<DistributionValue> distributions;
  real32 sum_of_weights = 0;
  int consumed_v = 0;
  int child_index = 0;

  // First, find out the remaining space after PIXELS and NATURAL children.
  for (Node *child = first_child();
       child;
       child = child->next_sibling(), ++child_index) {
    // find out constraint_v from distribution specification
    DistributionValue* distribution = ComputeDistribution(child_index);
    distributions.Add(distribution);

    // Accumulate the sum of weights - we will need
    if (distribution->unit == WEIGHT) {
      sum_of_weights += distribution->value;
    } else if (distribution->unit == PIXELS) {
      consumed_v += Round(distribution->value);
    } else if (distribution->unit == NATURAL) {
      consumed_v += Round(GetV(child->required_size()) * distribution->value);
    } else {
      ASSERT(false);  // there should not be more values in unit enum.
      break;
    }
  }

  int remaining_v = max<int>(0, reserved_v - consumed_v);
  int weight_slice = Round(remaining_v / sum_of_weights);
  int current_v = 0;
  child_index = 0;
  // Now we know everything to distribute the space.
  for (Node *child = first_child();
       child;
       child = child->next_sibling(), ++child_index) {
    // find out constraint_v from distribution specification
    DistributionValue* distribution = distributions[child_index];

    int child_v = 0;
    switch (distribution->unit) {
      case PIXELS:
        child_v = Round(distribution->value);
        break;
      case NATURAL:
        child_v = Round(GetV(child->required_size()) * distribution->value);
        break;
      case WEIGHT:
        child_v = Round(weight_slice * distribution->value);
        break;
      default:
        ASSERT(false);
        break;
    }

    Rectangle location_xy;
    switch (direction_) {
      case LEFT_TO_RIGHT:
        location_xy.Set(current_v,
                        0,
                        current_v + child_v,
                        reserved.height);
        break;
      case RIGHT_TO_LEFT:
        location_xy.Set(reserved.width - current_v - child_v,
                        0,
                        reserved.width - current_v,
                        reserved.height);
        break;
      case TOP_TO_BOTTOM:
        location_xy.Set(0,
                        current_v,
                        reserved.width,
                        current_v + child_v);
        break;
      case BOTTOM_TO_TOP:
        location_xy.Set(0,
                        reserved.height - current_v - child_v,
                        reserved.width,
                        reserved.height - current_v);
        break;
      default:
        ASSERT(false);
        break;
    }

    child->SetLayoutBounds(location_xy);
    current_v += child_v;
  }
  return reserved;
}


bool Row::ParseDistribution(const std::string& spec_string,
                            Array<DistributionValue>* parsed_values) {
  ASSERT(parsed_values);
  Lexer lex(spec_string);
  bool first_value = true;
  while (!lex.IsEmpty()) {
    if (!first_value && !lex.CheckToken(Lexer::COMMA))
      return false;
    first_value = false;

    real32 value;
    bool has_value = lex.ParseReal32(&value);
    if (!has_value) {
      value = 1;
    }

    DistributionUnit unit;
    std::string unit_string;
    if (lex.CheckToken(Lexer::STAR)) {
      unit = WEIGHT;
    } else if (has_value &&
               (lex.token() == Lexer::COMMA ||
                lex.token() == Lexer::EOL)) {
      unit = PIXELS;
    } else if (lex.ParseIdentifier(&unit_string) &&
               unit_string == "natural") {
      unit = NATURAL;
    } else {
      return false;
    }

    DistributionValue* distribution = new DistributionValue;
    distribution->value = max<real32>(0, value);
    distribution->unit = unit;
    parsed_values->Add(distribution);
  }
  return true;
}

bool Row::ReplaceDistribution(std::string distribution) {
  if (distribution == distribution_)
    return true;

  Array<DistributionValue> parsed_values;
  if (!ParseDistribution(distribution, &parsed_values))
    return false;

  parsed_distributions_.Swap(&parsed_values);
  distribution_ = distribution;
  Invalidate();
  return true;
}

#ifdef GLINT_ENABLE_XML
SetPropertyResult Row::SetDirection(BaseObject* node,
                                    const std::string& value) {
  ASSERT(node);
  Direction direction = LEFT_TO_RIGHT;
  if (value == "left_to_right") {
    direction = LEFT_TO_RIGHT;
  } else if (value == "right_to_left") {
    direction = RIGHT_TO_LEFT;
  } else if (value == "top_to_bottom") {
    direction = TOP_TO_BOTTOM;
  } else if (value == "bottom_to_top") {
    direction = BOTTOM_TO_TOP;
  } else {
    return PROPERTY_HAS_INCORRECT_FORMAT;
  }

  static_cast<Row*>(node)->set_direction(direction);
  return PROPERTY_OK;
}

SetPropertyResult Row::SetDistribution(BaseObject* node,
                                       const std::string& value) {
  ASSERT(node);
  if (!static_cast<Row*>(node)->ReplaceDistribution(value))
    return PROPERTY_HAS_INCORRECT_FORMAT;
  return PROPERTY_OK;
}
#endif  // GLINT_ENABLE_XML


}  // namespace glint
