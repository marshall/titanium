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

#include "gears/base/common/dispatcher.h"
#include "gears/base/common/module_wrapper.h"
#include "gears/base/common/string_utils.h"
#include "gears/blob/blob.h"
#include "gears/canvas/blob_backed_skia_input_stream.h"
#include "gears/canvas/blob_backed_skia_output_stream.h"
#include "gears/canvas/canvas_rendering_context_2d.h"
#include "third_party/skia/include/SkCanvas.h"
#include "third_party/skia/include/SkImageDecoder.h"
#include "third_party/skia/include/SkRect.h"
#include "third_party/skia/include/SkStream.h"

namespace canvas {
const SkBitmap::Config skia_config = SkBitmap::kARGB_8888_Config;
}

using canvas::skia_config;

DECLARE_GEARS_WRAPPER(GearsCanvas);
const std::string GearsCanvas::kModuleName("GearsCanvas");

GearsCanvas::GearsCanvas()
    : ModuleImplBaseClass(kModuleName),
    rendering_context_(NULL) {
  // Initial dimensions as per the HTML5 canvas spec.
  ResetCanvas(300, 150);
}

GearsCanvas::~GearsCanvas() {
  // The rendering context is destroyed first, since it has a scoped_refptr to
  // this object. See comment near bottom of canvas.h.
  assert(rendering_context_ == NULL);
}

void GearsCanvas::ClearRenderingContextReference() {
  rendering_context_ = NULL;
}

template<>
void Dispatcher<GearsCanvas>::Init() {
  RegisterMethod("load", &GearsCanvas::Load);
  RegisterMethod("toBlob", &GearsCanvas::ToBlob);
  RegisterMethod("clone", &GearsCanvas::Clone);
  RegisterMethod("crop", &GearsCanvas::Crop);
  RegisterMethod("resize", &GearsCanvas::Resize);
  RegisterProperty("width", &GearsCanvas::GetWidth, &GearsCanvas::SetWidth);
  RegisterProperty("height", &GearsCanvas::GetHeight, &GearsCanvas::SetHeight);
  RegisterMethod("getContext", &GearsCanvas::GetContext);
}

void GearsCanvas::Load(JsCallContext *context) {
  ModuleImplBaseClass *other_module;
  JsArgument args[] = {
    { JSPARAM_REQUIRED, JSPARAM_MODULE, &other_module },
  };
  context->GetArguments(ARRAYSIZE(args), args);
  if (context->is_exception_set())
    return;
  assert(other_module);
  if (GearsBlob::kModuleName != other_module->get_module_name()) {
    context->SetException(STRING16(L"Argument must be a Blob."));
    return;
  }
  scoped_refptr<BlobInterface> blob;
  static_cast<GearsBlob*>(other_module)->GetContents(&blob);
  assert(blob.get());
  
  BlobBackedSkiaInputStream blob_stream(blob.get());
  if (!SkImageDecoder::DecodeStream(&blob_stream,
                                    skia_bitmap(),
                                    skia_config,
                                    SkImageDecoder::kDecodePixels_Mode)) {
    context->SetException(STRING16(L"Couldn't decode blob."));
  }
}

void GearsCanvas::ToBlob(JsCallContext *context) {
  std::string16 mime_type;
  JsObject attributes;
  JsArgument args[] = {
    { JSPARAM_OPTIONAL, JSPARAM_STRING16, &mime_type },
    { JSPARAM_OPTIONAL, JSPARAM_OBJECT, &attributes }
  };
  int num_arguments = context->GetArguments(ARRAYSIZE(args), args);
  if (context->is_exception_set())
    return;
  SkImageEncoder::Type type;
  if (mime_type == STRING16(L"") ||
      StringCompareIgnoreCase(mime_type.c_str(), STRING16(L"image/png")) == 0) {
    type = SkImageEncoder::kPNG_Type;
  } else if (StringCompareIgnoreCase(mime_type.c_str(),
      STRING16(L"image/jpeg")) == 0) {
    type = SkImageEncoder::kJPEG_Type;
  } else {
    // TODO(nigeltao): Support BMP. Create it manually?
    // Skia doesn't support BMP encoding.
    context->SetException(STRING16(L"Unsupported MIME type."));
    return;
  }

  // Pixels should have been allocated, either in the contructor,
  // or in SetWidth() and SetHeight().
  assert(skia_bitmap()->getPixels());

  // SkBitmap's isOpaque flag tells whether the bitmap has any transparent
  // pixels. If this flag is set, the encoder creates a file without alpha
  // channel. We don't want this to happen, for several reasons:
  //   1. Consider:
  //        canvas.load(jpegBlob);
  //        canvas.crop(0, 0, canvas.width, canvas.height);
  //      The original SkBitmap has isOpaque set to true (jpegs have no
  //      transparent pixels), but the new SkBitmap created during crop() has
  //      the bit clear. So the crop will not be a noop.
  //   2. A similar situation occurs with clone()...
  //   3. ... and with drawImage().
  //   4. Consider:
  //        canvas.load(blob);
  //        canvas.width = 100;
  //        do some drawing on the canvas
  //        var outputBlob = canvas.toBlob();
  //
  //      When the image is loaded, the isOpaque flag is updated, and is not
  //      modified again anywhere. As a result, exported blobs will have an
  //      alpha channel or not, depending on whether the image that was loaded
  //      *before* resetting the dimensions has an alpha channel.
  //
  // All the above two cases except (3) can be dealt with by making Crop() and
  // Clone() propagate the flag from the source SkBitmap to the newly created
  // one, and by having SetWidth()/SetHeight create a fresh SkBitmap to prevent
  // picking up the isOpaque flag from the previous state.
  //
  // But for drawImage, if the source canvas is opaque, we can't set the target
  // opaque, since there may be other pixels outside the drawing rectangle that
  // may or may not be transparent. Futher, these other pixels can be cropped
  // away in the future. The only clean way is to set IsOpaque false and not
  // have the encoder strip away the alpha channel while exporting to a blob.
  skia_bitmap()->setIsOpaque(false);

  BlobBackedSkiaOutputStream output_stream;
  scoped_ptr<SkImageEncoder> encoder(SkImageEncoder::Create(type));
  bool encode_succeeded;
  double quality;
  if (num_arguments == 2 &&
      attributes.GetPropertyAsDouble(STRING16(L"quality"), &quality)) {
    if (quality < 0.0 || quality > 1.0) {
      context->SetException(STRING16(L"quality must be between 0.0 and 1.0"));
      return;
    }
    encode_succeeded = encoder->encodeStream(&output_stream, *skia_bitmap(),
        static_cast<int> (quality * 100));
  } else {
    encode_succeeded = encoder->encodeStream(&output_stream, *skia_bitmap());
  }
  if (!encode_succeeded) {
    context->SetException(STRING16(L"Could not encode image."));
    return;
  }

  scoped_refptr<BlobInterface> blob;
  output_stream.CreateBlob(&blob);
  scoped_refptr<GearsBlob> gears_blob;
  if (!CreateModule<GearsBlob>(module_environment_.get(),
                               context, &gears_blob)) {
    return;
  }
  gears_blob->Reset(blob.get());
  context->SetReturnValue(JSPARAM_MODULE, gears_blob.get());
}

void GearsCanvas::Clone(JsCallContext *context) {
  scoped_refptr<GearsCanvas> clone;
  if (!CreateModule<GearsCanvas>(module_environment_.get(), context, &clone)) {
    return;
    }

  clone->ResetCanvas(width(), height());
  assert(clone->skia_bitmap()->getPixels());
  clone->skia_canvas()->drawBitmap(*skia_bitmap_, 0, 0);

  assert(clone->skia_bitmap()->getSize() == skia_bitmap()->getSize());
  assert(memcmp(clone->skia_bitmap()->getPixels(),
      skia_bitmap()->getPixels(), skia_bitmap()->getSize()) == 0);

  clone->set_alpha(alpha());
  clone->set_composite_operation(composite_operation());
  clone->set_fill_style(fill_style());
  clone->set_font(font());
  clone->set_text_align(text_align());
  // TODO(nigeltao): Copy the transformation matrix and generally make sure that
  // all state is copied.

  context->SetReturnValue(JSPARAM_MODULE, clone.get());
}

void GearsCanvas::Crop(JsCallContext *context) {
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
  
  SkIRect src_rect = { x, y, x + width, y + height};
  if (!IsRectValid(src_rect)) {
    context->SetException(STRING16(L"Rectangle to crop stretches beyond the "
        L"bounds of the bitmap or has negative dimensions"));
    return;
  }
  SkBitmap new_bitmap;
  new_bitmap.setConfig(skia_config, width, height);
  new_bitmap.allocPixels();
  SkCanvas new_canvas(new_bitmap);
  SkRect dest_rect = { SkIntToScalar(0),
                       SkIntToScalar(0),
                       SkIntToScalar(width),
                       SkIntToScalar(height) };
  new_canvas.drawBitmapRect(*skia_bitmap_, &src_rect, dest_rect);
  new_bitmap.swap(*skia_bitmap_);
}

void GearsCanvas::Resize(JsCallContext *context) {
  int new_width, new_height;
  JsArgument args[] = {
    { JSPARAM_REQUIRED, JSPARAM_INT, &new_width },
    { JSPARAM_REQUIRED, JSPARAM_INT, &new_height }
  };
  context->GetArguments(ARRAYSIZE(args), args);
  if (context->is_exception_set())
    return;
  if (new_width < 0 || new_height < 0) {
    context->SetException(STRING16(L"Cannot resize to negative dimensions."));
    return;
  }
  SkBitmap new_bitmap;
  new_bitmap.setConfig(skia_config, new_width, new_height);
  new_bitmap.allocPixels();

  if (width() != 0 && height() != 0) {
    SkCanvas new_canvas(new_bitmap);
    SkScalar x_scale = SkDoubleToScalar(
        static_cast<double>(new_width) / width());
    SkScalar y_scale = SkDoubleToScalar(
        static_cast<double>(new_height) / height());
    if (!new_canvas.scale(x_scale, y_scale)) {
      context->SetException(STRING16(L"Could not resize the image."));
      return;
    }
    new_canvas.drawBitmap(*skia_bitmap_, SkIntToScalar(0), SkIntToScalar(0));
  } else {
    new_bitmap.eraseARGB(0, 0, 0, 0);
  }
  new_bitmap.swap(*skia_bitmap_);
}

void GearsCanvas::GetWidth(JsCallContext *context) {
  int its_width = width();
  context->SetReturnValue(JSPARAM_INT, &its_width);
}

void GearsCanvas::GetHeight(JsCallContext *context) {
  int its_height = height();
  context->SetReturnValue(JSPARAM_INT, &its_height);
}

void GearsCanvas::SetWidth(JsCallContext *context) {
  int new_width;
  JsArgument args[] = {
    { JSPARAM_REQUIRED, JSPARAM_INT, &new_width }
  };
  context->GetArguments(ARRAYSIZE(args), args);
  if (context->is_exception_set())
    return;
  ResetCanvas(new_width, height());
}

void GearsCanvas::SetHeight(JsCallContext *context) {
  int new_height;
  JsArgument args[] = {
    { JSPARAM_REQUIRED, JSPARAM_INT, &new_height }
  };
  context->GetArguments(ARRAYSIZE(args), args);
  if (context->is_exception_set())
    return;
  ResetCanvas(width(), new_height);
}

void GearsCanvas::GetContext(JsCallContext *context) {
  std::string16 context_id;
  JsArgument args[] = {
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &context_id },
  };
  context->GetArguments(ARRAYSIZE(args), args);
  if (context->is_exception_set())
    return;
  if (context_id != STRING16(L"gears-2d")) {
    context->SetReturnValue(JSPARAM_NULL, 0);
    // As per the HTML5 canvas spec.
    return;
  }
  // Make sure the rendering context is not destroyed before SetReturnValue().
  scoped_refptr<GearsCanvasRenderingContext2D> rendering_context_scoped_ptr;
  if (rendering_context_ == NULL) {
    if (!CreateModule<GearsCanvasRenderingContext2D>(
        module_environment_.get(), context, &rendering_context_scoped_ptr)) {
      return;
    }
    rendering_context_ = rendering_context_scoped_ptr.get();
    rendering_context_->InitCanvasField(this);
  }
  context->SetReturnValue(JSPARAM_MODULE, rendering_context_);
}

int GearsCanvas::width() const {
  return skia_bitmap()->width();
}

int GearsCanvas::height() const {
  return skia_bitmap()->height();
}

double GearsCanvas::alpha() const {
  return alpha_;
}

void GearsCanvas::set_alpha(double new_alpha) {
  if (new_alpha >= 0.0 && new_alpha <= 1.0)
    alpha_ = new_alpha;
}

std::string16 GearsCanvas::composite_operation() const {
  // Make sure it's a supported mode (neither a HTML5 canvas-only mode like
  // 'source-atop' nor some gibberish like 'foobar'). An invalid value should
  // not have been assigned to this variable in the first place.
  assert(ParseCompositeOperationString(composite_operation_) >= 0);
  return composite_operation_;
}

bool GearsCanvas::set_composite_operation(std::string16 new_composite_op) {
  int op_value = ParseCompositeOperationString(new_composite_op);
  if (op_value == COMPOSITE_MODE_HTML5_CANVAS_ONLY)
    return false;
  if (op_value != COMPOSITE_MODE_UNKNOWN)
    composite_operation_ = new_composite_op;
  return true;
}

std::string16 GearsCanvas::fill_style() const {
  return fill_style_;
}

void GearsCanvas::set_fill_style(std::string16 new_fill_style) {
  // TODO(nigeltao):
  // if (new_fill_style is not a valid CSS color)
  //  return;

  fill_style_ = new_fill_style;
}

std::string16 GearsCanvas::font() const {
  return font_;
}

void GearsCanvas::set_font(std::string16 new_font) {
  // TODO(nigeltao):
  // if (new_font is not a valid CSS font specification) {
  //   return;
  // }

  font_ = new_font;
}

std::string16 GearsCanvas::text_align() const {
  return text_align_;
}

void GearsCanvas::set_text_align(std::string16 new_text_align) {
  if (new_text_align != STRING16(L"left") &&
      new_text_align != STRING16(L"center") &&
      new_text_align != STRING16(L"right") &&
      new_text_align != STRING16(L"start") &&
      new_text_align != STRING16(L"end")) {
    return;
  }
  text_align_ = new_text_align;
}

void GearsCanvas::ResetCanvas(int width, int height) {
  // Since we're starting with a clean slate, let's reset the SkBitmap as well.
  // For some reason things don't work otherwise.
  // TODO(nigeltao): Figure out why.
  skia_bitmap_.reset(new SkBitmap);
  skia_bitmap_->setConfig(skia_config, width, height);

  // Must allocate pixels before performing any operations,
  // or assertions fire and some operations (like eraseARGB) fail silently.
  skia_bitmap_->allocPixels();
  skia_bitmap_->eraseARGB(0, 0, 0, 0);
  skia_canvas_.reset(new SkCanvas(*skia_bitmap_));

  alpha_ = 1.0;
  composite_operation_ = STRING16(L"source-over");
  fill_style_ = STRING16(L"#000000");
  font_ = STRING16(L"10px sans-serif");
  text_align_ = STRING16(L"start");
}

SkBitmap *GearsCanvas::skia_bitmap() const {
  return skia_bitmap_.get();
}

SkCanvas *GearsCanvas::skia_canvas() const {
  return skia_canvas_.get();
}

bool GearsCanvas::IsRectValid(const SkIRect &rect) {
  return rect.fLeft <= rect.fRight && rect.fTop <= rect.fBottom &&
      rect.fLeft >= 0 && rect.fTop >= 0 &&
      rect.fRight <= width() && rect.fBottom <= height();
}

// Skia's SkPorterDuff values are all non-negative.
const int GearsCanvas::COMPOSITE_MODE_HTML5_CANVAS_ONLY = -1;
const int GearsCanvas::COMPOSITE_MODE_UNKNOWN = -2;

int GearsCanvas::ParseCompositeOperationString(std::string16 op) {
  if (op == STRING16(L"source-atop") ||
      op == STRING16(L"source-in") ||
      op == STRING16(L"source-out") ||
      op == STRING16(L"destination-atop") ||
      op == STRING16(L"destination-in") ||
      op == STRING16(L"destination-atop") ||
      op == STRING16(L"lighter") ||
      op == STRING16(L"xor")) {
    return COMPOSITE_MODE_HTML5_CANVAS_ONLY;
  }
  if (op == STRING16(L"source-over"))
    return SkPorterDuff::kSrcOver_Mode;
  if (op == STRING16(L"copy"))
    return SkPorterDuff::kSrc_Mode;
  return COMPOSITE_MODE_UNKNOWN;
}
