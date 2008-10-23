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

#include "gears/base/common/process_utils_win32.h"
#include "gears/base/common/string16.h"
#include "gears/base/common/string_utils.h"
#include "gears/ui/common/html_dialog.h"
#ifdef WINCE
#include "gears/ui/ie/html_dialog_host_iemobile.h"
#else
#include "gears/ui/ie/html_dialog_host.h"
#endif
#include "gears/ui/ie/string_table.h"

bool HtmlDialog::DoModalImpl(const char16 *html_filename, int width, int height,
                             const char16 *arguments_string) {
  CComObject<HtmlDialogHost> *dialog = new CComObject<HtmlDialogHost>();
  CComPtr<HtmlDialogHost> ref(dialog);

  CComBSTR result_bstr;
  if (!dialog->ShowDialog(html_filename, CSize(width, height),
                          CComBSTR(arguments_string), &result_bstr)) {
      return false;
  }

  return SetResult(result_bstr.m_str);
}

bool HtmlDialog::DoModelessImpl(
    const char16 *html_filename, int width, int height,
    const char16 *arguments_string,
    ModelessCompletionCallback callback,
    void *closure) {
  // Unused in IE.
  assert(false);
  return false;
}

bool HtmlDialog::GetLocale(std::string16 *locale) {
  char16 locale_string[MAX_PATH];
  if (LoadString(GetGearsModuleHandle(), IDS_LOCALE,
                 locale_string, MAX_PATH)) {
    *locale = locale_string;
    return true;
  }
  return false;
}
