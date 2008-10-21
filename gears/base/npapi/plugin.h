// Copyright 2007, Google Inc.
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

#ifndef GEARS_BASE_NPAPI_PLUGIN_H__
#define GEARS_BASE_NPAPI_PLUGIN_H__

#include "gears/base/common/base_class.h"
#include "gears/base/common/dispatcher.h"

class JsCallContext;

// This is a base class for the bridge between the JavaScript engine and the
// Gears class implementations.  See ModuleWrapper for an example.
class PluginBase : public NPObject {
 public:
  // NPClass callbacks.  The browser calls these functions when JavaScript
  // interacts with a Gears object.
  template<class PluginClass>
  static NPObject* Allocate(NPP npp, NPClass *npclass) {
    return new PluginClass(npp);
  }
  static void Deallocate(NPObject *npobj);
  static bool HasMethod(NPObject *npobj, NPIdentifier name);
  static bool Invoke(NPObject *npobj, NPIdentifier name,
                     const NPVariant *args, uint32_t num_args,
                     NPVariant *result);
  static bool HasProperty(NPObject *npobj, NPIdentifier name);
  static bool GetProperty(NPObject *npobj, NPIdentifier name,
                          NPVariant *result);
  static bool SetProperty(NPObject *npobj, NPIdentifier name,
                          const NPVariant *value);
  PluginBase(NPP instance) : instance_(instance), dispatcher_(NULL) {}
  virtual ~PluginBase() {}

  // Secondary init function that requires more information than we have
  // at object creation time.
  void Init(DispatcherInterface *dispatcher) {
    dispatcher_ = dispatcher;
  }

 protected:
  NPP instance_;
  DispatcherInterface *dispatcher_;

 private:
  DISALLOW_EVIL_CONSTRUCTORS(PluginBase);
};

// Get the NPClass for a Plugin type (the type must derive from PluginBase).
// Note that this just returns a temporary that can be used to initialize a
// global.  The reason for not simply returning a static here is that this is a
// non-member inline function, which means different copies of the static may
// be created for the different compilation units that call this function.
template<class Plugin>
NPClass GetNPClassTemplate() {
  NPClass plugin_class = {
    NP_CLASS_STRUCT_VERSION,
    PluginBase::Allocate<Plugin>,
    PluginBase::Deallocate,
    NULL,  // PluginBase::Invalidate,
    PluginBase::HasMethod,
    PluginBase::Invoke,
    NULL,  // PluginBase::InvokeDefault,
    PluginBase::HasProperty,
    PluginBase::GetProperty,
    PluginBase::SetProperty,
    NULL,  // PluginBase::RemoveProperty,
  };

  return plugin_class;
}

#endif // GEARS_BASE_NPAPI_PLUGIN_H__
