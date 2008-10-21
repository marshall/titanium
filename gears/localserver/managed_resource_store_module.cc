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

#include "gears/localserver/managed_resource_store_module.h"

#include "gears/base/common/module_wrapper.h"
#include "gears/base/common/url_utils.h"
#include "gears/localserver/common/update_task.h"

DECLARE_GEARS_WRAPPER(GearsManagedResourceStore);

// static
template<>
void Dispatcher<GearsManagedResourceStore>::Init() {
  RegisterProperty("name", &GearsManagedResourceStore::GetName, NULL);
  RegisterProperty("requiredCookie",
                   &GearsManagedResourceStore::GetRequiredCookie, NULL);
  RegisterProperty("enabled", &GearsManagedResourceStore::GetEnabled,
                   &GearsManagedResourceStore::SetEnabled);
  RegisterProperty("manifestUrl", &GearsManagedResourceStore::GetManifestUrl,
                   &GearsManagedResourceStore::SetManifestUrl);
  RegisterProperty("lastUpdateCheckTime",
                   &GearsManagedResourceStore::GetLastUpdateCheckTime, NULL);
  RegisterProperty("updateStatus",
                   &GearsManagedResourceStore::GetUpdateStatus, NULL);
  RegisterProperty("lastErrorMessage",
                   &GearsManagedResourceStore::GetLastErrorMessage, NULL);
  RegisterProperty("currentVersion",
                   &GearsManagedResourceStore::GetCurrentVersion, NULL);
  RegisterProperty("onerror", &GearsManagedResourceStore::GetOnerror,
                   &GearsManagedResourceStore::SetOnerror);
  RegisterProperty("onprogress", &GearsManagedResourceStore::GetOnprogress,
                   &GearsManagedResourceStore::SetOnprogress);
  RegisterProperty("oncomplete", &GearsManagedResourceStore::GetOncomplete,
                   &GearsManagedResourceStore::SetOncomplete);
  RegisterMethod("checkForUpdate",
                 &GearsManagedResourceStore::CheckForUpdate);
}

GearsManagedResourceStore::~GearsManagedResourceStore() {
  MessageService::GetInstance()->RemoveObserver(this,
                                                observer_topic_.c_str());
}

//------------------------------------------------------------------------------
// GetName
//------------------------------------------------------------------------------
void GearsManagedResourceStore::GetName(JsCallContext *context) {
  std::string16 name(store_.GetName());
  context->SetReturnValue(JSPARAM_STRING16, &name);
}

//------------------------------------------------------------------------------
// GetRequiredCookie
//------------------------------------------------------------------------------
void GearsManagedResourceStore::GetRequiredCookie(JsCallContext *context) {
  std::string16 cookie(store_.GetRequiredCookie());
  context->SetReturnValue(JSPARAM_STRING16, &cookie);
}

//------------------------------------------------------------------------------
// GetEnabled
//------------------------------------------------------------------------------
void GearsManagedResourceStore::GetEnabled(JsCallContext *context) {
  bool enabled = store_.IsEnabled();
  context->SetReturnValue(JSPARAM_BOOL, &enabled);
}

//------------------------------------------------------------------------------
// SetEnabled
//------------------------------------------------------------------------------
void GearsManagedResourceStore::SetEnabled(JsCallContext *context) {
  bool enabled;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_BOOL, &enabled },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set())
    return;

  if (!store_.SetEnabled(enabled)) {
    context->SetException(STRING16(L"Failed to set the enabled property."));
    return;
  }
}

//------------------------------------------------------------------------------
// GetManifestUrl
//------------------------------------------------------------------------------
void GearsManagedResourceStore::GetManifestUrl(JsCallContext *context) {
  std::string16 manifest_url;
  if (store_.GetManifestUrl(&manifest_url)) {
    context->SetReturnValue(JSPARAM_STRING16, &manifest_url);
  } else {
    context->SetException(STRING16(L"Failed to get manifest url."));
  }
}

//------------------------------------------------------------------------------
// SetManifestUrl
//------------------------------------------------------------------------------
void GearsManagedResourceStore::SetManifestUrl(JsCallContext *context) {
  std::string16 url;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &url },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set())
    return;

  std::string16 full_url;
  if (!ResolveAndNormalize(EnvPageLocationUrl().c_str(), url.c_str(),
                           &full_url)) {
    context->SetException(STRING16(L"Failed to resolve url."));
    return;
  }

  if (!EnvPageSecurityOrigin().IsSameOriginAsUrl(full_url.c_str())) {
    context->SetException(STRING16(L"Url is not from the same origin"));
    return;
  }

  if (!store_.SetManifestUrl(full_url.c_str())) {
    context->SetException(STRING16(L"Failed to set manifest url."));
    return;
  }
}

//------------------------------------------------------------------------------
// GetLastUpdateCheckTime
//------------------------------------------------------------------------------
void GearsManagedResourceStore::GetLastUpdateCheckTime(JsCallContext *context) {
  WebCacheDB::UpdateStatus status;
  int64 time64 = 0;
  if (store_.GetUpdateInfo(&status, &time64, NULL, NULL)) {
    int time = static_cast<int>(time64 / 1000);  // convert to seconds
    context->SetReturnValue(JSPARAM_INT, &time);
  } else {
    context->SetException(STRING16(L"Failed to get update info."));
    return;
  }
}

//------------------------------------------------------------------------------
// GetUpdateStatus
//------------------------------------------------------------------------------
void GearsManagedResourceStore::GetUpdateStatus(JsCallContext *context) {
  WebCacheDB::UpdateStatus status = WebCacheDB::UPDATE_OK;
  int64 time64 = 0;
  if (store_.GetUpdateInfo(&status, &time64, NULL, NULL)) {
    int return_status = static_cast<int>(status);
    context->SetReturnValue(JSPARAM_INT, &return_status);
  } else {
    context->SetException(STRING16(L"Failed to get update info."));
    return;
  }
}

//------------------------------------------------------------------------------
// GetLastErrorMessage
//------------------------------------------------------------------------------
void GearsManagedResourceStore::GetLastErrorMessage(JsCallContext *context) {
  WebCacheDB::UpdateStatus status;
  int64 time64 = 0;
  std::string16 error_message;
  if (store_.GetUpdateInfo(&status, &time64, NULL, &error_message)) {
    context->SetReturnValue(JSPARAM_STRING16, &error_message);
  } else {
    context->SetException(STRING16(L"Failed to get last error message."));
    return;
  }
}

//------------------------------------------------------------------------------
// CheckForUpdate
//------------------------------------------------------------------------------
void GearsManagedResourceStore::CheckForUpdate(JsCallContext *context) {
  scoped_ptr<UpdateTask> update_task(
      UpdateTask::CreateUpdateTask(EnvPageBrowsingContext()));

  if (!update_task->StartUpdate(&store_)) {
    context->SetException(STRING16(L"Failed to start update task."));
    return;
  }

  // We wait here so the updateStatus property reflects a running task
  // upon return
  update_task->AwaitStartup();

  update_task.release()->DeleteWhenDone();
}

//------------------------------------------------------------------------------
// GetCurrentVersion
//------------------------------------------------------------------------------
void GearsManagedResourceStore::GetCurrentVersion(JsCallContext *context) {
  std::string16 version;
  store_.GetVersionString(WebCacheDB::VERSION_CURRENT, &version);
  context->SetReturnValue(JSPARAM_STRING16, &version);
}

//------------------------------------------------------------------------------
// GetOnerror
//------------------------------------------------------------------------------
void GearsManagedResourceStore::GetOnerror(JsCallContext *context) {
  context->SetException(STRING16(L"This property is write only."));
}

//------------------------------------------------------------------------------
// SetOnerror
//------------------------------------------------------------------------------
void GearsManagedResourceStore::SetOnerror(JsCallContext *context) {
  SetEventHandler(context, &onerror_handler_);
}

//------------------------------------------------------------------------------
// GetOnprogress
//------------------------------------------------------------------------------
void GearsManagedResourceStore::GetOnprogress(JsCallContext *context) {
  context->SetException(STRING16(L"This property is write only."));
}

//------------------------------------------------------------------------------
// SetOnprogress
//------------------------------------------------------------------------------
void GearsManagedResourceStore::SetOnprogress(JsCallContext *context) {
  SetEventHandler(context, &onprogress_handler_);
}

//------------------------------------------------------------------------------
// GetOncomplete
//------------------------------------------------------------------------------
void GearsManagedResourceStore::GetOncomplete(JsCallContext *context) {
  context->SetException(STRING16(L"This property is write only."));
}

//------------------------------------------------------------------------------
// SetOncomplete
//------------------------------------------------------------------------------
void GearsManagedResourceStore::SetOncomplete(JsCallContext *context) {
  SetEventHandler(context, &oncomplete_handler_);
}

//------------------------------------------------------------------------------
// SetEventHandler
//------------------------------------------------------------------------------
void GearsManagedResourceStore::SetEventHandler(
    JsCallContext *context, scoped_ptr<JsRootedCallback> *handler) {
  JsRootedCallback *function = NULL;
  JsArgument argv[] = {
    { JSPARAM_OPTIONAL, JSPARAM_FUNCTION, &function },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  scoped_ptr<JsRootedCallback> scoped_function(function);
  if (context->is_exception_set())
    return;

  // Create an event monitor to alert us when the page unloads.
  if (unload_monitor_ == NULL) {
    unload_monitor_.reset(new JsEventMonitor(GetJsRunner(), JSEVENT_UNLOAD,
                                             this));

    // Add the callbacks to handle deserializing messages.
    UpdateTask::RegisterEventClasses();
  }

  handler->swap(scoped_function);  // transfer ownership
  if (function) {
    observer_topic_ = UpdateTask::GetNotificationTopic(&store_);
    MessageService::GetInstance()->AddObserver(this, observer_topic_.c_str());
  }
}

//------------------------------------------------------------------------------
// HandleEvent
//------------------------------------------------------------------------------
void GearsManagedResourceStore::HandleEvent(JsEventType event_type) {
  assert(event_type == JSEVENT_UNLOAD);

  // Drop references, js context is going away.
  onerror_handler_.reset();
  onprogress_handler_.reset();
  oncomplete_handler_.reset();
  MessageService::GetInstance()->RemoveObserver(this,
                                                observer_topic_.c_str());
}

//------------------------------------------------------------------------------
// OnNotify
//------------------------------------------------------------------------------
void GearsManagedResourceStore::OnNotify(MessageService *service,
                                         const char16 *topic,
                                         const NotificationData *data) {
  scoped_ptr<JsObject> param;
  JsRootedCallback *handler = 0;

  const UpdateTask::Event *event = static_cast<const UpdateTask::Event *>(data);
  switch(event->event_type()) {
    case UpdateTask::ERROR_EVENT: {
        if (!onerror_handler_.get()) return;
        handler = onerror_handler_.get();

        const UpdateTask::ErrorEvent *error_event =
            static_cast<const UpdateTask::ErrorEvent *>(data);
        param.reset(GetJsRunner()->NewError(error_event->error_message()));
        if (!param.get()) return;
      }
      break;

    case UpdateTask::PROGRESS_EVENT: {
        if (!onprogress_handler_.get()) return;
        handler = onprogress_handler_.get();

        param.reset(GetJsRunner()->NewObject());
        if (!param.get()) return;

        const UpdateTask::ProgressEvent *progress_event =
            static_cast<const UpdateTask::ProgressEvent *>(data);
        param->SetPropertyInt(STRING16(L"filesTotal"),
                              progress_event->files_total());
        param->SetPropertyInt(STRING16(L"filesComplete"),
                              progress_event->files_complete());
      }
      break;

    case UpdateTask::COMPLETION_EVENT: {
        if (!oncomplete_handler_.get()) return;
        handler = oncomplete_handler_.get();

        param.reset(GetJsRunner()->NewObject());
        if (!param.get()) return;

        const UpdateTask::CompletionEvent *completion_event =
            static_cast<const UpdateTask::CompletionEvent *>(data);
        param->SetPropertyString(STRING16(L"newVersion"),
                                 completion_event->new_version_string());
      }
      break;

    default:
      return;
  }

  const int argc = 1;
  JsParamToSend argv[argc] = {
    { JSPARAM_OBJECT, param.get() }
  };
  GetJsRunner()->InvokeCallback(handler, argc, argv, NULL);
}
