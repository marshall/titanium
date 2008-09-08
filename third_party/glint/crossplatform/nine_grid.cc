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

#include "glint/include/nine_grid.h"
#include "glint/crossplatform/core_util.h"
#include "glint/include/bitmap.h"
#include "glint/include/draw_stack.h"
#include "glint/include/xml_parser.h"

namespace glint {

#ifdef DEBUG
static const char* kNineGridNodeName = "NineGrid";
#endif

// the offset (to right-bottom) of the generated shadow
static const Point shadow_offset = Point(1, 1);
// Shadow blackness, 0-100
static const int shadow_percent = 100;
// A margin which inflates bitmap to fit the blurred shadow
static const int shadow_margin = 8;

NineGrid::NineGrid()
  : nine_grid_(NULL),
    center_width_(1),
    center_height_(1),
    shadow_(false) {
#ifdef DEBUG
  type_name_ = kNineGridNodeName;
#endif
}

// Take all given space
Size NineGrid::OnSetLayoutBounds(Size reserved) {
  return reserved;
}

// Need to account for the drop shadow
void NineGrid::OnComputeDrawingBounds(Rectangle* bounds) {
  ASSERT(bounds);
  // Take the image itself
  bounds->Set(Point(), final_size());
  // Union with shadow area
  Rectangle shadow_rect;
  GetShadowRect(*bounds, &shadow_rect);
  bounds->Union(shadow_rect);
}

void NineGrid::GetShadowRect(const Rectangle& draw_area,
                             Rectangle* shadow_rect) const {
  ASSERT(shadow_rect);
  if (shadow_) {
    // Expand and offset the area.
    int expand = shadow_margin * 2;
    shadow_rect->set_left(draw_area.left() - expand + shadow_offset.x);
    shadow_rect->set_top(draw_area.top() - expand + shadow_offset.y);
    shadow_rect->set_right(draw_area.right() + expand + shadow_offset.x);
    shadow_rect->set_bottom(draw_area.bottom() + expand + shadow_offset.y);
  } else {
    shadow_rect->Set(0, 0, 0, 0);
  }
}

bool NineGrid::OnDraw(DrawStack* stack) {
  // Complain in debug, quietly exit in retail.
  ASSERT(stack && stack->target() && stack->Top());
  if (!stack || !stack->target() || !stack->Top())
    return false;

  // No bitmap specified - nothing to render.
  if (!nine_grid_) {
    return true;
  }

  Rectangle local_draw_area(Point(), final_size());

  Bitmap* target = stack->target();
  DrawContext* draw_context = stack->Top();

  Rectangle local_clip;
  draw_context->transform_to_local.TransformRectangle(draw_context->clip,
                                                      &local_clip);

  local_draw_area.Intersect(local_clip);

  // Nothing to draw
  if (local_draw_area.IsEmpty())
    return true;

  Bitmap temp(local_draw_area.origin(), local_draw_area.size());
  temp.Fill(colors::kTransparent);

  GridStops grid;
  if (!GetGrid(&grid))
    return false;

  Bitmap *nine_grid = GetGridImage();
  if (!nine_grid)
    return false;

  Rectangle destination_rect(Point(), final_size());

  temp.DrawNineGrid(*nine_grid,
                    grid,
                    destination_rect,
                    local_draw_area,
                    255);


  // TODO(dimich): consider optimization that would avoid generating/blurring
  // shadows under stretches of opaque colors. Why render under opaque pixels?
  if (shadow_) {
    Rectangle shadow_rect;
    GetShadowRect(local_draw_area, &shadow_rect);
    Bitmap shadow(shadow_rect.origin(), shadow_rect.size());
    shadow.Fill(colors::kTransparent);

    // Set shadow pixels to be a BW copy of the source image, shifted.
    Color dark(0xFF000000);
    for (int y = local_draw_area.top(); y < local_draw_area.bottom(); ++y) {
      Color* pixel = temp.GetPixelAt(local_draw_area.left(), y);
      Color* shadow_pixel =
          shadow.GetPixelAt(shadow_offset.x + local_draw_area.left(),
                            shadow_offset.y + y);
      for (int x = local_draw_area.left(); x < local_draw_area.right(); ++x) {
        dark.set_alpha(pixel->alpha() * shadow_percent / 100);
        ++pixel;
        *shadow_pixel = dark;
        ++shadow_pixel;
      }
    }

    // Blur the shadow. Use 5 passes to get the stronger blur.
    shadow.GaussianBlur(5);
    // Overlap the original image on top of the shadow
    Transform identity;
    shadow.Compose(temp,
                   local_draw_area,
                   identity,
                   shadow_rect,
                   255);

    target->Compose(shadow,
                    shadow_rect,
                    draw_context->transform_to_global,
                    draw_context->clip,
                    draw_context->alpha);
  } else {
     target->Compose(temp,
                     local_draw_area,
                     draw_context->transform_to_global,
                     draw_context->clip,
                     draw_context->alpha);
  }
  return true;
}

NineGrid::~NineGrid() {
  ReleaseBitmap();
}

void NineGrid::ReleaseBitmap() {
  delete nine_grid_;
  nine_grid_ = NULL;
}

bool NineGrid::ReplaceImage(const std::string& file_name) {
  ReleaseBitmap();
  if (!platform()->LoadBitmapFromFile(file_name, &nine_grid_))
    return false;

  Invalidate();
  return true;
}

bool NineGrid::GetGrid(GridStops* grid) {
  if (!grid || !nine_grid_)
    return false;

  int width = nine_grid_->size().width;
  int height = nine_grid_->size().height;

  grid->x[0] = 0;
  grid->x[1] = max<int>(0, (width - center_width_) / 2);
  grid->x[2] = min<int>(width, grid->x[1] + center_width_);
  grid->x[3] = width;

  grid->y[0] = 0;
  grid->y[1] = max<int>(0, (height - center_height_) / 2);
  grid->y[2] = min<int>(height, grid->y[1] + center_height_);
  grid->y[3] = height;
  return true;
}

Bitmap* NineGrid::GetGridImage() {
  // Default implementation. Derived classes may use this to return altered
  // 9-grid (for example, rotated etc)
  return nine_grid_;
}

#ifdef GLINT_ENABLE_XML
SetPropertyResult NineGrid::SetSource(BaseObject* node,
                                      const std::string& value) {
  ASSERT(node);
  static_cast<NineGrid*>(node)->ReplaceImage(value);
  return PROPERTY_OK;
}

SetPropertyResult NineGrid::SetShadow(BaseObject* node,
                                      const std::string& value) {
  ASSERT(node);
  bool shadow;
  if (!XmlParser::StringToBool(value, &shadow))
    return PROPERTY_HAS_INCORRECT_FORMAT;
  static_cast<NineGrid*>(node)->shadow_ = shadow;
  return PROPERTY_OK;
}

SetPropertyResult NineGrid::SetCenterWidth(BaseObject* node,
                                           const std::string& value) {
  return SetSizeFromString(value,
                           &(static_cast<NineGrid*>(node)->center_width_));
}

SetPropertyResult NineGrid::SetCenterHeight(BaseObject* node,
                                            const std::string& value) {
  return SetSizeFromString(value,
                           &(static_cast<NineGrid*>(node)->center_height_));
}
#endif  // GLINT_ENABLE_XML


// Exclude shadow area as not hittestable (otherwise buttons abutt to
// a NineGrid in the shadow area are not clickable ).
bool NineGrid::OnHitTestContent(Point local_point) {
  Rectangle clip;
  GetClipRect(&clip);
  Rectangle hit_area(Point(), final_size());
  hit_area.Intersect(clip);

  if (hit_area.IsEmpty())
    return false;

  return local_point.x >= hit_area.left() &&
         local_point.y >= hit_area.top() &&
         local_point.x <  hit_area.right()  &&
         local_point.y <  hit_area.bottom();
}

}  // namespace glint
