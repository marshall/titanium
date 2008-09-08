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

#ifndef GEARS_CANVAS_CANVAS_H__
#define GEARS_CANVAS_CANVAS_H__

#if !defined(OFFICIAL_BUILD) && (defined(WIN32) || defined(OS_MACOSX))

#include "gears/base/common/common.h"
#include "gears/base/common/scoped_refptr.h"
#include "third_party/scoped_ptr/scoped_ptr.h"
// Can't include any Skia header here, or factory.cc will fail to compile due
// to unused inline functions defined in the Skia header.
class SkBitmap;
class SkCanvas;
struct SkIRect;

class GearsCanvasRenderingContext2D;

// Extension of a subset of HTML5 canvas for photo manipulation.
// See http://code.google.com/p/google-gears/wiki/CanvasAPI for more detailed
// documentation.
class GearsCanvas : public ModuleImplBaseClass {
 public:
  static const std::string kModuleName;

  GearsCanvas();
  virtual ~GearsCanvas();
  
  // Clears the plain pointer to GearsCanvasRenderingContext2D, to prevent a
  // dangling pointer. The rendering context will be recreated when needed.
  void ClearRenderingContextReference();

  // Loads an image from a supplied blob.
  // IN: Blob blob.
  // OUT: -
  void Load(JsCallContext *context);

  // Exports the contents of this canvas into a blob. This is a one-time
  // operation; updates to the canvas don't reflect in the blob.
  // IN: optional String mimeType, optional Object attributes
  // OUT: Blob
  void ToBlob(JsCallContext *context);

  // Returns a new Canvas object with the same state (width, height, pixels).
  // IN: -
  // OUT: Canvas
  void Clone(JsCallContext *context);

  // Crops the canvas to the specified rectangle, in-place.
  // IN: int x, int y, int width, int height
  // OUT: -
  void Crop(JsCallContext *context);

  // Resizes the canvas to the supplied dimensions, in-place.
  // IN: int width, int height
  // OUT: -
  void Resize(JsCallContext *context);

  // Accessors for the state of the canvas. Setting any of these causes the
  // canvas to be reset to transparent black pixels and its context to be
  // reset (to alpha=1, geometry transformation matrix=identity matrix, etc).
  void GetWidth(JsCallContext *context);
  void GetHeight(JsCallContext *context);
  void SetWidth(JsCallContext *context);
  void SetHeight(JsCallContext *context);

  // Returns a context object to draw onto the canvas.
  // IN: String contextId
  // OUT: CanvasRenderingContext2D
  void GetContext(JsCallContext *context);


  // The following are not exported to Javascript.

  // Returns true if the rectangle is contained completely within the bounds
  // of this bitmap, and has non-negative width and height.
  bool IsRectValid(const SkIRect &rect);
  
  // Parses a composite operation string (as per HTML5 canvas) and returns
  // one of the two following values or a Skia PorterDuff enum
  // (represented as an int since we can't include any Skia headers here due to
  // compilation issues; see comment at top of file).
  static const int COMPOSITE_MODE_HTML5_CANVAS_ONLY;
  static const int COMPOSITE_MODE_UNKNOWN;
  static int ParseCompositeOperationString(std::string16 mode);

  // Accessor functions.
  // If given an argument that HTML5 canvas doesn't support, the setters below
  // fail silently without returning an error, as per the HTML5 canvas spec.
  // But if given an argument that HTML5 canvas supports but we don't,
  // the setters (set_composite_operation, to be precise) returns false.
  // Returns the SkBitmap backing store for this canvas.
  SkBitmap *skia_bitmap() const;
  // Returns the SkCanvas object used to draw onto this canvas.
  SkCanvas *skia_canvas() const;
  int width() const;
  int height() const;
  double alpha() const;
  void set_alpha(double new_alpha);
  std::string16 composite_operation() const;
  // Returns false if given a mode HTML5 canvas supports but we don't.
  bool set_composite_operation(std::string16 new_composite_op);
  std::string16 fill_style() const;
  void set_fill_style(std::string16 new_fill_style);
  std::string16 font() const;
  void set_font(std::string16 new_font);
  std::string16 text_align() const;
  void set_text_align(std::string16 new_text_align);

 private:
  // Resets the Canvas to the specified dimensions and fills it with transparent
  // black pixels. All Context state is also destroyed. The HTML5 canvas spec
  // requires this when the user sets the canvas's width or height.
  void ResetCanvas(int width, int height);

  // Can't use a scoped_refptr since that will create a reference cycle.
  // Instead, use a plain pointer and clear it when the target is destroyed.
  // Recreate this pointer when accessed again. For this to work, we make
  // the Context stateless.
  GearsCanvasRenderingContext2D *rendering_context_;
  // Cannot embed objects directly due to compilation issues; see comment
  // at top of file.
  scoped_ptr<SkBitmap> skia_bitmap_;
  scoped_ptr<SkCanvas> skia_canvas_;

  // Context state:
  // TODO(nigeltao): Move this state into SkCanvas?
  double alpha_;
  std::string16 composite_operation_;
  std::string16 fill_style_;
  std::string16 font_;
  std::string16 text_align_;

  DISALLOW_EVIL_CONSTRUCTORS(GearsCanvas);
};

#endif  // !defined(OFFICIAL_BUILD) && ...
#endif  // GEARS_CANVAS_CANVAS_H__
