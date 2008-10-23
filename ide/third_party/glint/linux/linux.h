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

// This file contains Linux-specific declarations shared by Linux Glint code.

#ifndef GLINT_LINUX_LINUX_H__
#define GLINT_LINUX_LINUX_H__

#include <gtk/gtk.h>
#include "third_party/scoped_ptr/scoped_ptr.h"

namespace glint {
class Bitmap;
class Color;
}

namespace glint_linux {

// Cleanup/free functions for various types of objects
class ScopedGObjectFree {
 public:
  void operator()(void* x) const;
};

class ScopedPangoAttrListFree {
 public:
  void operator()(void* x) const;
};

class ScopedPangoLayoutIterFree {
 public:
  void operator()(void* x) const;
};

/* Various types of smart pointers for various types of objects with different
   cleanup requirements.
*/
typedef scoped_ptr_malloc<GdkGC, ScopedGObjectFree> ScopedGC;
typedef scoped_ptr_malloc<GdkPixbuf, ScopedGObjectFree> ScopedPixbuf;
typedef scoped_ptr_malloc<GdkPixmap, ScopedGObjectFree> ScopedPixmap;
typedef scoped_ptr_malloc<PangoContext, ScopedGObjectFree> ScopedContext;
typedef scoped_ptr_malloc<PangoLayout, ScopedGObjectFree> ScopedLayout;
typedef scoped_ptr_malloc<PangoAttrList, ScopedPangoAttrListFree>
    ScopedAttrList;
typedef scoped_ptr_malloc<PangoLayoutIter, ScopedPangoLayoutIterFree>
    ScopedLayoutIter;

// Fills a specified rectangular region with a specified RGBA color
void cairo_fill(GdkDrawable* drawable,
                double r,
                double g,
                double b,
                double a,
                int x,
                int y,
                int width,
                int height);

/* Draws a text layout to a new bitmap of the specified dimensions, using
   transparent background and the specified foreground color, replacing
   target with the new bitmap.
*/
bool draw_text_layout(PangoLayout* pango_layout,
                      glint::Color foreground,
                      int width,
                      int height,
                      glint::Bitmap* target);
}

#endif  // GLINT_LINUX_LINUX_H__
