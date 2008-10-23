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

#include "gears/localserver/localserver_module.h"

#include "gears/base/common/module_wrapper.h"
#include "gears/base/common/paths.h"
#include "gears/base/common/url_utils.h"
#include "gears/localserver/common/http_request.h"
#include "gears/localserver/managed_resource_store_module.h"
#include "gears/localserver/resource_store_module.h"

DECLARE_GEARS_WRAPPER(GearsLocalServer);

// static
template<>
void Dispatcher<GearsLocalServer>::Init() {
  RegisterMethod("canServeLocally", &GearsLocalServer::CanServeLocally);
  RegisterMethod("createManagedStore", &GearsLocalServer::CreateManagedStore);
  RegisterMethod("openManagedStore", &GearsLocalServer::OpenManagedStore);
  RegisterMethod("removeManagedStore", &GearsLocalServer::RemoveManagedStore);
  RegisterMethod("createStore", &GearsLocalServer::CreateStore);
  RegisterMethod("openStore", &GearsLocalServer::OpenStore);
  RegisterMethod("removeStore", &GearsLocalServer::RemoveStore);
}

const std::string GearsLocalServer::kModuleName("GearsLocalServer");

//-----------------------------------------------------------------------------
// CanServeLocally
//-----------------------------------------------------------------------------
void GearsLocalServer::CanServeLocally(JsCallContext *context) {
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

  bool can = LocalServer::CanServeLocally(full_url.c_str(),
                                          EnvPageBrowsingContext());
// TODO(cprince): remove #ifdef and string conversion after refactoring LOG().
#ifdef DEBUG
  std::string url_ascii;
  String16ToUTF8(url.c_str(), url.length(), &url_ascii);
  LOG(("LocalServer::CanServeLocally( %s ) %s\n",
       url_ascii.c_str(), can ? "TRUE" : "FALSE"));
#endif

  context->SetReturnValue(JSPARAM_BOOL, &can);
}

//-----------------------------------------------------------------------------
// CreateManagedStore
//-----------------------------------------------------------------------------
void GearsLocalServer::CreateManagedStore(JsCallContext *context) {
  std::string16 name;
  std::string16 required_cookie;
  if (!GetAndCheckParameters(context, &name, &required_cookie))
    return;

  // Check that this page uses a supported URL scheme.
  if (!HttpRequest::IsSchemeSupported(
                        EnvPageSecurityOrigin().scheme().c_str())) {
    context->SetException(STRING16(L"URL scheme not supported."));
    return;
  }

// TODO(cprince): remove #ifdef and string conversion after refactoring LOG().
#ifdef DEBUG
  std::string name_ascii;
  String16ToUTF8(name.c_str(), name.length(), &name_ascii);
  std::string required_cookie_ascii;
  String16ToUTF8(required_cookie.c_str(), required_cookie.length(), 
                 &required_cookie_ascii);
  LOG(("LocalServer::CreateManagedStore( %s, %s )\n",
         name_ascii.c_str(), required_cookie_ascii.c_str()));
#endif

  scoped_refptr<GearsManagedResourceStore> store;
  if (!CreateModule<GearsManagedResourceStore>(module_environment_.get(),
                                               context, &store)) {
    return;
  }

  if (!store->store_.CreateOrOpen(EnvPageSecurityOrigin(),
                                  name.c_str(), required_cookie.c_str())) {
    context->SetException(
        STRING16(L"Error initializing ManagedResourceStore."));
    return;
  }

  context->SetReturnValue(JSPARAM_MODULE, store.get());
}

//-----------------------------------------------------------------------------
// OpenManagedStore
//-----------------------------------------------------------------------------
void GearsLocalServer::OpenManagedStore(JsCallContext *context) {
  std::string16 name;
  std::string16 required_cookie;
  if (!GetAndCheckParameters(context, &name, &required_cookie))
    return;

// TODO(cprince): remove #ifdef and string conversion after refactoring LOG().
#ifdef DEBUG
  std::string name_ascii;
  String16ToUTF8(name.c_str(), name.length(), &name_ascii);
  std::string required_cookie_ascii;
  String16ToUTF8(required_cookie.c_str(), required_cookie.length(), 
                 &required_cookie_ascii);
  LOG(("LocalServer::OpenManagedStore( %s, %s )\n",
         name_ascii.c_str(), required_cookie_ascii.c_str()));
#endif

  int64 existing_store_id = WebCacheDB::kUnknownID;
  if (!ManagedResourceStore::ExistsInDB(EnvPageSecurityOrigin(),
                                        name.c_str(),
                                        required_cookie.c_str(),
                                        &existing_store_id)) {
    context->SetReturnValue(JSPARAM_NULL, 0);
    return;
  }

  scoped_refptr<GearsManagedResourceStore> store;
  if (!CreateModule<GearsManagedResourceStore>(module_environment_.get(),
                                               context, &store)) {
    return;
  }

  if (!store->store_.Open(existing_store_id)) {
    context->SetException(
        STRING16(L"Error initializing ManagedResourceStore."));
    return;
  }

  context->SetReturnValue(JSPARAM_MODULE, store.get());
}

//-----------------------------------------------------------------------------
// RemoveManagedStore
//-----------------------------------------------------------------------------
void GearsLocalServer::RemoveManagedStore(JsCallContext *context) {
  std::string16 name;
  std::string16 required_cookie;
  if (!GetAndCheckParameters(context, &name, &required_cookie))
    return;

// TODO(cprince): remove #ifdef and string conversion after refactoring LOG().
#ifdef DEBUG
  std::string name_ascii;
  String16ToUTF8(name.c_str(), name.length(), &name_ascii);
  std::string required_cookie_ascii;
  String16ToUTF8(required_cookie.c_str(), required_cookie.length(), 
                 &required_cookie_ascii);
  LOG(("LocalServer::RemoveManagedStore( %s, %s )\n",
         name_ascii.c_str(), required_cookie_ascii.c_str()));
#endif
  
  int64 existing_store_id = WebCacheDB::kUnknownID;
  if (!ManagedResourceStore::ExistsInDB(EnvPageSecurityOrigin(),
                                        name.c_str(),
                                        required_cookie.c_str(),
                                        &existing_store_id)) {
    context->SetReturnValue(JSPARAM_NULL, 0);
    return;
  }

  ManagedResourceStore store;
  if (!store.Open(existing_store_id)) {
    context->SetException(
        STRING16(L"Error initializing ManagedResourceStore."));
    return;
  }

  if (!store.Remove()) {
    context->SetException(STRING16(L"Error removing ManagedResourceStore."));
    return;
  }
}

//-----------------------------------------------------------------------------
// CreateStore
//-----------------------------------------------------------------------------
void GearsLocalServer::CreateStore(JsCallContext *context) {
  std::string16 name;
  std::string16 required_cookie;
  if (!GetAndCheckParameters(context, &name, &required_cookie))
    return;

  // Check that this page uses a supported URL scheme.
  if (!HttpRequest::IsSchemeSupported(
                        EnvPageSecurityOrigin().scheme().c_str())) {
    context->SetException(STRING16(L"URL scheme not supported."));
    return;
  }

// TODO(cprince): remove #ifdef and string conversion after refactoring LOG().
#ifdef DEBUG
  std::string name_ascii;
  String16ToUTF8(name.c_str(), name.length(), &name_ascii);
  std::string required_cookie_ascii;
  String16ToUTF8(required_cookie.c_str(), required_cookie.length(), 
                 &required_cookie_ascii);
  LOG(("LocalServer::CreateStore( %s, %s )\n",
         name_ascii.c_str(), required_cookie_ascii.c_str()));
#endif

  scoped_refptr<GearsResourceStore> store;
  if (!CreateModule<GearsResourceStore>(module_environment_.get(),
                                        context, &store)) {
    return;
  }

  if (!store->store_.CreateOrOpen(EnvPageSecurityOrigin(),
                                  name.c_str(), required_cookie.c_str())) {
    context->SetException(STRING16(L"Error initializing ResourceStore."));
    return;
  }

  context->SetReturnValue(JSPARAM_MODULE, store.get());
}

//-----------------------------------------------------------------------------
// OpenStore
//-----------------------------------------------------------------------------
void GearsLocalServer::OpenStore(JsCallContext *context) {
  std::string16 name;
  std::string16 required_cookie;
  if (!GetAndCheckParameters(context, &name, &required_cookie))
    return;

// TODO(cprince): remove #ifdef and string conversion after refactoring LOG().
#ifdef DEBUG
  std::string name_ascii;
  String16ToUTF8(name.c_str(), name.length(), &name_ascii);
  std::string required_cookie_ascii;
  String16ToUTF8(required_cookie.c_str(), required_cookie.length(), 
                 &required_cookie_ascii);
  LOG(("LocalServer::OpenStore( %s, %s )\n",
         name_ascii.c_str(), required_cookie_ascii.c_str()));
#endif

  int64 existing_store_id = WebCacheDB::kUnknownID;
  if (!ResourceStore::ExistsInDB(EnvPageSecurityOrigin(),
                                 name.c_str(),
                                 required_cookie.c_str(),
                                 &existing_store_id)) {
    context->SetReturnValue(JSPARAM_NULL, 0);
    return;
  }

  scoped_refptr<GearsResourceStore> store;
  if (!CreateModule<GearsResourceStore>(module_environment_.get(),
                                        context, &store)) {
    return;
  }

  if (!store->store_.Open(existing_store_id)) {
    context->SetException(STRING16(L"Error initializing ResourceStore."));
    return;
  }

  context->SetReturnValue(JSPARAM_MODULE, store.get());
}

//-----------------------------------------------------------------------------
// RemoveStore
//-----------------------------------------------------------------------------
void GearsLocalServer::RemoveStore(JsCallContext *context) {
  std::string16 name;
  std::string16 required_cookie;
  if (!GetAndCheckParameters(context, &name, &required_cookie))
    return;

// TODO(cprince): remove #ifdef and string conversion after refactoring LOG().
#ifdef DEBUG
  std::string name_ascii;
  String16ToUTF8(name.c_str(), name.length(), &name_ascii);
  std::string required_cookie_ascii;
  String16ToUTF8(required_cookie.c_str(), required_cookie.length(), 
                 &required_cookie_ascii);
  LOG(("LocalServer::RemoveStore( %s, %s )\n",
         name_ascii.c_str(), required_cookie_ascii.c_str()));
#endif

  int64 existing_store_id = WebCacheDB::kUnknownID;
  if (!ResourceStore::ExistsInDB(EnvPageSecurityOrigin(),
                                 name.c_str(),
                                 required_cookie.c_str(),
                                 &existing_store_id)) {
    return;
  }

  ResourceStore store;
  if (!store.Open(existing_store_id)) {
    context->SetException(STRING16(L"Error initializing ResourceStore."));
    return;
  }

  if (!store.Remove()) {
    context->SetException(STRING16(L"Error removing ResourceStore."));
    return;
  }
}

//------------------------------------------------------------------------------
// GetAndCheckParameters
//------------------------------------------------------------------------------
bool GearsLocalServer::GetAndCheckParameters(JsCallContext *context,
                                             std::string16 *name,
                                             std::string16 *required_cookie) {
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_STRING16, name },
    { JSPARAM_OPTIONAL, JSPARAM_STRING16, required_cookie },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set())
    return false;

  // Validate parameters
  if (name->empty()) {
    context->SetException(STRING16(L"The name parameter is required."));
    return false;
  }

  // TODO(michaeln): validate the required_cookie parameter value, parse the
  // name & value, name must not be empty, value must not contain ';' unless
  // it's the kNegatedRequiredCookieValue.
  std::string16 error_message;
  if (!IsUserInputValidAsPathComponent(*name, &error_message)) {
    context->SetException(error_message.c_str());
    return false;
  }

  return true;
}
