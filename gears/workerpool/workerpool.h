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

#ifndef GEARS_WORKERPOOL_WORKERPOOL_H__
#define GEARS_WORKERPOOL_WORKERPOOL_H__

#include "gears/base/common/base_class.h"
#include "gears/base/common/js_runner.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

class PoolThreadsManager;

class GearsWorkerPool
    : public ModuleImplBaseClass,
      public JsEventHandlerInterface {
 public:
  static const std::string kModuleName;

  // Need a default constructor to instance objects from the Factory.
  GearsWorkerPool();
  virtual ~GearsWorkerPool();

  // IN: string full_script
  // OUT: int worker_id
  void CreateWorker(JsCallContext *context);

  // IN: string url
  // OUT: int worker_id
  void CreateWorkerFromUrl(JsCallContext *context);

  // IN: -
  // OUT: -
  void AllowCrossOrigin(JsCallContext *context);

  // IN: variant message, int dest_worker_id
  // OUT: -
  void SendMessage(JsCallContext *context);

  // IN: function handler
  // OUT: -
  void SetOnmessage(JsCallContext *context);

  // IN: -
  // OUT: function
  void GetOnmessage(JsCallContext *context);

  // IN: function handler
  // OUT: -
  void SetOnerror(JsCallContext *context);

  // IN: -
  // OUT: function
  void GetOnerror(JsCallContext *context);

#ifdef DEBUG
  // IN: -
  // OUT: -
  void ForceGC(JsCallContext *context);
#endif

  void HandleEvent(JsEventType event_type);

 private:
  friend class PoolThreadsManager; // for SetThreadsManager

  void Initialize(); // okay to call this multiple times
  void SetThreadsManager(PoolThreadsManager *manager);

  PoolThreadsManager *threads_manager_;
  bool owns_threads_manager_;
  scoped_ptr<JsEventMonitor> unload_monitor_;

  DISALLOW_EVIL_CONSTRUCTORS(GearsWorkerPool);
};

#endif // GEARS_WORKERPOOL_WORKERPOOL_H__
