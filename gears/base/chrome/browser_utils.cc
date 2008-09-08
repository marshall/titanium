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

#include "gears/base/npapi/browser_utils.h"

#include <stack>

#include "gears/base/chrome/browsing_context_cr.h"
#include "gears/base/chrome/module_cr.h"
#include "gears/base/common/js_types.h"
#include "gears/base/common/string_utils.h"
#include "gears/base/common/thread_locals.h"
#include "gears/base/npapi/np_utils.h"
#include "gears/base/npapi/scoped_npapi_handles.h"

extern std::string16 g_user_agent;  // Defined in base/npapi/npp_bindings.cc

typedef std::stack<JsCallContext*> JsCallStack;
static const ThreadLocals::Slot kJsCallStackKey = ThreadLocals::Alloc();

static void DeleteJsCallStack(void *context) {
  JsCallStack *call_stack = reinterpret_cast<JsCallStack*>(context);
  delete call_stack;
}

static JsCallStack &GetJsCallStack() {
  JsCallStack *call_stack =
      reinterpret_cast<JsCallStack*>(ThreadLocals::GetValue(kJsCallStackKey));
  if (!call_stack) {
    call_stack = new JsCallStack;
    ThreadLocals::SetValue(kJsCallStackKey, call_stack, &DeleteJsCallStack);
  }
  return *call_stack;
}

void BrowserUtils::EnterScope(JsCallContext *context) {
  GetJsCallStack().push(context);
}

void BrowserUtils::ExitScope() {
  assert(!GetJsCallStack().empty());
  GetJsCallStack().pop();
}

void BrowserUtils::SetJsException(const std::string16& message) {
  assert(!GetJsCallStack().empty());
  GetJsCallStack().top()->SetException(message);
}

JsCallContext *BrowserUtils::GetCurrentJsCallContext() {
  if (GetJsCallStack().empty())
    return NULL;
  return GetJsCallStack().top();
}

bool BrowserUtils::GetPageLocationUrl(JsContextPtr context,
                                      std::string16 *location_url) {
  assert(location_url);

  // Retrieve window.location.href.
  NPObject* window;
  if (NPN_GetValue(context, NPNVWindowNPObject, &window) != NPERR_NO_ERROR)
    return false;
  ScopedNPObject window_scoped(window);

  NPIdentifier location_id = NPN_GetStringIdentifier("location");
  ScopedNPVariant np_location;
  if (!NPN_GetProperty(context, window, location_id, &np_location) ||
      !NPVARIANT_IS_OBJECT(np_location))
    return false;

  NPIdentifier href_id = NPN_GetStringIdentifier("href");
  ScopedNPVariant np_href;
  if (!NPN_GetProperty(context, NPVARIANT_TO_OBJECT(np_location),
                       href_id, &np_href) ||
      !NPVARIANT_IS_STRING(np_href)) {
    return false;
  }

  NPString np_str = NPVARIANT_TO_STRING(np_href);
  if (np_str.UTF8Length == 0)
    return false;

  return (UTF8ToString16(GetNPStringUTF8Characters(np_str),
                         GetNPStringUTF8Length(np_str),
                         location_url));
}

bool BrowserUtils::GetUserAgentString(std::string16 *user_agent) {
  if (g_user_agent.empty()) {
    return false;  // Not initialized yet.
  }
  *user_agent = g_user_agent;
  return true;
}

bool BrowserUtils::GetPageSecurityOrigin(JsContextPtr context,
                                         SecurityOrigin *security_origin) {
  std::string16 location;
  if (!GetPageLocationUrl(context, &location))
    return false;
  return security_origin->InitFromUrl(location.c_str());
}

bool BrowserUtils::GetPageBrowsingContext(
   JsContextPtr context, scoped_refptr<BrowsingContext> *browsing_context) {
  CPBrowsingContext cp_context = CP::GetBrowsingContext(context);
  browsing_context->reset(new CRBrowsingContext(cp_context));
  return true;
}

bool BrowserUtils::IsOnline() {
  // TODO(mpcomplete): implement me.
  return true;
}
