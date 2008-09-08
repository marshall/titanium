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

#ifndef GEARS_UI_COMMON_HTML_DIALOG_H__
#define GEARS_UI_COMMON_HTML_DIALOG_H__

#include "gears/base/common/string16.h"
#include "third_party/jsoncpp/json.h"

// Return values for showing dialogs. If the dialog returns SUPRESSED, it means
// that the SupressDialog preference in PermissionsDB is enabled for automated
// testing.
enum HtmlDialogReturnValue {
  HTML_DIALOG_SUCCESS,
  HTML_DIALOG_FAILURE,
  HTML_DIALOG_SUPPRESSED
};

// This class implements a cross-platform modal dialog using HTML for the
// dialog UI. We use HTML because it's easy to create nice-looking UIs that
// work across platforms.
//
// Arguments and results are passed to JavaScript inside the dialog by way of
// JSON because that is easier and more flexible than mucking about with
// cross-platform VARIANT types.
//
// TODO(aa): The Json arguments and result fields want UTF-8 strings, but we
// use UTF-16 everywhere else in Gears. What can we do about this? Change
// jsoncpp? Or wrap it?
class HtmlDialog {
 public:
  typedef void (* ModelessCompletionCallback)(Json::Value *result, 
                                              void *closure);
 public:
  // Constructor.
  HtmlDialog()
      : arguments(Json::objectValue), result(Json::nullValue),
        platform_data_(NULL) {}
  HtmlDialog(void *platform_data)
      : arguments(Json::objectValue), result(Json::nullValue),
        platform_data_(platform_data) {}

  // Open the dialog.
  HtmlDialogReturnValue DoModal(const char16 *html_filename, int width,
                                int height);
  
  // Open the dialog and call the callback when it's closed.
  HtmlDialogReturnValue DoModeless(const char16 *html_filename, int width,
                                   int height,
                                   ModelessCompletionCallback callback,
                                   void *closure);

  // Parses the result from the dialog and assings to result_.
  bool SetResult(const char16 *result_string);

  // The arguments object that will be serialized to pass to the dialog.
  Json::Value arguments;

  // The results that were returned from the dialog.
  Json::Value result;

 private:
  // Platform-specific implementation of DoModal().
  bool DoModalImpl(const char16 *html_filename, int width, int height,
                   const char16 *arguments_string);

  bool DoModelessImpl(const char16 *html_filename, int width, int height,
                      const char16 *arguments_string,
                      ModelessCompletionCallback callback,
                      void *closure);

  // GetLocale() is in the browser-specific HtmlDialog code.
  bool GetLocale(std::string16 *locale);

  // Platform-specific data.
  void *platform_data_;
};

#endif  // GEARS_UI_COMMON_HTML_DIALOG_H__
