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
  // The Drag-and-Drop API has not been finalized for official builds.
#else
#include "gears/desktop/drag_and_drop_registry.h"


#if BROWSER_FF
#include <gecko_sdk/include/nsIDOMHTMLElement.h>
#include <gecko_sdk/include/nsIDOMEvent.h>
#include <gecko_sdk/include/nsIDOMEventListener.h>
#include <gecko_sdk/include/nsIDOMEventTarget.h>
#include "gears/desktop/drop_target_ff.h"

#elif BROWSER_IE && !defined(WINCE)
#include <windows.h>
#include "gears/desktop/drop_target_ie.h"

#endif

#include "gears/base/common/js_dom_element.h"
#include "third_party/scoped_ptr/scoped_ptr.h"


#if BROWSER_FF || (BROWSER_IE && !defined(WINCE))
static bool InitializeCallback(const std::string16 name,
                               JsObject &js_callbacks,
                               scoped_ptr<JsRootedCallback> *scoped_callback,
                               std::string16 *error_out) {
  JsParamType property_type = js_callbacks.GetPropertyType(name);
  if (property_type != JSPARAM_UNDEFINED &&
      property_type != JSPARAM_FUNCTION) {
    *error_out = STRING16(L"options.");
    *error_out += name;
    *error_out += STRING16(L" should be a function.");
    return false;
  }
  JsRootedCallback *callback;
  if (js_callbacks.GetPropertyAsFunction(name, &callback)) {
    scoped_callback->reset(callback);
  }
  return true;
}


static bool InitializeDropTarget(ModuleImplBaseClass *sibling_module,
                                 JsObject &js_callbacks,
                                 DropTarget *drop_target,
                                 std::string16 *error_out) {
  sibling_module->GetModuleEnvironment(&drop_target->module_environment_);
  drop_target->unload_monitor_.reset(
      new JsEventMonitor(sibling_module->GetJsRunner(),
                         JSEVENT_UNLOAD,
                         drop_target));
  return InitializeCallback(STRING16(L"ondragenter"), js_callbacks,
                            &drop_target->on_drag_enter_, error_out) &&
         InitializeCallback(STRING16(L"ondragover"), js_callbacks,
                            &drop_target->on_drag_over_, error_out) &&
         InitializeCallback(STRING16(L"ondragleave"), js_callbacks,
                            &drop_target->on_drag_leave_, error_out) &&
         InitializeCallback(STRING16(L"ondrop"), js_callbacks,
                            &drop_target->on_drop_, error_out);
}
#endif


DropTarget *DragAndDropRegistry::RegisterDropTarget(
    ModuleImplBaseClass *sibling_module,
    JsDomElement &dom_element,
    JsObject &js_callbacks,
    std::string16 *error_out) {
#if BROWSER_FF
  nsCOMPtr<nsIDOMEventTarget> event_target =
      do_QueryInterface(dom_element.dom_html_element());
  if (!event_target) {
    *error_out = STRING16(L"Argument must be a DOMEventTarget.");
    return NULL;
  }

  DropTarget *drop_target = new DropTarget;
  if (!InitializeDropTarget(
          sibling_module, js_callbacks, drop_target, error_out)) {
    delete drop_target;
    return NULL;
  }

  if (drop_target->on_drag_enter_.get()) {
    event_target->AddEventListener(
        NS_LITERAL_STRING("dragenter"), drop_target, false);
  }
  if (drop_target->on_drag_over_.get()) {
    event_target->AddEventListener(
        NS_LITERAL_STRING("dragover"), drop_target, false);
  }
  if (drop_target->on_drag_leave_.get()) {
    // Note that the HTML5 event name ("dragleave") differs from the Mozilla
    // event name ("dragexit"). The former is the one that the Gears API uses.
    event_target->AddEventListener(
        NS_LITERAL_STRING("dragexit"), drop_target, false);
  }
  if (drop_target->on_drop_.get()) {
    // Note that the HTML5 event name ("drop") differs from the Mozilla
    // event name ("dragdrop"). The former is the one that the Gears API uses.
    event_target->AddEventListener(
        NS_LITERAL_STRING("dragdrop"), drop_target, false);
  }
  return drop_target;

#elif BROWSER_IE && !defined(WINCE)
  CComQIPtr<IHTMLElement2> html_element_2(dom_element.dispatch());
  if (!html_element_2) return NULL;
  CComBSTR behavior_url(L"#Google" PRODUCT_SHORT_NAME L"#DropTarget");

  CComObject<DropTarget> *drop_target;
  if (FAILED(CComObject<DropTarget>::CreateInstance(&drop_target))) {
    return NULL;
  }
  CComPtr<CComObject<DropTarget> >
      reference_adder_drop_target(drop_target);
  if (!InitializeDropTarget(
          sibling_module, js_callbacks, drop_target, error_out)) {
    return NULL;
  }

  // DropTarget overrides IDispatch::Invoke to react to drag and drop events.
  // To intercept those events, we have to attach an IElementBehavior (which
  // also happens to be the same DropTarget object) to the html_element_2, and
  // to do that, we have to introduce that html_element_2 to an
  // IElementBehaviorFactory (which also happens to be the same DropTarget).
  // Conceptually, the IDispatch, IElementBehavior and IElementBehaviorFactory
  // could be separate instances of separate classes, but for convenience, all
  // three roles are played by the one instance of DropTarget.
  CComQIPtr<IElementBehaviorFactory> factory(drop_target);
  if (!factory) return NULL;
  CComVariant factory_as_variant(static_cast<IUnknown*>(factory));
  LONG ignored_cookie = 0;
  if (FAILED(html_element_2->addBehavior(behavior_url,
                                         &factory_as_variant,
                                         &ignored_cookie))) {
    return NULL;
  }
  return drop_target;

#else
  *error_out = STRING16(L"Desktop.registerDropTarget is not implemented.");
  return NULL;
#endif
}


void DragAndDropRegistry::UnregisterDropTarget(DropTarget *drop_target) {
#if BROWSER_FF
  delete drop_target;
#elif BROWSER_IE && !defined(WINCE)
  // On IE, DropTarget is a COM object, which is ref-counted, so it should
  // automatically delete itself when no longer referred to.
  // In the future, if we allow explicit unregistration of a DropTarget, then
  // we might need to AddRef it during RegisterDropTarget, and Unref it here.
#endif
}
#endif  // OFFICIAL_BUILD
