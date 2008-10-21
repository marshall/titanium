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
// The Image API has not been finalized for official builds
#else

#include "gears/base/common/dispatcher.h"
#include "gears/base/common/module_wrapper.h"
#include "gears/blob/blob.h"
#include "gears/image/image.h"
#include "gears/image/image_loader.h"

DECLARE_GEARS_WRAPPER(GearsImageLoader);

template<>
void Dispatcher<GearsImageLoader>::Init() {
  RegisterMethod("createImageFromBlob", &GearsImageLoader::CreateImageFromBlob);
}

void GearsImageLoader::CreateImageFromBlob(JsCallContext *context) {
  ModuleImplBaseClass *other_module = NULL;

  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_MODULE, &other_module },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set()) return;

  // Extract the blob.
  if (GearsBlob::kModuleName != other_module->get_module_name()) {
    context->SetException(STRING16(L"First argument must be a Blob."));
    return;
  }
  scoped_refptr<BlobInterface> blob_contents;
  static_cast<GearsBlob*>(other_module)->GetContents(&blob_contents);
  assert(blob_contents.get());

  // Create the backing_image.
  scoped_ptr<BackingImage> backing_image(new BackingImage());
  std::string16 error;
  if (!backing_image->Init(blob_contents.get(), &error)) {
    context->SetException(error.c_str());
    return;
  }

  // Create and return the gears_image.
  scoped_refptr<GearsImage> gears_image;
  if (!CreateModule<GearsImage>(module_environment_.get(),
                                context, &gears_image)) {
    return;
  }
  gears_image->Init(backing_image.release());
  context->SetReturnValue(JSPARAM_MODULE, gears_image.get());
}

#endif  // not OFFICIAL_BUILD
