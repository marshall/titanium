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

#ifndef GEARS_DUMMY_DUMMY_MODULE_H__
#define GEARS_DUMMY_DUMMY_MODULE_H__

#include "gears/base/common/base_class.h"
#include "gears/base/common/common.h"
#include "gears/base/common/js_types.h"

// TODO: Provide detailed description of the module.
class GearsDummyModule : public ModuleImplBaseClass {
 public:
  GearsDummyModule()
      : ModuleImplBaseClass("GearsDummyModule"),
        property_value_(false) {}
  // TODO: For each method and property, provide input/output specification

  // IN: string firstArgument, optional int secondArgument
  // OUT: int
  void Method(JsCallContext *context);

  // OUT: bool propertyValue
  void GetProperty(JsCallContext *context);
  // IN: bool propertValue
  void SetProperty(JsCallContext *context);

 private:
  bool property_value_;

  // make copy constructors private
  DISALLOW_EVIL_CONSTRUCTORS(GearsDummyModule);
};

#endif // GEARS_DUMMY_DUMMY_MODULE_H__
