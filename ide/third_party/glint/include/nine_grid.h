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

#ifndef GLINT_INCLUDE_NINE_GRID_H__
#define GLINT_INCLUDE_NINE_GRID_H__

#include "glint/include/node.h"

namespace glint {

class Bitmap;
struct GridStops;

class NineGrid : public Node {
 public:
  NineGrid();
  ~NineGrid();

  bool shadow() {
    return shadow_;
  }
  void set_shadow(bool shadow) {
    if (shadow_ != shadow) {
      shadow_ = shadow;
      Invalidate();
    }
  }

  // Customization, normally used in derived classes.
  // Derived class can slice and dice the nine_grid_ into portions (one
  // scenario is to work off an "image strip") and also replace image.
  // Useful when NineGrid changes appearance (ie a button which may be pressed).
  virtual bool GetGrid(GridStops* grid);
  virtual Bitmap* GetGridImage();

  Bitmap* nine_grid() { return nine_grid_; }
  // Takes ownership of passed Bitmap. Caller should not delete it.
  void set_nine_grid(Bitmap* nine_grid) {
    ReleaseBitmap();
    nine_grid_ = nine_grid;
    Invalidate();
  }
  bool ReplaceImage(const std::string& file_name);

  //  Width of a center cutout of a 9-grid. By default, it is '1'.
  int center_width() { return center_width_; }
  void set_center_width(int center_width) {
    if (center_width_ != center_width) {
      center_width_ = center_width;
      Invalidate();
    }
  }

  //  Height of a center cutout of a 9-grid. By default, it is '1'.
  int center_height() { return center_height_; }
  void set_center_height(int center_height) {
    if (center_height_ != center_height) {
      center_height_ = center_height;
      Invalidate();
    }
  }

#ifdef GLINT_ENABLE_XML
  static BaseObject* CreateInstance() {
    return new NineGrid();
  }
  static SetPropertyResult SetSource(BaseObject* node,
                                     const std::string& value);
  static SetPropertyResult SetShadow(BaseObject* node,
                                     const std::string& value);

  static SetPropertyResult SetCenterWidth(BaseObject* node,
                                          const std::string& value);

  static SetPropertyResult SetCenterHeight(BaseObject* node,
                                           const std::string& value);
#endif  // GLINT_ENABLE_XML

 protected:
  virtual Size OnSetLayoutBounds(Size reserved);
  virtual bool OnDraw(DrawStack* stack);
  virtual void OnComputeDrawingBounds(Rectangle* bounds);
  virtual bool OnHitTestContent(Point local_point);

 private:
  void GetShadowRect(const Rectangle& draw_area,
                     Rectangle* shadow_rect) const;
  void ReleaseBitmap();

  Bitmap* nine_grid_;
  int center_width_;
  int center_height_;
  bool shadow_;
  DISALLOW_EVIL_CONSTRUCTORS(NineGrid);
};

}  // namespace glint

#endif  // GLINT_INCLUDE_NINE_GRID_H__


