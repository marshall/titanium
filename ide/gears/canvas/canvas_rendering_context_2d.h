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

#ifndef GEARS_CANVAS_CANVAS_RENDERING_CONTEXT_2D_H__
#define GEARS_CANVAS_CANVAS_RENDERING_CONTEXT_2D_H__

#if !defined(OFFICIAL_BUILD) && (defined(WIN32) || defined(OS_MACOSX))

#include "gears/base/common/base_class.h"
#include "gears/base/common/common.h"
#include "gears/canvas/canvas.h"
#include "third_party/skia/include/SkCanvas.h"

namespace canvas {
extern const SkBitmap::Config skia_config;
}

// 2D Context to manipulate the canvas.
// See http://code.google.com/p/google-gears/wiki/CanvasAPI for more detailed
// documentation.
class GearsCanvasRenderingContext2D
    : public ModuleImplBaseClass,
      public JsEventHandlerInterface {
 public:
  static const std::string kModuleName;

  GearsCanvasRenderingContext2D();
  virtual ~GearsCanvasRenderingContext2D();

  // Initializes the canvas backreference from this object.
  // This function must be called only once.
  // This function is not exposed to Javascript and exists only for internal
  // use.
  void InitCanvasField(GearsCanvas *canvas);

  // Returns a backreference to the canvas this context was created from.
  // IN: -
  // OUT: Canvas
  void GetCanvas(JsCallContext *context);

  // Pushes the current state on the state stack.
  // IN: -
  // OUT: -
  void Save(JsCallContext *context);

  // Restores the current state by popping the state stack.
  // IN: -
  // OUT: -
  void Restore(JsCallContext *context);

  // Sets the geometry transformation matrix to scale future drawing operations.
  // If only one scale argument is supplied, we use it for both dimensions.
  // IN: double x, double y
  // OUT: -
  void Scale(JsCallContext *context);

  // Sets the geometry transformation matrix to rotate future drawing
  // operations by the given angle, in radians.
  // IN: double angle
  // OUT: -
  void Rotate(JsCallContext *context);

  // Sets the geometry transformation matrix to translate future drawing
  // operations by the specified amount.
  // IN: int x, int y
  // OUT: -
  void Translate(JsCallContext *context);

  // Multiplies the current geometry transformation matrix by the given
  // matrix.
  // IN: float m11, float m12, float m21, float m22, float dx, float dy
  // OUT: -
  void Transform(JsCallContext *context);

  // Sets the current geometry transformation matrix to the given matrix.
  // IN: float m11, float m12, float m21, float m22, float dx, float dy
  // OUT: -
  void SetTransform(JsCallContext *context);


  // The following items constitute the state of the context.
  // They affect future operations on the canvas.

  // Returns the current global alpha, which is used for future drawing
  // operations.
  // IN: -
  // OUT: double
  void GetGlobalAlpha(JsCallContext *context);

  // Sets the global alpha, which must be between 0.0 and 1.0.
  // IN: double
  // OUT: -
  void SetGlobalAlpha(JsCallContext *context);

  // Returns the current global composite operation, which defines how future
  // drawing operations are composited onto the canvas.
  // IN: -
  // OUT: int
  void GetGlobalCompositeOperation(JsCallContext *context);

  // Sets the global composite operation, which must be one of 'source-over'
  // and 'copy'.
  // IN: int
  // OUT: -
  void SetGlobalCompositeOperation(JsCallContext *context);

  // Returns the current fill style, which is used for future drawing
  // operations.
  // IN: -
  // OUT: string
  void GetFillStyle(JsCallContext *context);

  // Sets the fill style, which must be a valid CSS3 color specification.
  // IN: string
  // OUT: -
  void SetFillStyle(JsCallContext *context);

  // Rectangle operations.
  // IN: int x, int y, int width, int height
  // OUT: -
  // Clears the specified rectangle (sets it to transparent black).
  void ClearRect(JsCallContext *context);
  // Fills the specified rectangle by the current fill color.
  void FillRect(JsCallContext *context);
  // Strokes the rectangle with a one-pixel think border.
  // TODO(nigeltao): Define stroke color.
  void StrokeRect(JsCallContext *context);

  // These affect future text rendering-related
  // operations (FillText and MeasureText).
  void GetFont(JsCallContext *context);
  void SetFont(JsCallContext *context);
  void GetTextAlign(JsCallContext *context);
  void SetTextAlign(JsCallContext *context);

  // Renders filled text at the specified point (baseline left).
  // If a maxWidth is specified and the text exceeds the width, it is
  // truncated, unless wrap is specified, in which case it's wrapped.
  // The text may exceed the height of the canvas (especially with
  // wrapping) in which case it's truncated vertically.
  // IN: string text, float x, float y, optional float maxWidth
  // OUT: -
  void FillText(JsCallContext *context);

  // Returns a text metrics object with information about the given text.
  // IN: string text
  // OUT: TextMetrics
  void MeasureText(JsCallContext *context);

  // Draws a canvas onto this canvas. sx, sy, sw, and sh identify a rectangular
  // subset of the source canvas, and dx, dy, dw, dh identify the target area
  // in the destination canvas (this canvas) into which pixels from the source
  // rectangle are drawn, with resizing if necessary.
  // IN: Canvas image, int sx, int sy, int sw, int sh, int dx, int dy, int dw,
  //     int dh
  // OUT: -
  void DrawImage(JsCallContext *context);

  // Creates an ImageData object, used for pixel manipulation.
  // IN: int sw, int sh
  // OUT: ImageData
  void CreateImageData(JsCallContext *context);

  // Reads pixels from the specified rectangular portion of the canvas, copies
  // them into a new ImageData object and returns it.
  // IN: int sx, int sy, int sw, int sh
  // OUT: ImageData
  void GetImageData(JsCallContext *context);

  // Copies pixels from the given ImageData object into the canvas at a
  // specified (x, y) offset. If the 'dirty' arguments are supplied, they
  // identify a rectangular subset of the given ImageData; only those pixels
  // are copied into the canvas.
  // Overloaded:
  // IN: ImageData imagedata, int dx, int dy
  // IN: ImageData imagedata, int dx, int dy, int dirtyX, int dirtyY,
  //     int dirtyWidth, int dirtyHeight
  // OUT: -
  void PutImageData(JsCallContext *context);

  // Performs a color transformation on the canvas with the given matrix.
  // IN: double[] colorMatrix
  // OUT: -
  void ColorTransform(JsCallContext *context);

  // Performs a convolution transformation on the canvas with the given matrix.
  // IN: double[] convolutionMatrix, optional float bias,
  //     optional boolean applyToAlpha
  // OUT: -
  void ConvolutionTransform(JsCallContext *context);

  // Performs a median filter with the given radius, which can be fractional.
  // IN: double radius
  // OUT: -
  void MedianFilter(JsCallContext *context);

  // Adjusts brigthness by the given amount, which is between -1.0 and +1.0
  // IN: double delta.
  // OUT: -
  void AdjustBrightness(JsCallContext *context);

  // Adjusts contrast by the given amount.
  // IN: double contrast.
  // OUT: -
  void AdjustContrast(JsCallContext *context);

  // Adjusts saturation by the given amount, while preserving luminance.
  // IN: double saturation
  // OUT: -
  void AdjustSaturation(JsCallContext *context);

  // Rotates hue on the color wheel by the given amount, while preserving
  // luminance.
  // IN: double angle
  // OUT: -
  void AdjustHue(JsCallContext *context);

  // Performs a blur by taking a weighted mean of pixels within the
  // specified radius.
  // IN: double factor, int radius.
  // OUT: -
  void Blur(JsCallContext *context);

  // Inverse of Blur().
  // IN: double factor, int radius.
  // OUT: -
  void Sharpen(JsCallContext *context);

  // Resets the transformation matrix to the identity matrix.
  // IN: -
  // OUT: -
  void ResetTransform(JsCallContext *context);

 private:
  // Calls ClearRenderingContextReference() on canvas_, if canvas_ is not NULL.
  void ClearReferenceFromGearsCanvas();

  // Callback used to handle the 'JSEVENT_UNLOAD' event.
  virtual void HandleEvent(JsEventType event_type);

  scoped_refptr<GearsCanvas> canvas_;

  scoped_ptr<JsEventMonitor> unload_monitor_;

  DISALLOW_EVIL_CONSTRUCTORS(GearsCanvasRenderingContext2D);
};

#endif  // !defined(OFFICIAL_BUILD) && ...
#endif  // GEARS_CANVAS_CANVAS_RENDERING_CONTEXT_2D_H__
