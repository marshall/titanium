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

#ifndef GEARS_BASE_COMMON_JS_DOM_ELEMENT_H__
#define GEARS_BASE_COMMON_JS_DOM_ELEMENT_H__

#if BROWSER_FF
#include <gecko_sdk/include/nsCOMPtr.h>
#include <gecko_sdk/include/nsIDOMHTMLElement.h>
#elif BROWSER_IE
#include <windows.h>
#endif

#include "gears/base/common/basictypes.h"
#include "gears/base/common/js_types.h"
#include "gears/base/common/string16.h"

// NOTE: A JsDomElement should never be created, or otherwise manipulated,
// from a worker thread.
class JsDomElement {
 public:
  JsDomElement();
  ~JsDomElement();

  bool InitJsDomElement(JsContextPtr context, JsToken token);

  bool GetFileInputElementValue(std::string16 *file_name_out);
  bool SetFileInputElementValue(std::string16 &file_name);

#if BROWSER_FF
  nsIDOMHTMLElement *dom_html_element() {
    assert(is_initialized_);
    return dom_html_element_;
  }
#elif BROWSER_IE
  IDispatch *dispatch() {
    assert(is_initialized_);
    return dispatch_;
  }
#elif BROWSER_NPAPI
#endif

 private:
#if BROWSER_FF
  nsCOMPtr<nsIDOMHTMLElement> dom_html_element_;
#elif BROWSER_IE
  CComPtr<IDispatch> dispatch_;
#elif BROWSER_NPAPI
#endif

  bool is_initialized_;

  DISALLOW_EVIL_CONSTRUCTORS(JsDomElement);
};

#endif  // GEARS_BASE_COMMON_JS_DOM_ELEMENT_H__
