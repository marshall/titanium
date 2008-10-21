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

#ifdef OFFICIAL_BUILD
// The Image API has not been finalized for official builds.
#else

#include "gears/base/common/dispatcher.h"
#include "gears/base/common/module_wrapper.h"
#include "gears/blob/blob.h"
#include "gears/image/image.h"

DECLARE_GEARS_WRAPPER(GearsImage);

template<>
void Dispatcher<GearsImage>::Init() {
  RegisterMethod("crop", &GearsImage::Crop);
  RegisterMethod("drawImage", &GearsImage::DrawImage);
  RegisterMethod("flipHorizontal", &GearsImage::FlipHorizontal);
  RegisterMethod("flipVertical", &GearsImage::FlipVertical);
  RegisterMethod("resize", &GearsImage::Resize);
  RegisterMethod("rotate", &GearsImage::Rotate);
  RegisterMethod("toBlob", &GearsImage::ToBlob);
  RegisterProperty("width", &GearsImage::GetWidth, NULL);
  RegisterProperty("height", &GearsImage::GetHeight, NULL);
}

const std::string GearsImage::kModuleName("GearsImage");

void GearsImage::Resize(JsCallContext *context) {
  assert(image_.get());
  int width, height;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_INT, &width },
    { JSPARAM_REQUIRED, JSPARAM_INT, &height },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set()) return;

  std::string16 error;
  if (!image_->Resize(width, height, &error)) {
    context->SetException(error);
    return;
  }
}

void GearsImage::Crop(JsCallContext *context) {
  assert(image_.get());
  int x, y, width, height;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_INT, &x },
    { JSPARAM_REQUIRED, JSPARAM_INT, &y },
    { JSPARAM_REQUIRED, JSPARAM_INT, &width },
    { JSPARAM_REQUIRED, JSPARAM_INT, &height },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set()) return;

  std::string16 error;
  if (!image_->Crop(x, y, width, height, &error)) {
    context->SetException(error);
    return;
  }
}

void GearsImage::Rotate(JsCallContext *context) {
  assert(image_.get());
  int degrees;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_INT, &degrees },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set()) return;

  std::string16 error;
  if (!image_->Rotate(degrees, &error)) {
    context->SetException(error);
    return;
  }
}

void GearsImage::FlipHorizontal(JsCallContext *context) {
  assert(image_.get());
  std::string16 error;
  if (!image_->FlipHorizontal(&error)) {
    context->SetException(error);
    return;
  }
}

void GearsImage::FlipVertical(JsCallContext *context) {
  assert(image_.get());
  std::string16 error;
  if (!image_->FlipVertical(&error)) {
    context->SetException(error);
    return;
  }
}

void GearsImage::DrawImage(JsCallContext *context) {
  assert(image_.get());
  ModuleImplBaseClass *other_module;
  int x;
  int y;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_MODULE, &other_module },
    { JSPARAM_REQUIRED, JSPARAM_INT, &x },
    { JSPARAM_REQUIRED, JSPARAM_INT, &y },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set()) return;

  if (GearsImage::kModuleName != other_module->get_module_name()) {
    context->SetException(STRING16(L"First argument must be an Image."));
    return;
  }
  BackingImage *other_image =
      static_cast<GearsImage*>(other_module)->image_.get();
  assert(other_image);
  std::string16 error;
  if (!image_->DrawImage(other_image, x, y, &error)) {
    context->SetException(error);
    return;
  }
}

void GearsImage::GetWidth(JsCallContext *context) {
  assert(image_.get());
  int result = image_->Width();
  context->SetReturnValue(JSPARAM_INT, &result);
}

void GearsImage::GetHeight(JsCallContext *context) {
  assert(image_.get());
  int result = image_->Height();
  context->SetReturnValue(JSPARAM_INT, &result);
}

void GearsImage::ToBlob(JsCallContext *context) {
  assert(image_.get());
  std::string16 type = STRING16(L"");
  JsArgument argv[] = {
    { JSPARAM_OPTIONAL, JSPARAM_STRING16, &type },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set()) return;

  scoped_refptr<BlobInterface> blob(NULL);
  if (type.empty()) {
    image_->ToBlob(&blob);
  } else if (type == STRING16(L"image/png")) {
    image_->ToBlob(&blob, BackingImage::FORMAT_PNG);
  } else if (type == STRING16(L"image/gif")) {
    image_->ToBlob(&blob, BackingImage::FORMAT_GIF);
  } else if (type == STRING16(L"image/jpeg")) {
    image_->ToBlob(&blob, BackingImage::FORMAT_JPEG);
  } else {
    context->SetException(STRING16(
        L"Format must be either 'image/png', 'image/gif' or 'image/jpeg'."));
    return;
  }

  scoped_refptr<GearsBlob> gears_blob;
  if (!CreateModule<GearsBlob>(module_environment_.get(),
                               context, &gears_blob)) {
    return;
  }
  gears_blob->Reset(blob.get());
  context->SetReturnValue(JSPARAM_MODULE, gears_blob.get());
}

#endif  // not OFFICIAL_BUILD
