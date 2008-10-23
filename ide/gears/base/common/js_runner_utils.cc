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

#include "gears/base/common/js_runner_utils.h"
#include "gears/base/common/js_runner.h"
#include "gears/base/common/string_utils.h"

std::string16 EscapeMessage(const std::string16 &message) {
  std::string16 escaped_message(message);
  // This replacement must preceed the others.
  ReplaceAll(escaped_message, std::string16(STRING16(L"\\")),
            std::string16(STRING16(L"\\\\")));
  ReplaceAll(escaped_message, std::string16(STRING16(L"'")),
            std::string16(STRING16(L"\\'")));
  ReplaceAll(escaped_message, std::string16(STRING16(L"\r")),
            std::string16(STRING16(L"\\r")));
  ReplaceAll(escaped_message, std::string16(STRING16(L"\n")),
            std::string16(STRING16(L"\\n")));
  return escaped_message;
}

void ThrowGlobalErrorImpl(JsRunnerInterface *js_runner,
                          const std::string16 &message) {
#if BROWSER_FF3
  JS_BeginRequest(js_runner->GetContext());
  JS_SetPendingException(
      js_runner->GetContext(),
      STRING_TO_JSVAL(JS_NewUCStringCopyZ(
          js_runner->GetContext(),
          reinterpret_cast<const jschar *>(message.c_str()))));
  JS_ReportPendingException(js_runner->GetContext());
  JS_EndRequest(js_runner->GetContext());
#elif BROWSER_NPAPI
  std::string16 string_to_eval =
      std::string16(STRING16(L"window.onerror('")) +
      EscapeMessage(message) +
      std::string16(STRING16(L"')"));
  js_runner->Eval(string_to_eval);
#else
  std::string16 string_to_eval =
      std::string16(STRING16(L"throw new Error('")) +
      EscapeMessage(message) +
      std::string16(STRING16(L"')"));
  js_runner->Eval(string_to_eval);
#endif
}
