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

#ifndef GEARS_LOCALSERVER_NPAPI_UPDATE_TASK_CR_H__
#define GEARS_LOCALSERVER_NPAPI_UPDATE_TASK_CR_H__

#include "gears/base/common/serialization.h"
#include "gears/localserver/common/update_task.h"
#include "gears/base/chrome/module_cr.h"


// TODO(michaeln): renamed to CRUpdateTask
class NPUpdateTask : public UpdateTask {
 public:
  NPUpdateTask(BrowsingContext *context) : UpdateTask(context) {}

  // Overriden to execute update tasks in the plugin process
  virtual bool StartUpdate(ManagedResourceStore *store);

 protected:
  // Overriden to ensure only one task per application runs at a time
  virtual void Run();

 private:
  static bool SetRunningTask(NPUpdateTask *task);
  static void ClearRunningTask(NPUpdateTask *task);
};


// TODO(michaeln): move to SerializableClassId enumeration
#define SERIALIZABLE_AUTOUPDATE_MESSAGE  ((SerializableClassId)1224)

// The message sent from browser process to plugin process for auto updates
class AutoUpdateMessage : public PluginMessage {
 public:
  AutoUpdateMessage() : store_id_(0), context_(0) {}
  AutoUpdateMessage(int64 store_id, CPBrowsingContext context)
      : store_id_(store_id), context_(context) {}

  int64 store_id_;
  CPBrowsingContext context_;

  // PluginMessage override
  virtual void OnMessageReceived();

  // Serailizable overrides
  virtual SerializableClassId GetSerializableClassId() const {
    return SERIALIZABLE_AUTOUPDATE_MESSAGE;
  }
  virtual bool Serialize(Serializer *out) const {
    out->WriteInt64(store_id_);
    out->WriteInt(context_);
    return true;
  }
  virtual bool Deserialize(Deserializer *in) {
    return (in->ReadInt64(&store_id_) &&
            in->ReadInt(reinterpret_cast<int*>(&context_)));
  }

  // Serializable registration
  static Serializable *New() {
    return new AutoUpdateMessage;
  }
  static void Register() {
    Serializable::RegisterClass(SERIALIZABLE_AUTOUPDATE_MESSAGE, New);
  }  
};

#endif  // GEARS_LOCALSERVER_NPAPI_UPDATE_TASK_CR_H__
