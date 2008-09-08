// Copyright 2006, Google Inc.
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

struct JSContext; // must declare this before including nsIJSContextStack.h
#include <gecko_sdk/include/nsIURI.h>
#include <gecko_sdk/include/nsIIOService.h>
#include <gecko_sdk/include/nsIDOMWindow.h>
#include <gecko_sdk/include/nsIInterfaceRequestor.h>
#include <gecko_internal/jsapi.h>
#include <gecko_internal/nsIBaseWindow.h>
#include <gecko_internal/nsIDocShell.h>
#include <gecko_internal/nsIDocShellTreeItem.h>
#include <gecko_internal/nsIDocShellTreeOwner.h>
#if BROWSER_FF3
#include <gecko_internal/nsDOMJSUtils.h>
#endif
#include <gecko_internal/nsIDOM3Node.h>
#include <gecko_internal/nsIDOMWindowInternal.h>
#include <gecko_internal/nsIInterfaceInfoManager.h>
#include <gecko_internal/nsIJSContextStack.h>
#include <gecko_internal/nsIScriptContext.h>
#include <gecko_internal/nsIScriptGlobalObject.h>
#include <gecko_internal/nsIScriptSecurityManager.h>
#include <gecko_internal/nsIWebNavigation.h>
#include <gecko_internal/nsIWidget.h>
#include <gecko_internal/nsIXPConnect.h>
#include <gecko_internal/nsIXULWindow.h>
#include "gears/base/common/browsing_context.h"
#include "gears/base/common/common.h"
#include "gears/base/common/security_model.h"
#include "gears/base/firefox/dom_utils.h"

#if defined(LINUX) && !defined(OS_MACOSX)
#include <gtk/gtk.h>
#endif


bool DOMUtils::GetJsContext(JSContext **context) {
  // Get JSContext from stack.
  nsCOMPtr<nsIJSContextStack> stack =
      do_GetService("@mozilla.org/js/xpc/ContextStack;1");
  if (!stack) { return false; }

  JSContext *cx;
  if (NS_FAILED(stack->Peek(&cx)) || !cx) {
    return false;
  }

  *context = cx;  // only modify output param on success
  return true;
}


nsresult DOMUtils::GetDOMWindowInternal(JSContext *context,
                                        nsIDOMWindowInternal **result) {
  nsIScriptContext *script_context = GetScriptContextFromJSContext(context);
  if (script_context) {
    // TODO(miket): We found a case where GetScriptContextFromJSContext() fails
    // if Toolbar is installed with safebrowsing. Safebrowsing wants to grab
    // https://www.google.com/safebrowsing/getkey? on startup, and for some
    // reason there's no JSContext at that point. We (I and Darin) decided that
    // ignoring that problem with this test is probably the equivalent of
    // whatever the real right thing is to do.
    NS_ADDREF(script_context);
  }
  NS_ENSURE_STATE(script_context);

  return CallQueryInterface(script_context->GetGlobalObject(), result);
}

nsresult DOMUtils::GetNativeWindow(JSContext *js_context,
                                   NativeWindowPtr* window) {
  nsIDOMWindowInternal* internal_window = NULL;
  nsresult nr = GetDOMWindowInternal(js_context, &internal_window);
  if (NS_FAILED(nr))
    return nr;

  nsCOMPtr<nsIInterfaceRequestor> requestor(do_QueryInterface(internal_window,
                                                              &nr));
  if (NS_FAILED(nr))
    return nr;

  nsCOMPtr<nsIWebNavigation> nav;
  nr = requestor->GetInterface(NS_GET_IID(nsIWebNavigation),
                               getter_AddRefs(nav));
  if (NS_FAILED(nr))
    return nr;

  nsCOMPtr<nsIDocShellTreeItem> treeItem(do_QueryInterface(nav, &nr));
  if (NS_FAILED(nr))
    return nr;

  nsCOMPtr<nsIDocShellTreeOwner> treeOwner;
  nr = treeItem->GetTreeOwner(getter_AddRefs(treeOwner));
  if (NS_FAILED(nr))
    return nr;

  requestor = do_QueryInterface(treeOwner, &nr);
  if (NS_FAILED(nr))
    return nr;

  nsCOMPtr<nsIXULWindow> xulWindow;
  nr = requestor->GetInterface(NS_GET_IID(nsIXULWindow),
                               getter_AddRefs(xulWindow));
  if (NS_FAILED(nr))
    return nr;

  nsCOMPtr<nsIDocShell> docShell;
  nr = xulWindow->GetDocShell(getter_AddRefs(docShell));
  if (NS_FAILED(nr))
    return nr;

  nsCOMPtr<nsIBaseWindow> baseWindow(do_QueryInterface(docShell, &nr));
  if (NS_FAILED(nr))
    return nr;

  nsCOMPtr<nsIWidget> widget;
  nr = baseWindow->GetParentWidget(getter_AddRefs(widget));
  if (!widget || NS_FAILED(nr))
    return nr;

  void* parentWindow = widget->GetNativeData(NS_NATIVE_WINDOW);
  if (!parentWindow)
    return NS_ERROR_FAILURE;

#if defined(OS_MACOSX)
  // TODO(bpm): Fix this from crashing on FF2.
  //*window = GetWindowPtrFromNSWindow(parentWindow);
  return NS_ERROR_FAILURE;
#elif defined(LINUX)
  GdkWindow* parentGdkWindow = reinterpret_cast<GdkWindow*>(parentWindow);
  gpointer user_data = NULL;
  gdk_window_get_user_data(gdk_window_get_toplevel(parentGdkWindow),
                           &user_data);
  if (!user_data)
    return NS_ERROR_FAILURE;
  *window = GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(user_data)));
#else  // !LINUX && !OS_MACOSX
  *window = reinterpret_cast<NativeWindowPtr>(parentWindow);
#endif

  return NS_OK;
}


bool DOMUtils::NewResolvedURI(const char16 *base_url,
                              const char16 *url,
                              nsIURI **url_obj) {
  assert(base_url);
  assert(url);
  assert(url_obj);

  nsCOMPtr<nsIIOService> ios =
      do_GetService("@mozilla.org/network/io-service;1");
  if (!ios) { return false; }

  nsCOMPtr<nsIURI> base_url_obj;
  nsresult nr = ios->NewURI(NS_ConvertUTF16toUTF8(base_url),
                            nsnull, nsnull, getter_AddRefs(base_url_obj));
  if (NS_FAILED(nr)) { return false; }

  nr = ios->NewURI(NS_ConvertUTF16toUTF8(url),
                   nsnull, base_url_obj, url_obj);
  if (NS_FAILED(nr)) { return false; }
  return true;
}


bool DOMUtils::NewAbsoluteURI(const char16 *url, nsIURI **url_obj) {
  assert(url);
  assert(url_obj);

  nsCOMPtr<nsIIOService> ios =
      do_GetService("@mozilla.org/network/io-service;1");
  if (!ios) { return false; }

  nsresult nr = ios->NewURI(NS_ConvertUTF16toUTF8(url),
                            nsnull, nsnull, url_obj);
  if (NS_FAILED(nr)) { return false; }
  return true;
}


bool DOMUtils::GetPageLocation(std::string16 *location_url) {
  assert(location_url);
  nsresult nr;

  // get a nsIURI for the current page
  nsCOMPtr<nsIScriptSecurityManager> sec_man =
      do_GetService(NS_SCRIPTSECURITYMANAGER_CONTRACTID, &nr);
  if (NS_FAILED(nr) || !sec_man) { return false; }

  nsCOMPtr<nsIPrincipal> ppl;
  nr = sec_man->GetSubjectPrincipal(getter_AddRefs(ppl));
  if (NS_FAILED(nr) || !ppl) { return false; }

  nsCOMPtr<nsIURI> url;
  nr = ppl->GetURI(getter_AddRefs(url));
  if (NS_FAILED(nr) || !url) { return false; }

  // get the page location url
  nsCString out_utf8;
  nr = url->GetSpec(out_utf8);
  if (NS_FAILED(nr)) { return false; }

  location_url->assign(NS_ConvertUTF8toUTF16(out_utf8).get());

  return true;  // succeeded
}


bool DOMUtils::GetPageOrigin(SecurityOrigin *security_origin) {
  std::string16 location;
  if (!GetPageLocation(&location))
    return false;
  return security_origin->InitFromUrl(location.c_str());
}

bool DOMUtils::GetPageBrowsingContext(
    scoped_refptr<BrowsingContext> *browsing_context) {
  browsing_context->reset();
  return true;
}

bool DOMUtils::IsOnline() {
  nsCOMPtr<nsIIOService> ios = do_GetService(
                                    "@mozilla.org/network/io-service;1");
  NS_ENSURE_TRUE(ios, false);
  PRBool offline = PR_FALSE;
  ios->GetOffline(&offline);
  return offline ? false : true;
}
