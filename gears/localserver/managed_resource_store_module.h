// Copyright 2005, Google Inc.
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

#ifndef GEARS_LOCALSERVER_MANAGED_RESOURCE_STORE_MODULE_H__
#define GEARS_LOCALSERVER_MANAGED_RESOURCE_STORE_MODULE_H__

#include "gears/base/common/base_class.h"
#include "gears/base/common/common.h"
#include "gears/base/common/js_runner.h"
#include "gears/base/common/message_service.h"
#include "gears/localserver/common/managed_resource_store.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

//-----------------------------------------------------------------------------
// GearsManagedResourceStore
//-----------------------------------------------------------------------------
class GearsManagedResourceStore
    : public ModuleImplBaseClass,
      public MessageObserverInterface,
      public JsEventHandlerInterface {
 public:
  GearsManagedResourceStore()
      : ModuleImplBaseClass("GearsManagedResourceStore") {}

  // IN: -
  // OUT: string
  void GetName(JsCallContext *context);

  // IN: -
  // OUT: string
  void GetRequiredCookie(JsCallContext *context);

  // IN: -
  // OUT: bool
  void GetEnabled(JsCallContext *context);
  // IN: bool enabled
  // OUT: -
  void SetEnabled(JsCallContext *context);

  // IN: -
  // OUT: string
  void GetManifestUrl(JsCallContext *context);
  // IN: string manifest_url
  // OUT: -
  void SetManifestUrl(JsCallContext *context);

  // IN: -
  // OUT: int
  void GetLastUpdateCheckTime(JsCallContext *context);

  // IN: -
  // OUT: int
  void GetUpdateStatus(JsCallContext *context);

  // IN: -
  // OUT: string
  void GetLastErrorMessage(JsCallContext *context);

  // IN: -
  // OUT: -
  void CheckForUpdate(JsCallContext *context);

  // IN: -
  // OUT: string
  void GetCurrentVersion(JsCallContext *context);

  // IN: -
  // OUT: function
  void GetOnerror(JsCallContext *context);
  // IN: function onerror
  // OUT: -
  void SetOnerror(JsCallContext *context);

  // IN: -
  // OUT: function
  void GetOnprogress(JsCallContext *context);
  // IN: function onprogress
  // OUT: -
  void SetOnprogress(JsCallContext *context);

  // IN: -
  // OUT: function
  void GetOncomplete(JsCallContext *context);
  // IN: function oncomplete
  // OUT: -
  void SetOncomplete(JsCallContext *context);

 protected:
  ~GearsManagedResourceStore();

 private:
  // JsEventHandlerInterface
  virtual void HandleEvent(JsEventType event_type);
  // MessageObserverInterface
  virtual void OnNotify(MessageService *service,
                        const char16 *topic,
                        const NotificationData *data);

  // Common helper for SetOn* methods
  void SetEventHandler(JsCallContext *context,
                       scoped_ptr<JsRootedCallback> *handler);

  ManagedResourceStore store_;
  scoped_ptr<JsRootedCallback> onerror_handler_;
  scoped_ptr<JsRootedCallback> onprogress_handler_;
  scoped_ptr<JsRootedCallback> oncomplete_handler_;
  scoped_ptr<JsEventMonitor> unload_monitor_;
  std::string16 observer_topic_;

  friend class GearsLocalServer;

  DISALLOW_EVIL_CONSTRUCTORS(GearsManagedResourceStore);
};

#endif // GEARS_LOCALSERVER_MANAGED_RESOURCE_STORE_MODULE_H__
