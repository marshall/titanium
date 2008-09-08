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
// The Dummy module is not included in official builds.
#else

// TODO: change this location to your module
#include "gears/dummy/dummy_module.h"

#include "gears/base/common/base_class.h"
#include "gears/base/common/common.h"
#include "gears/base/common/dispatcher.h"
#include "gears/base/common/module_wrapper.h"

// TODO: Replace GearsDummyModule with the name of your module.

// Define the dispatcher for your Module.
DECLARE_GEARS_WRAPPER(GearsDummyModule);

template<>
void Dispatcher<GearsDummyModule>::Init() {
  // TODO: Register methods and properties that are exposed to JavaScript.
  RegisterMethod("method", &GearsDummyModule::Method);
  // Property can have both a getter and a setter. To make property read-only,
  // specify NULL as the setter.
  RegisterProperty("property", &GearsDummyModule::GetProperty,
                   &GearsDummyModule::SetProperty);
}

void GearsDummyModule::Method(JsCallContext *context) {
  std::string16 first_argument;
  int second_argument;

  // Specify which arguments and types to read and whether they are optional.
  // JSPARAM_REQUIRED may not appear after JSPARAM_OPTIONAL in declaration.
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &first_argument },
    { JSPARAM_OPTIONAL, JSPARAM_INT, &second_argument }
  };

  // Read arguments.
  int argument_count = context->GetArguments(ARRAYSIZE(argv), argv);

  // Context will throw an exception if required arguments are missing or
  // argument types are mismatched. Check for this and return if exception has
  // been thrown.
  if (context->is_exception_set()) return;

  // You can check for number of arguments with which the method was called and
  // adjust your logic accordingly.
  if (argument_count < 2) {
    second_argument = -1;
  }

  // TODO: Implement your method here.

  // Return value.
  context->SetReturnValue(JSPARAM_INT, &second_argument);
}

void GearsDummyModule::GetProperty(JsCallContext *context) {
  // TODO: Implement additional property getter logic here.

  // Reaturn value of specified type.
  context->SetReturnValue(JSPARAM_BOOL, &property_value_);
}

void GearsDummyModule::SetProperty(JsCallContext *context) {
  // Specify arguments and types. The mechanism is consistent with the way
  // method arguments are read.
  JsArgument argv[] = {
    { JSPARAM_OPTIONAL, JSPARAM_BOOL, &property_value_ }
  };

  // Read property setter argument.
  context->GetArguments(ARRAYSIZE(argv), argv);

  // If exception was thrown while reading the argument, return.
  if (context->is_exception_set()) return;

  // TODO: Implement additional property logic, if necessary.
}

#endif
