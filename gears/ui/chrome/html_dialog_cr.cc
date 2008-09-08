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

#include "gears/ui/chrome/html_dialog_cr.h"

#include "gears/base/chrome/browsing_context_cr.h"
#include "gears/base/chrome/module_cr.h"
#include "gears/base/npapi/browser_utils.h"

static bool GetDialogUrlUTF8(const char16 *html_filename,
                             std::string *url_utf8) {
  if (!String16ToUTF8(html_filename, url_utf8))
    return false;
  *url_utf8 = "gears://resources/" + *url_utf8;
  return true;
}

bool HtmlDialog::DoModalImpl(const char16 *html_filename, int width, int height,
                             const char16 *arguments_string) {
  // We only support modal dialogs in the plugin process.
  assert(CP::IsPluginProcess());

  std::string url_utf8, arguments_utf8;
  if (!GetDialogUrlUTF8(html_filename, &url_utf8))
    return false;
  if (!String16ToUTF8(arguments_string, &arguments_utf8))
    return false;

  char *result = NULL;
  std::string16 result16;
  JsCallContext *context = BrowserUtils::GetCurrentJsCallContext();
  bool rv = g_cpbrowser_funcs.show_html_dialog_modal(
      g_cpid, CP::GetBrowsingContext(context->js_context()), url_utf8.c_str(),
      width, height, arguments_utf8.c_str(), &result) == CPERR_SUCCESS;

  if (rv && result) {
    rv = UTF8ToString16(result, &result16);
    g_cpbrowser_funcs.free(result);
  }

  return rv && SetResult(result16.c_str());
}

bool HtmlDialog::DoModelessImpl(
    const char16 *html_filename, int width, int height,
    const char16 *arguments_string,
    ModelessCompletionCallback callback,
    void *closure) {
  // We only support modeless dialogs in the browser process right now.
  assert(CP::IsBrowserProcess());

  std::string url_utf8, arguments_utf8;
  if (!GetDialogUrlUTF8(html_filename, &url_utf8))
    return false;
  if (!String16ToUTF8(arguments_string, &arguments_utf8))
    return false;

  scoped_refptr<CRBrowsingContext> browsing_context(
      static_cast<CRBrowsingContext*>(platform_data_));
  platform_data_ = NULL;  // browsing_context may be deleted at end of scope

  // Note: we pass a callback object as the plugin_context.  When the dialog
  // closes, we get a notification with a pointer to that plugin_context, which
  // we use to handle the results.
  void *plugin_context = new HtmlDialogCallback(this, callback, closure);
  bool rv = g_cpbrowser_funcs.show_html_dialog(
      g_cpid, browsing_context->context, url_utf8.c_str(), width, height,
      arguments_utf8.c_str(), plugin_context) == CPERR_SUCCESS;
  return rv;
}

bool HtmlDialog::GetLocale(std::string16 *locale) {
  *locale = CP::locale();
  return true;
}

void HtmlDialogCallback::DialogClosed(const char16 *result_string) {
  dialog_->SetResult(result_string);
  if (callback_)
    callback_(&dialog_->result, closure_);
  delete this;
}
