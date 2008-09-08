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

#include <assert.h> // TODO(cprince): use DCHECK() when have google3 logging
#include <queue>

#include "gears/workerpool/workerpool.h"

#include "gears/base/common/module_wrapper.h"
#include "gears/base/common/permissions_db.h"
#include "gears/base/common/url_utils.h"
#include "gears/localserver/common/http_request.h"
#if BROWSER_FF
#include "gears/workerpool/firefox/pool_threads_manager.h"
#elif BROWSER_IE
#include "gears/workerpool/ie/pool_threads_manager.h"
#elif BROWSER_NPAPI
#include "gears/workerpool/npapi/pool_threads_manager.h"
#endif
#include "third_party/scoped_ptr/scoped_ptr.h"

DECLARE_GEARS_WRAPPER(GearsWorkerPool);

// static
template <>
void Dispatcher<GearsWorkerPool>::Init() {
  RegisterMethod("createWorker", &GearsWorkerPool::CreateWorker);
  RegisterMethod("createWorkerFromUrl", &GearsWorkerPool::CreateWorkerFromUrl);
  RegisterMethod("allowCrossOrigin", &GearsWorkerPool::AllowCrossOrigin);
  RegisterMethod("sendMessage", &GearsWorkerPool::SendMessage);
  RegisterProperty("onmessage", &GearsWorkerPool::GetOnmessage,
                   &GearsWorkerPool::SetOnmessage);
  RegisterProperty("onerror", &GearsWorkerPool::GetOnerror,
                   &GearsWorkerPool::SetOnerror);
#ifdef DEBUG
  RegisterMethod("forceGC", &GearsWorkerPool::ForceGC);
#endif
}

const std::string GearsWorkerPool::kModuleName("GearsWorkerPool");

GearsWorkerPool::GearsWorkerPool()
    : ModuleImplBaseClass(kModuleName),
      threads_manager_(NULL),
      owns_threads_manager_(false) {
}

GearsWorkerPool::~GearsWorkerPool() {
  if (owns_threads_manager_) {
    assert(threads_manager_);
    threads_manager_->ShutDown();
  }

  if (threads_manager_) {
    threads_manager_->UninitWorkerThread();
    threads_manager_->Unref();
  }
}

void GearsWorkerPool::SetThreadsManager(PoolThreadsManager *manager) {
  assert(!threads_manager_);
  threads_manager_ = manager;
  threads_manager_->Ref();

  // Leave owns_threads_manager_ set to false.
  assert(!owns_threads_manager_);
}

void GearsWorkerPool::SetOnmessage(JsCallContext *context) {
  JsRootedCallback *function = NULL;
  JsArgument argv[] = {
    { JSPARAM_OPTIONAL, JSPARAM_FUNCTION, &function },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  scoped_ptr<JsRootedCallback> scoped_function(function);
  if (context->is_exception_set())
    return;

  Initialize();

  if (!threads_manager_->SetCurrentThreadMessageHandler(function)) {
    context->SetException(STRING16(L"Error setting onmessage handler"));
    return;
  }

  scoped_function.release();  // ownership was transferred on success
}

void GearsWorkerPool::GetOnmessage(JsCallContext *context) {
  // TODO(nigeltao): implement, a la HttpRequest::GetOnReadyStateChange.
  context->SetException(STRING16(L"Not Implemented"));
}

void GearsWorkerPool::SetOnerror(JsCallContext *context) {
  JsRootedCallback *function = NULL;
  JsArgument argv[] = {
    { JSPARAM_OPTIONAL, JSPARAM_FUNCTION, &function },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  scoped_ptr<JsRootedCallback> scoped_function(function);
  if (context->is_exception_set())
    return;

  Initialize();

  if (!threads_manager_->SetCurrentThreadErrorHandler(function)) {
    // Currently, the only reason this can fail is because of this one
    // particular error.
    // TODO(aa): We need a system throughout Gears for being able to handle
    // exceptions from deep inside the stack better.
    context->SetException(STRING16(L"The onerror property cannot be set on a "
                                   L"parent worker"));
    return;
  }

  scoped_function.release();  // ownership was transferred on success
}

void GearsWorkerPool::GetOnerror(JsCallContext *context) {
  // TODO(nigeltao): implement, a la HttpRequest::GetOnReadyStateChange.
  context->SetException(STRING16(L"Not Implemented"));
}

void GearsWorkerPool::CreateWorker(JsCallContext *context) {
  std::string16 full_script;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &full_script },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set())
    return;

  Initialize();

  int worker_id;
  bool succeeded = threads_manager_->CreateThread(full_script,
                                                  true,  // is_param_script
                                                  &worker_id);
  if (!succeeded) {
    context->SetException(STRING16(L"Internal error."));
    return;
  }

  context->SetReturnValue(JSPARAM_INT, &worker_id);
}

void GearsWorkerPool::CreateWorkerFromUrl(JsCallContext *context) {
  std::string16 url;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &url },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set())
    return;

  Initialize();

  // Make sure URLs are only fetched from the main thread.
  // TODO(michaeln): This HttpRequest limitation has been removed.
  //                 Add unit tests and remove the test below.
  if (EnvIsWorker()) {
    context->SetException(
        STRING16(L"createWorkerFromUrl() cannot be called from a worker."));
    return;
  }

  std::string16 absolute_url;
  ResolveAndNormalize(EnvPageLocationUrl().c_str(), url.c_str(), &absolute_url);

  SecurityOrigin script_origin;
  if (!script_origin.InitFromUrl(absolute_url.c_str())) {
    context->SetException(STRING16(L"Internal error."));
    return;
  }

  // We do not currently support file:// URLs. See bug:
  // http://code.google.com/p/google-gears/issues/detail?id=239.
  if (!HttpRequest::IsSchemeSupported(script_origin.scheme().c_str())) {
    std::string16 message(STRING16(L"URL scheme '"));
    message += script_origin.scheme();
    message += STRING16(L"' is not supported.");
    context->SetException(message.c_str());
    return;
  }

  // Enable the worker's origin for gears access if it isn't explicitly
  // disabled: specifically, we only enable the same permission types that are
  // already granted to the worker's owner.
  // NOTE: It is OK to do this here, even though there is a race with starting
  // the background thread. Even if permission is revoked before after this
  // happens that is no different than what happens if permission is revoked
  // after the thread starts.
  if (!script_origin.IsSameOrigin(EnvPageSecurityOrigin())) {
    PermissionsDB *db = PermissionsDB::GetDB();
    if (!db) {
      context->SetException(STRING16(L"Internal error."));
      return;
    }

    if (!db->EnableGearsForWorker(script_origin, EnvPageSecurityOrigin())) {
      std::string16 message(STRING16(L"Gears access is denied for url: "));
      message += absolute_url;
      message += STRING16(L".");
      context->SetException(message.c_str());
      return;
    }
  }

  int worker_id;
  bool succeeded = threads_manager_->CreateThread(absolute_url,
                                                  false,  // is_param_script
                                                  &worker_id);
  if (!succeeded) {
    context->SetException(STRING16(L"Internal error."));
    return;
  }

  context->SetReturnValue(JSPARAM_INT, &worker_id);
}

void GearsWorkerPool::AllowCrossOrigin(JsCallContext *context) {
  Initialize();
  
  if (owns_threads_manager_) {
    context->SetException(STRING16(L"Method is only used by child workers."));
    return;
  }

  threads_manager_->AllowCrossOrigin();
}

void GearsWorkerPool::SendMessage(JsCallContext *context) {
  Initialize();

  JsParamType message_body_type = context->GetArgumentType(0);
  // We marshal the first argument, with the extra caveat that you can't send
  // a JavaScript null or undefined as a message (although you can send an
  // array containing nulls and undefineds).
  if (message_body_type == JSPARAM_NULL ||
      message_body_type == JSPARAM_UNDEFINED) {
    context->SetException(
        STRING16(L"The message parameter has an invalid type."));
    return;
  }

  JsToken message_body;
  int dest_worker_id;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_TOKEN, &message_body },
    { JSPARAM_REQUIRED, JSPARAM_INT, &dest_worker_id },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set())
    return;

  std::string16 error;
  JsContextPtr cx = GetJsRunner()->GetContext();

  MarshaledJsToken *mjt = MarshaledJsToken::Marshal(
      message_body, GetJsRunner(), cx, &error);
  if (!mjt) {
    context->SetException(error.empty()
        ? STRING16(L"The message parameter has an invalid type.")
        : error.c_str());
    return;
  }

  // We also keep the old string-only message.text for backwards compatability.
  std::string16 text;
  if (message_body_type == JSPARAM_STRING16) {
    JsTokenToString_NoCoerce(message_body, cx, &text);
  }

  bool succeeded = threads_manager_->PutPoolMessage(mjt,
                                                    text,
                                                    dest_worker_id,
                                                    EnvPageSecurityOrigin());
  if (!succeeded) {
    std::string16 error(STRING16(L"Worker "));
    error += IntegerToString16(dest_worker_id);
    error += STRING16(L" does not exist.");
    context->SetException(error);
  }
}

#ifdef DEBUG
void GearsWorkerPool::ForceGC(JsCallContext *context) {
#if BROWSER_IE
  // TODO(aa): Investigate why this is crashing the unit tests.  In the
  // meantime, not a big deal for IE.  forceGC() was added for finding garbage
  // collection bugs on Firefox, where we control the JS engine more manually.
  // Though it can also be useful for debugging JsRootedToken on all browsers.
#else
  threads_manager_->ForceGCCurrentThread();
#endif
}
#endif // DEBUG

void GearsWorkerPool::HandleEvent(JsEventType event_type) {
  assert(event_type == JSEVENT_UNLOAD);

  if (owns_threads_manager_ && threads_manager_) {
    // Note: the following line can cause us to be deleted
    threads_manager_->ShutDown();
  }
}

void GearsWorkerPool::Initialize() {
  if (!threads_manager_) {
    assert(EnvPageSecurityOrigin().full_url() == EnvPageLocationUrl());
    SetThreadsManager(new PoolThreadsManager(EnvPageSecurityOrigin(),
                                             GetJsRunner(), this));
    owns_threads_manager_ = true;
  }

  // Monitor 'onunload' to shutdown threads when the page goes away.
  //
  // A thread that keeps running after the page changes can cause odd problems,
  // if it continues to send messages. (This can happen if it busy-loops.)  On
  // Firefox, such a thread triggered the Print dialog after the page changed!
  if (unload_monitor_ == NULL) {
    unload_monitor_.reset(new JsEventMonitor(GetJsRunner(), JSEVENT_UNLOAD,
                                             this));
  }
}
