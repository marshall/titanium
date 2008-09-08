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

#ifndef GEARS_UI_CHROME_HTML_DIALOG_CR_H__
#define GEARS_UI_CHROME_HTML_DIALOG_CR_H__

#include "gears/ui/common/html_dialog.h"
#include "third_party/chrome/chrome_plugin_api.h"

// This class is used as the 'plugin_context' parameter to Chrome's dialog
// API.  It holds the data we need to handle the closing of a dialog.
class HtmlDialogCallback {
 public:
  // Called with the results when the dialog is closed.  Object is no longer
  // valid after this call.
  void DialogClosed(const char16 *result_string);

 private:
  friend class HtmlDialog;

  HtmlDialogCallback(HtmlDialog *dialog,
                     HtmlDialog::ModelessCompletionCallback callback,
                     void *closure)
      : dialog_(dialog), callback_(callback), closure_(closure) {}
  ~HtmlDialogCallback() {}

  HtmlDialog *dialog_;
  HtmlDialog::ModelessCompletionCallback callback_;
  void *closure_;
};

#endif  // GEARS_UI_CHROME_HTML_DIALOG_CR_H__
