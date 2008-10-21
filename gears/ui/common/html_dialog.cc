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

#include "gears/base/common/common.h"
#include "gears/base/common/permissions_db.h"
#include "gears/base/common/string16.h"
#include "gears/base/common/string_utils.h"
#include "gears/ui/common/html_dialog.h"


HtmlDialogReturnValue HtmlDialog::DoModal(const char16 *html_filename,
                                          int width, int height) {
  PermissionsDB *permissions_db = PermissionsDB::GetDB();
  if (!permissions_db) return HTML_DIALOG_FAILURE;
  if (permissions_db->ShouldSupressDialogs())
    return HTML_DIALOG_SUPPRESSED;

  // The Json library deals only in UTF-8, so we need to convert :(.
  std::string16 locale;
  if (GetLocale(&locale)) {
    std::string locale8;
    if (String16ToUTF8(locale.c_str(), &locale8)) {
      arguments["locale"] = Json::Value(locale8);
    }
  }

  std::string16 input_string;
  if (!UTF8ToString16(arguments.toStyledString().c_str(), &input_string)) {
    return HTML_DIALOG_FAILURE;
  }

  if (DoModalImpl(html_filename, width, height, input_string.c_str())) {
    return HTML_DIALOG_SUCCESS;
  } else {
    return HTML_DIALOG_FAILURE;
  }
}

HtmlDialogReturnValue HtmlDialog::DoModeless(const char16 *html_filename,
    int width, int height, ModelessCompletionCallback callback, void *closure) {
  PermissionsDB *permissions_db = PermissionsDB::GetDB();
  if (!permissions_db) return HTML_DIALOG_FAILURE;
  if (permissions_db->ShouldSupressDialogs())
    return HTML_DIALOG_SUPPRESSED;

  // The Json library deals only in UTF-8, so we need to convert :(.
  std::string16 locale;
  if (GetLocale(&locale)) {
    std::string locale8;
    if (String16ToUTF8(locale.c_str(), &locale8)) {
      arguments["locale"] = Json::Value(locale8);
    }
  }

  std::string16 input_string;
  if (!UTF8ToString16(arguments.toStyledString().c_str(), &input_string)) {
    return HTML_DIALOG_FAILURE;
  }

  if (DoModelessImpl(html_filename, width, height, input_string.c_str(),
                     callback, closure)) {
    return HTML_DIALOG_SUCCESS;
  } else {
    return HTML_DIALOG_FAILURE;
  }
}

bool HtmlDialog::SetResult(const char16 *value) {
  // NULL and empty are OK. They just means that the dialog did not set a
  // result.
  if (value == NULL || (*value) == L'\0') {
    result = Json::Value(Json::nullValue);
    return true;
  }

  std::string result_string;
  if (!String16ToUTF8(value, &result_string)) {
    return false;
  }

  Json::Reader reader;
  if (!reader.parse(result_string, result)) {
    LOG(("Error parsing return value from dialog. Error was: %s", 
         reader.getFormatedErrorMessages().c_str()));
    return false;
  }

  return true;
}
