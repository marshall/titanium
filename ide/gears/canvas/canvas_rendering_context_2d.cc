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

#include "gears/canvas/canvas_rendering_context_2d.h"
#include "gears/base/common/dispatcher.h"
#include "gears/base/common/js_runner.h"
#include "gears/base/common/module_wrapper.h"
#include "third_party/skia/include/SkPorterDuff.h"

DECLARE_GEARS_WRAPPER(GearsCanvasRenderingContext2D);
const std::string
    GearsCanvasRenderingContext2D::kModuleName("GearsCanvasRenderingContext2D");

using canvas::skia_config;

GearsCanvasRenderingContext2D::GearsCanvasRenderingContext2D()
    : ModuleImplBaseClass(kModuleName) {
}

void GearsCanvasRenderingContext2D::InitCanvasField(GearsCanvas *new_canvas) {
  // Prevent double initialization.
  assert(canvas_ == NULL);
  assert(new_canvas != NULL);
  canvas_ = new_canvas;
  unload_monitor_.reset(
      new JsEventMonitor(GetJsRunner(), JSEVENT_UNLOAD, this));
}

void GearsCanvasRenderingContext2D::ClearReferenceFromGearsCanvas() {
  // The canvas pointer will be null if this object didn't initalize in the
  // first place (that is, if InitBaseFromSibling failed), or after a page
  // unload event.
  if (canvas_)
    canvas_->ClearRenderingContextReference();
}

void GearsCanvasRenderingContext2D::HandleEvent(JsEventType event_type) {
  assert(event_type == JSEVENT_UNLOAD);
  // On page unload, we must destroy the scoped_refptr to GearsCanvas.
  // But to do that, we must first clear the pointer from GearsCanvas, so that
  // it doesn't become a dangling pointer. If we don't clear it now, we can't
  // clear it in the destructor since at that time we no longer have a pointer
  // to Canvas.
  ClearReferenceFromGearsCanvas();
  canvas_.reset();
}

GearsCanvasRenderingContext2D::~GearsCanvasRenderingContext2D() {
  ClearReferenceFromGearsCanvas();
}

template<>
void Dispatcher<GearsCanvasRenderingContext2D>::Init() {
  RegisterProperty("canvas", &GearsCanvasRenderingContext2D::GetCanvas, NULL);
  RegisterMethod("save", &GearsCanvasRenderingContext2D::Save);
  RegisterMethod("restore", &GearsCanvasRenderingContext2D::Restore);
  RegisterMethod("scale", &GearsCanvasRenderingContext2D::Scale);
  RegisterMethod("rotate", &GearsCanvasRenderingContext2D::Rotate);
  RegisterMethod("translate", &GearsCanvasRenderingContext2D::Translate);
  RegisterMethod("transform", &GearsCanvasRenderingContext2D::Transform);
  RegisterMethod("setTransform", &GearsCanvasRenderingContext2D::SetTransform);
  RegisterProperty("globalAlpha",
      &GearsCanvasRenderingContext2D::GetGlobalAlpha,
      &GearsCanvasRenderingContext2D::SetGlobalAlpha);
  RegisterProperty("globalCompositeOperation",
      &GearsCanvasRenderingContext2D::GetGlobalCompositeOperation,
      &GearsCanvasRenderingContext2D::SetGlobalCompositeOperation);
  RegisterProperty("fillStyle",
      &GearsCanvasRenderingContext2D::GetFillStyle,
      &GearsCanvasRenderingContext2D::SetFillStyle);
  RegisterMethod("clearRect", &GearsCanvasRenderingContext2D::ClearRect);
  RegisterMethod("fillRect", &GearsCanvasRenderingContext2D::FillRect);
  RegisterMethod("strokeRect", &GearsCanvasRenderingContext2D::StrokeRect);
  RegisterProperty("font", &GearsCanvasRenderingContext2D::GetFont,
      &GearsCanvasRenderingContext2D::SetFont);
  RegisterProperty("textAlign", &GearsCanvasRenderingContext2D::GetTextAlign,
      &GearsCanvasRenderingContext2D::SetTextAlign);
  RegisterMethod("fillText", &GearsCanvasRenderingContext2D::FillText);
  RegisterMethod("measureText", &GearsCanvasRenderingContext2D::MeasureText);
  RegisterMethod("drawImage", &GearsCanvasRenderingContext2D::DrawImage);
  RegisterMethod("createImageData",
      &GearsCanvasRenderingContext2D::CreateImageData);
  RegisterMethod("getImageData", &GearsCanvasRenderingContext2D::GetImageData);
  RegisterMethod("putImageData", &GearsCanvasRenderingContext2D::PutImageData);
  RegisterMethod("colorTransform",
      &GearsCanvasRenderingContext2D::ColorTransform);
  RegisterMethod("convolutionTransform",
      &GearsCanvasRenderingContext2D::ConvolutionTransform);
  RegisterMethod("medianFilter", &GearsCanvasRenderingContext2D::MedianFilter);
  RegisterMethod("adjustBrightness",
      &GearsCanvasRenderingContext2D::AdjustBrightness);
  RegisterMethod("adjustContrast",
      &GearsCanvasRenderingContext2D::AdjustContrast);
  RegisterMethod("adjustSaturation",
      &GearsCanvasRenderingContext2D::AdjustSaturation);
  RegisterMethod("adjustHue",
      &GearsCanvasRenderingContext2D::AdjustHue);
  RegisterMethod("blur", &GearsCanvasRenderingContext2D::Blur);
  RegisterMethod("sharpen", &GearsCanvasRenderingContext2D::Sharpen);
  RegisterMethod("resetTransform",
      &GearsCanvasRenderingContext2D::ResetTransform);
}

// TODO(nigeltao): Unless otherwise stated, for the 2D context interface, any
// method call with a numeric argument whose value is infinite or
// a NaN value must be ignored.

void GearsCanvasRenderingContext2D::GetCanvas(JsCallContext *context) {
  assert(canvas_ != NULL);
  context->SetReturnValue(JSPARAM_MODULE, canvas_.get());
}

void GearsCanvasRenderingContext2D::Save(JsCallContext *context) {
  context->SetException(STRING16(L"Unimplemented"));
}

void GearsCanvasRenderingContext2D::Restore(JsCallContext *context) {
  context->SetException(STRING16(L"Unimplemented"));
}

void GearsCanvasRenderingContext2D::Scale(JsCallContext *context) {
  double scale_x, scale_y;
  JsArgument args[] = {
    { JSPARAM_REQUIRED, JSPARAM_DOUBLE, &scale_x },
    { JSPARAM_OPTIONAL, JSPARAM_DOUBLE, &scale_y }
  };
  context->GetArguments(ARRAYSIZE(args), args);
  if (context->is_exception_set())
    return;
  if (context->GetArgumentType(1) == JSPARAM_UNDEFINED) {
    // If only one scale argument is supplied, use it for both dimensions.
    // TODO(nigeltao): Test this.
    scale_y = scale_x;
  }
  context->SetException(STRING16(L"Unimplemented"));
}

void GearsCanvasRenderingContext2D::Rotate(JsCallContext *context) {
  // TODO(nigeltao): Remove this after unit testing.
  context->SetException(STRING16(L"Unimplemented"));
  return;
  double angle;
  JsArgument args[] = {
    { JSPARAM_REQUIRED, JSPARAM_DOUBLE, &angle },
  };
  context->GetArguments(ARRAYSIZE(args), args);
  if (context->is_exception_set())
    return;
  // Convert to degrees.
  const double kPi = 3.1415926535;
  const double kDegreesPerRadian = 180.0/kPi;
  angle *= kDegreesPerRadian;

  canvas_->skia_canvas()->rotate(SkDoubleToScalar(angle));
}

void GearsCanvasRenderingContext2D::Translate(JsCallContext *context) {
  int x, y;
  JsArgument args[] = {
    { JSPARAM_REQUIRED, JSPARAM_INT, &x },
    { JSPARAM_REQUIRED, JSPARAM_INT, &y }
  };
  context->GetArguments(ARRAYSIZE(args), args);
  if (context->is_exception_set())
    return;
  context->SetException(STRING16(L"Unimplemented"));
}

void GearsCanvasRenderingContext2D::Transform(JsCallContext *context) {
  double m11, m12, m21, m22, dx, dy;
  JsArgument args[] = {
    { JSPARAM_REQUIRED, JSPARAM_DOUBLE, &m11 },
    { JSPARAM_REQUIRED, JSPARAM_DOUBLE, &m12 },
    { JSPARAM_REQUIRED, JSPARAM_DOUBLE, &m21 },
    { JSPARAM_REQUIRED, JSPARAM_DOUBLE, &m22 },
    { JSPARAM_REQUIRED, JSPARAM_DOUBLE, &dx },
    { JSPARAM_REQUIRED, JSPARAM_DOUBLE, &dy }
  };
  context->GetArguments(ARRAYSIZE(args), args);
  if (context->is_exception_set())
    return;
  context->SetException(STRING16(L"Unimplemented"));
}

void GearsCanvasRenderingContext2D::SetTransform(JsCallContext *context) {
  double m11, m12, m21, m22, dx, dy;
  JsArgument args[] = {
    { JSPARAM_REQUIRED, JSPARAM_DOUBLE, &m11 },
    { JSPARAM_REQUIRED, JSPARAM_DOUBLE, &m12 },
    { JSPARAM_REQUIRED, JSPARAM_DOUBLE, &m21 },
    { JSPARAM_REQUIRED, JSPARAM_DOUBLE, &m22 },
    { JSPARAM_REQUIRED, JSPARAM_DOUBLE, &dx },
    { JSPARAM_REQUIRED, JSPARAM_DOUBLE, &dy }
  };
  context->GetArguments(ARRAYSIZE(args), args);
  if (context->is_exception_set())
    return;
  context->SetException(STRING16(L"Unimplemented"));
}

void GearsCanvasRenderingContext2D::GetGlobalAlpha(
    JsCallContext *context) {
  double alpha = canvas_->alpha();
  context->SetReturnValue(JSPARAM_DOUBLE, &alpha);
}

void GearsCanvasRenderingContext2D::SetGlobalAlpha(
    JsCallContext *context) {
  double new_alpha;
  JsArgument args[] = {
    { JSPARAM_REQUIRED, JSPARAM_DOUBLE, &new_alpha }
  };
  context->GetArguments(ARRAYSIZE(args), args);
  if (context->is_exception_set())
    return;
  canvas_->set_alpha(new_alpha);
}

void GearsCanvasRenderingContext2D::GetGlobalCompositeOperation(
    JsCallContext *context) {
  std::string16 op = canvas_->composite_operation();
  context->SetReturnValue(JSPARAM_STRING16, &op);
}

void GearsCanvasRenderingContext2D::SetGlobalCompositeOperation(
    JsCallContext *context) {
  std::string16 new_composite_op;
  JsArgument args[] = {
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &new_composite_op }
  };
  context->GetArguments(ARRAYSIZE(args), args);
  if (context->is_exception_set())
    return;
  if (!canvas_->set_composite_operation(new_composite_op)) {
    // HTML5 canvas-only composite operation.
    context->SetException(
        STRING16(L"This composite operation is implemented only in HTML5 canvas"
                 L" and not in Gears' canvas."));
    return;
  }
}

void GearsCanvasRenderingContext2D::GetFillStyle(
    JsCallContext *context) {
  std::string16 fill_style = canvas_->fill_style();
  context->SetReturnValue(JSPARAM_STRING16, &fill_style);
}

// TODO(nigeltao): Generate a better error if given a CanvasGradient or
// CanvasPattern object (from the browser's canvas implementation).
void GearsCanvasRenderingContext2D::SetFillStyle(
    JsCallContext *context) {
  std::string16 new_fill_style;
  JsArgument args[] = {
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &new_fill_style }
  };
  context->GetArguments(ARRAYSIZE(args), args);
  // TODO(nigeltao): Do not generate error on type mismatch; fail silently
  // (as per HTML5 canvas spec).
  if (context->is_exception_set())
    return;

  canvas_->set_fill_style(new_fill_style);
}

void GearsCanvasRenderingContext2D::ClearRect(JsCallContext *context) {
  int x, y, width, height;
  JsArgument args[] = {
    { JSPARAM_REQUIRED, JSPARAM_INT, &x },
    { JSPARAM_REQUIRED, JSPARAM_INT, &y },
    { JSPARAM_REQUIRED, JSPARAM_INT, &width },
    { JSPARAM_REQUIRED, JSPARAM_INT, &height }
  };
  context->GetArguments(ARRAYSIZE(args), args);
  if (context->is_exception_set())
    return;
  context->SetException(STRING16(L"Unimplemented"));
}

void GearsCanvasRenderingContext2D::FillRect(JsCallContext *context) {
  int x, y, width, height;
  JsArgument args[] = {
    { JSPARAM_REQUIRED, JSPARAM_INT, &x },
    { JSPARAM_REQUIRED, JSPARAM_INT, &y },
    { JSPARAM_REQUIRED, JSPARAM_INT, &width },
    { JSPARAM_REQUIRED, JSPARAM_INT, &height }
  };
  context->GetArguments(ARRAYSIZE(args), args);
  if (context->is_exception_set())
    return;
  context->SetException(STRING16(L"Unimplemented"));
}

void GearsCanvasRenderingContext2D::StrokeRect(JsCallContext *context) {
  int x, y, width, height;
  JsArgument args[] = {
    { JSPARAM_REQUIRED, JSPARAM_INT, &x },
    { JSPARAM_REQUIRED, JSPARAM_INT, &y },
    { JSPARAM_REQUIRED, JSPARAM_INT, &width },
    { JSPARAM_REQUIRED, JSPARAM_INT, &height }
  };
  context->GetArguments(ARRAYSIZE(args), args);
  if (context->is_exception_set())
    return;
  context->SetException(STRING16(L"Unimplemented"));
}

void GearsCanvasRenderingContext2D::GetFont(JsCallContext *context) {
  std::string16 font = canvas_->font();
  context->SetReturnValue(JSPARAM_STRING16, &font);
}

void GearsCanvasRenderingContext2D::SetFont(JsCallContext *context) {
  std::string16 new_font;
  JsArgument args[] = {
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &new_font }
  };
  context->GetArguments(ARRAYSIZE(args), args);
  if (context->is_exception_set())
    return;
  canvas_->set_font(new_font);
  context->SetException(STRING16(L"Unimplemented"));
}

void GearsCanvasRenderingContext2D::GetTextAlign(JsCallContext *context) {
  std::string16 text_align = canvas_->text_align();
  context->SetReturnValue(JSPARAM_STRING16, &text_align);
}

void GearsCanvasRenderingContext2D::SetTextAlign(JsCallContext *context) {
  std::string16 new_align;
  JsArgument args[] = {
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &new_align }
  };
  context->GetArguments(ARRAYSIZE(args), args);
  if (context->is_exception_set())
    return;
  canvas_->set_text_align(new_align);
  context->SetException(STRING16(L"Unimplemented"));
}

void GearsCanvasRenderingContext2D::FillText(JsCallContext *context) {
  std::string16 text;
  JsArgument args[] = {
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &text }
  };
  context->GetArguments(ARRAYSIZE(args), args);
  if (context->is_exception_set())
    return;
  context->SetException(STRING16(L"Unimplemented"));
}

void GearsCanvasRenderingContext2D::MeasureText(JsCallContext *context) {
  std::string16 text;
  JsArgument args[] = {
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &text }
  };
  context->GetArguments(ARRAYSIZE(args), args);
  if (context->is_exception_set())
    return;
  context->SetException(STRING16(L"Unimplemented"));
}

void GearsCanvasRenderingContext2D::DrawImage(JsCallContext *context) {
  // TODO(nigeltao): This function has a bug that doesn't make it work after
  // calling resize() on the target canvas. That is, if you resize() a canvas
  // and then draw another canvas on it, the drawImage() is a noop.
  // Also this function doesn't properly respect globalAlpha and
  // globalCompositeOperation.
  // Return an error code to prevent clients from getting bitten by these bugs.
  context->SetException(STRING16(L"Unimplemented"));
  // Some of the functionality of this function can be achieved using
  // crop(), resize() and clone().

  // TODO(nigeltao): Generate a better error message if given a HTMLImageElement
  // or a HTMLCanvasElement.
  
  /*
  ModuleImplBaseClass *other_module;
  int source_x, source_y, source_width, source_height;
  int dest_x, dest_y, dest_width, dest_height;
  JsArgument args[] = {
    { JSPARAM_REQUIRED, JSPARAM_MODULE, &other_module },
    { JSPARAM_OPTIONAL, JSPARAM_INT, &source_x },
    { JSPARAM_OPTIONAL, JSPARAM_INT, &source_y },
    { JSPARAM_OPTIONAL, JSPARAM_INT, &source_width },
    { JSPARAM_OPTIONAL, JSPARAM_INT, &source_height },
    { JSPARAM_OPTIONAL, JSPARAM_INT, &dest_x },
    { JSPARAM_OPTIONAL, JSPARAM_INT, &dest_y },
    { JSPARAM_OPTIONAL, JSPARAM_INT, &dest_width },
    { JSPARAM_OPTIONAL, JSPARAM_INT, &dest_height }
  };
  int num_arguments = context->GetArguments(ARRAYSIZE(args), args);
  if (context->is_exception_set())
    return;
  assert(other_module);
  if (GearsCanvas::kModuleName != other_module->get_module_name()) {
    context->SetException(STRING16(L"Argument must be a Canvas."));
    return;
  }
  scoped_refptr<GearsCanvas> src = static_cast<GearsCanvas*>(other_module);
  
  if (num_arguments != 9) {
    // Handle missing arguments.
    if (num_arguments == 5) {
      dest_width = source_width;
      dest_height = source_height;
    } else if (num_arguments == 3) {
      dest_width = src->width();
      dest_height = src->height();
    } else {
      context->SetException(STRING16(L"Unsupported number of arguments."));
      return;
    }
    source_width = src->width();
    source_height = src->height();
    dest_x = source_x;
    dest_y = source_y;
    source_x = 0;
    source_y = 0;
  }
  
  SkIRect src_irect = {
      source_x, source_y, source_x + source_width, source_y + source_height };
  SkIRect dest_irect = {
      dest_x, dest_y, dest_x + dest_width, dest_y + dest_height };

  // The HTML5 canvas spec says that an invalid src rect must trigger an
  // exception, but it does not say what to do if the dest rect is invalid.
  // So, if both rects are invalid, it's more spec-compliant to raise an error
  // for the src rect.
  if (!src->IsRectValid(src_irect)) {
    context->SetException(STRING16(
        L"INDEX_SIZE_ERR: Source rectangle stretches beyond the bounds"
        L"of its bitmap or has negative dimensions."));
    return;
  }
  if (src_irect.isEmpty()) {
    context->SetException(STRING16(
        L"INDEX_SIZE_ERR: Source rectangle has zero width or height."));
    return;
  }
  if (!canvas_->IsRectValid(dest_irect)) {
    context->SetException(STRING16(
        L"INDEX_SIZE_ERR: Destination rectangle stretches beyond the bounds"
        L"of its bitmap or has negative dimensions."));
    return;
  }
  if (dest_irect.isEmpty()) {
    context->SetException(STRING16(
        L"INDEX_SIZE_ERR: Destination rectangle has zero width or height."));
    return;
  }
  
  // TODO(nigeltao): First apply geometry xform, then alpha, then compositing,
  // as per HTML5 canvas spec.

  // If the globalAlpha is not 1.0, the source image must be multiplied by
  // globalAlpha *before* compositing.
  SkBitmap *modified_src = src->skia_bitmap();
  scoped_ptr<SkBitmap> bitmap_to_delete;

  if (canvas_->alpha() != 1.0) {
    bitmap_to_delete.reset(new SkBitmap);
    modified_src = bitmap_to_delete.get();
    modified_src->setConfig(skia_config, src->width(), src->height());
    modified_src->allocPixels();
    SkCanvas temp_canvas(*modified_src);
    SkPaint temp_paint;
    temp_paint.setAlpha(static_cast<U8CPU>(canvas_->alpha() * 255));
    temp_canvas.drawBitmap(
        *modified_src, SkIntToScalar(0), SkIntToScalar(0), &temp_paint);
  }

  SkPaint paint;
  SkPorterDuff::Mode porter_duff = static_cast<SkPorterDuff::Mode>(
      canvas_->ParseCompositeOperationString(canvas_->composite_operation()));
  paint.setPorterDuffXfermode(porter_duff);


  SkRect dest_rect;
  dest_rect.set(dest_irect);

  // When drawBitmapRect() is called with a source rectangle, it (also) handles
  // the case where source canvas is same as this canvas.
  // TODO(nigeltao): This function silently fails on errors. Find out what can
  // be done about this.
  canvas_->skia_canvas()->drawBitmapRect(
      *modified_src, &src_irect, dest_rect);
  */
  
  // TODO(nigeltao): Are the colors premultiplied? If so, unpremultiply them.
}

void GearsCanvasRenderingContext2D::CreateImageData(JsCallContext *context) {
  int width, height;
  JsArgument args[] = {
    { JSPARAM_REQUIRED, JSPARAM_INT, &width },
    { JSPARAM_REQUIRED, JSPARAM_INT, &height }
  };
  context->GetArguments(ARRAYSIZE(args), args);
  if (context->is_exception_set())
    return;
  context->SetException(STRING16(L"Unimplemented"));
}

void GearsCanvasRenderingContext2D::GetImageData(JsCallContext *context) {
  int sx, sy, sw, sh;
  JsArgument args[] = {
    { JSPARAM_REQUIRED, JSPARAM_INT, &sx },
    { JSPARAM_REQUIRED, JSPARAM_INT, &sy },
    { JSPARAM_REQUIRED, JSPARAM_INT, &sw },
    { JSPARAM_REQUIRED, JSPARAM_INT, &sh }
  };
  context->GetArguments(ARRAYSIZE(args), args);
  if (context->is_exception_set())
    return;
  context->SetException(STRING16(L"Unimplemented"));
}

void GearsCanvasRenderingContext2D::PutImageData(JsCallContext *context) {
  JsObject image_data;
  int dx, dy, dirty_x, dirty_y, dirty_width, dirty_height;
  JsArgument args[] = {
    { JSPARAM_REQUIRED, JSPARAM_OBJECT, &image_data },
    { JSPARAM_REQUIRED, JSPARAM_INT, &dx },
    { JSPARAM_REQUIRED, JSPARAM_INT, &dy },
    { JSPARAM_OPTIONAL, JSPARAM_INT, &dirty_x },
    { JSPARAM_OPTIONAL, JSPARAM_INT, &dirty_y },
    { JSPARAM_OPTIONAL, JSPARAM_INT, &dirty_width },
    { JSPARAM_OPTIONAL, JSPARAM_INT, &dirty_height }
  };
  context->GetArguments(ARRAYSIZE(args), args);
  if (context->is_exception_set())
    return;
  context->SetException(STRING16(L"Unimplemented"));
}

void GearsCanvasRenderingContext2D::ColorTransform(JsCallContext *context) {
  JsObject matrix;
  JsArgument args[] = {
    { JSPARAM_REQUIRED, JSPARAM_OBJECT, &matrix}
  };
  context->GetArguments(ARRAYSIZE(args), args);
  if (context->is_exception_set())
    return;
  context->SetException(STRING16(L"Unimplemented"));
}

void GearsCanvasRenderingContext2D::ConvolutionTransform(
    JsCallContext *context) {
  JsObject matrix;
  JsArgument args[] = {
    { JSPARAM_REQUIRED, JSPARAM_OBJECT, &matrix }
  };
  context->GetArguments(ARRAYSIZE(args), args);
  if (context->is_exception_set())
    return;
  context->SetException(STRING16(L"Unimplemented"));
}

void GearsCanvasRenderingContext2D::MedianFilter(JsCallContext *context) {
  double radius;
  JsArgument args[] = {
    { JSPARAM_REQUIRED, JSPARAM_DOUBLE, &radius }
  };
  context->GetArguments(ARRAYSIZE(args), args);
  if (context->is_exception_set())
    return;
  context->SetException(STRING16(L"Unimplemented"));
}

void GearsCanvasRenderingContext2D::AdjustBrightness(JsCallContext *context) {
  double delta;
  JsArgument args[] = {
    { JSPARAM_REQUIRED, JSPARAM_DOUBLE, &delta }
  };
  context->GetArguments(ARRAYSIZE(args), args);
  if (context->is_exception_set())
    return;
  context->SetException(STRING16(L"Unimplemented"));
}

void GearsCanvasRenderingContext2D::AdjustContrast(JsCallContext *context) {
  double amount;
  JsArgument args[] = {
    { JSPARAM_REQUIRED, JSPARAM_DOUBLE, &amount }
  };
  context->GetArguments(ARRAYSIZE(args), args);
  if (context->is_exception_set())
    return;
  context->SetException(STRING16(L"Unimplemented"));
}

void GearsCanvasRenderingContext2D::AdjustSaturation(JsCallContext *context) {
  double amount;
  JsArgument args[] = {
    { JSPARAM_REQUIRED, JSPARAM_DOUBLE, &amount }
  };
  context->GetArguments(ARRAYSIZE(args), args);
  if (context->is_exception_set())
    return;
  context->SetException(STRING16(L"Unimplemented"));
}

void GearsCanvasRenderingContext2D::AdjustHue(JsCallContext *context) {
  double angle;
  JsArgument args[] = {
    { JSPARAM_REQUIRED, JSPARAM_DOUBLE, &angle }
  };
  context->GetArguments(ARRAYSIZE(args), args);
  if (context->is_exception_set())
    return;
  context->SetException(STRING16(L"Unimplemented"));
}

void GearsCanvasRenderingContext2D::Blur(JsCallContext *context) {
  double factor;
  int radius;
  JsArgument args[] = {
    { JSPARAM_REQUIRED, JSPARAM_DOUBLE, &factor },
    { JSPARAM_REQUIRED, JSPARAM_INT, &radius }
  };
  context->GetArguments(ARRAYSIZE(args), args);
  if (context->is_exception_set())
    return;
  context->SetException(STRING16(L"Unimplemented"));
}

void GearsCanvasRenderingContext2D::Sharpen(JsCallContext *context) {
  double factor;
  int radius;
  JsArgument args[] = {
    { JSPARAM_REQUIRED, JSPARAM_DOUBLE, &factor },
    { JSPARAM_REQUIRED, JSPARAM_INT, &radius }
  };
  context->GetArguments(ARRAYSIZE(args), args);
  if (context->is_exception_set())
    return;
  context->SetException(STRING16(L"Unimplemented"));
}

void GearsCanvasRenderingContext2D::ResetTransform(JsCallContext *context) {
  context->SetException(STRING16(L"Unimplemented"));
}
