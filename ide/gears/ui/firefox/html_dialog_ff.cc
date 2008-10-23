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

#include <gecko_sdk/include/nsIURI.h>
#include <gecko_sdk/include/nsIIOService.h>
#include <gecko_sdk/include/nsIProperties.h>
#include <gecko_sdk/include/nsISupportsPrimitives.h>
#include <gecko_sdk/include/nsCOMPtr.h>
#include <gecko_internal/nsIDOMWindowInternal.h>
#include <gecko_internal/nsIChromeRegistry.h>

#include "gears/base/common/string16.h"
#include "gears/base/common/string_utils.h"
#include "gears/base/firefox/dom_utils.h"
#include "gears/ui/common/html_dialog.h"


// Helper function used by DoModalImpl(). Finds the active nsIDOMWindow and
// uses it to launch a modal dialog to a given html file, passing it the
// parameters given in the params argument.
static bool LaunchDialog(nsIProperties *params, const char16 *html_filename,
                         int width, int height) {
  // Build the URL to open in the dialog
  std::string16 html_url(STRING16(L"chrome://"
                                  PRODUCT_SHORT_NAME L"/content/"));
  html_url += html_filename;

  // Build the options string which tells Firefox various bits about how we
  // would like the dialog to display.
  std::string16 options(STRING16(
      L"chrome,centerscreen,modal,dialog,titlebar,resizable,"));
  options += STRING16(L"width=");
  options += IntegerToString16(width);
  options += STRING16(L",height=");
  options += IntegerToString16(height);

  // Get the browser window corresponding to the calling JS
  // NOTE: We assume that there is a js window somewhere on the stack.
  JSContext *js_context;
  if (!DOMUtils::GetJsContext(&js_context)) {
    return false;
  }
  nsCOMPtr<nsIDOMWindowInternal> calling_window;
  DOMUtils::GetDOMWindowInternal(js_context, getter_AddRefs(calling_window));
  if (!calling_window) {
    return false;
  }

  // Use Firefox's built-in openDialog() method to actually open the modal
  // dialog. Args are:
  // - The URL to open
  // - An arbitrary name for the window
  // - Window options string
  // - An optional nsISupports instance to pass to the dialog as an argument
  nsCOMPtr<nsIDOMWindow> dialog_window;
  nsresult nr = calling_window->OpenDialog(
                    nsString(html_url.c_str()),
                    NS_LITERAL_STRING("html_dialog"),
                    nsString(options.c_str()),
                    params,
                    getter_AddRefs(dialog_window));
  if (NS_FAILED(nr)) {
    return false;
  }

  return true;
}


bool HtmlDialog::DoModalImpl(const char16 *html_filename, int width, int height,
                             const char16 *arguments_string) {
  // Build the params object to send into the dialog
  nsCOMPtr<nsISupportsString> input_supports(
    do_CreateInstance("@mozilla.org/supports-string;1"));
  nsCOMPtr<nsIProperties> params(
    do_CreateInstance("@mozilla.org/properties;1"));
  if (!input_supports || !params) {
    return false;
  }

  input_supports->SetData(nsString(arguments_string));
  params->Set("dialogArguments", input_supports);

  // Launch the dialog
  if (!LaunchDialog(params, html_filename, width, height)) {
    return false;
  }

  // Retrieve the dialog result
  nsString output_nsstring;
  nsCOMPtr<nsISupportsString> output_supports;
  nsresult nr = params->Get("dialogResult", NS_GET_IID(nsISupportsString),
    getter_AddRefs(output_supports));
  if (NS_SUCCEEDED(nr)) {
    nr = output_supports->GetData(output_nsstring);
    if (NS_FAILED(nr)) {
      return false;
    }
  }

  // Set up the result property
  return SetResult(output_nsstring.BeginReading());
}

bool HtmlDialog::DoModelessImpl(
    const char16 *html_filename, int width, int height,
    const char16 *arguments_string,
    ModelessCompletionCallback callback,
    void *closure) {
  // Unused in FF.
  assert(false);
  return false;
}

bool HtmlDialog::GetLocale(std::string16 *locale) {
  // On Firefox, the UI locale can be different from the system locale, and
  // there is no way to directly query for it.  We deal with this by requesting
  // a locale URL from the chrome registry, then parsing the adjusted result to
  // extract the locale name.
  nsresult nr;
  nsCOMPtr<nsIIOService> io_service =
      do_GetService("@mozilla.org/network/io-service;1", &nr);
  if (NS_SUCCEEDED(nr)) {
    nsCOMPtr<nsIURI> base_locale_uri;
    nr = io_service->NewURI(NS_LITERAL_CSTRING("chrome://gears/locale"), nsnull,
                            nsnull, getter_AddRefs(base_locale_uri));
    if (NS_SUCCEEDED(nr)) {
      nsCOMPtr<nsIChromeRegistry> chrome_registry =
          do_GetService("@mozilla.org/chrome/chrome-registry;1", &nr);
      if (NS_SUCCEEDED(nr)) {
        nsCOMPtr<nsIURI> locale_uri;
        nr = chrome_registry->ConvertChromeURL(base_locale_uri,
                                               getter_AddRefs(locale_uri));
        if (NS_SUCCEEDED(nr)) {
          nsCString locale_string;
          locale_uri->GetPath(locale_string);

          // Once we get the path from the chrome registry, the UI locale will
          // be the last path component.
          const char *locale_path = locale_string.BeginReading();
          const char *locale_end = strrchr(locale_path, '/');
          if (!locale_end) {
            return false;
          }
          const char *locale_start = locale_end;
          while (locale_start > locale_path) {
            --locale_start;
            if (*locale_start == '/') {
              ++locale_start;
              break;
            }
          }
          std::string locale_utf8(locale_start, locale_end);
          return UTF8ToString16(locale_utf8.c_str(), locale);
        }
      }
    }
  }
  return false;
}
