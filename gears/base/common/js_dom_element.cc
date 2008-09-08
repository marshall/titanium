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

#if BROWSER_FF
struct JSContext; // must declare this before including nsIJSContextStack.h
#include <gecko_sdk/include/nsCOMPtr.h>
#include <gecko_sdk/include/nsIDOMHTMLInputElement.h>
#include <gecko_sdk/include/nsIFile.h>
#include <gecko_internal/nsIJSContextStack.h>
#include <gecko_internal/nsIXPConnect.h>
#elif BROWSER_IE
#ifdef WINCE
#include <webvw.h>  // For IPIEHTMLInputTextElement
#endif
#endif

#include "gears/base/common/js_dom_element.h"

#include "gears/base/common/leak_counter.h"

#if BROWSER_FF
#include "gears/base/firefox/ns_file_utils.h"
#endif


JsDomElement::JsDomElement()
    : is_initialized_(false) {
  LEAK_COUNTER_INCREMENT(JsDomElement);
}


JsDomElement::~JsDomElement() {
  LEAK_COUNTER_DECREMENT(JsDomElement);
}


#if BROWSER_FF


// The IIDs for nsIContent in different versions of Firefox/Gecko.
// TODO(michaeln): Add to this list as new versions show up.

#if BROWSER_FF3
// Firefox 3.0.x
#define NS_ICONTENT_IID_GECKO190 \
{ 0x0acd0482, 0x09a2, 0x42fd, \
  { 0xb6, 0x1b, 0x95, 0xa2, 0x01, 0x6a, 0x55, 0xd3 } }
#else
// Firefox 1.5.0.x
#define NS_ICONTENT_IID_GECKO180 \
{ 0x3fecc374, 0x2839, 0x4db3, \
  { 0x8d, 0xe8, 0x6b, 0x76, 0xd1, 0xd8, 0xe6, 0xf6 } }
// Firefox 2.0.0.x
#define NS_ICONTENT_IID_GECKO181 \
{ 0x9d059608, 0xddb0, 0x4e6a, \
  { 0x99, 0x69, 0xd2, 0xf3, 0x63, 0xa1, 0xb5, 0x57 } }
#endif  // BROWSER_FF3

static const nsIID kPossibleNsContentIIDs[] = {
#if BROWSER_FF3
  NS_ICONTENT_IID_GECKO190,
#else
  NS_ICONTENT_IID_GECKO180,
  NS_ICONTENT_IID_GECKO181,
#endif  // BROWSER_FF3
};


#if BROWSER_FF2
static PRBool StringBeginsWith(const nsAString &source,
                               const nsAString &substring) {
  nsAString::size_type src_len = source.Length();
  nsAString::size_type sub_len = substring.Length();
  if (sub_len > src_len)
    return PR_FALSE;
  return Substring(source, 0, sub_len).Equals(substring);
}
#endif


// This is a security measure to prevent script from spoofing DOM elements.
static bool VerifyNsContent(nsISupports *unknown) {
  if (!unknown) return false;

  nsresult rv = NS_OK;
  nsCOMPtr<nsIInterfaceInfoManager> iface_info_manager;
  iface_info_manager = do_GetService(NS_INTERFACEINFOMANAGER_SERVICE_CONTRACTID,
                                     &rv);
  if (NS_FAILED(rv) || !iface_info_manager) { return false; }

  // The nsIContentIID is version dependent, we test for all we know about
  for (size_t i = 0; i < ARRAYSIZE(kPossibleNsContentIIDs); ++i) {
    const nsIID *ns_content_iid = &kPossibleNsContentIIDs[i];

    // Paranoia, ensure that the IID we query for is either unknown to the
    // interface manager or not scriptable. The XPConnect JSWrapper
    // QueryInterface implementation will not forward to script for such
    // interfaces. In Firefox 2.0 and 1.5, nsIContent is not known by
    // the interface manager.
    nsCOMPtr<nsIInterfaceInfo> iface_info;
    rv = iface_info_manager->GetInfoForIID(ns_content_iid,
                                           getter_AddRefs(iface_info));
    if (NS_SUCCEEDED(rv) && iface_info) {
      PRBool is_scriptable = PR_TRUE;
      rv = iface_info->IsScriptable(&is_scriptable);
      if (NS_FAILED(rv) || is_scriptable) {
        continue;  // Don't test for this interface id
      }
    }

    // Test if our 'unknown' argument implements nsIContent,
    // a positive test indicates 'unknown' is not script based.
    nsCOMPtr<nsISupports> nscontent;
    rv = unknown->QueryInterface(*ns_content_iid,
                                 reinterpret_cast<void**>(&nscontent));
    if (NS_SUCCEEDED(rv) && nscontent) {
      return true;
    }
  }

  return false;
}


bool JsDomElement::InitJsDomElement(JsContextPtr context, JsToken token) {
  assert(!is_initialized_);
  assert(!dom_html_element_);

  if (!JSVAL_IS_OBJECT(token)) { return false; }
  JSObject *obj = JSVAL_TO_OBJECT(token);

  nsresult nr;
  nsCOMPtr<nsIXPConnect> xpc;
  xpc = do_GetService("@mozilla.org/js/xpc/XPConnect;1", &nr);
  if (NS_FAILED(nr)) { return false; }

  nsCOMPtr<nsIXPConnectWrappedNative> wrapper;
  nr = xpc->GetWrappedNativeOfJSObject(context, obj, getter_AddRefs(wrapper));
  if (NS_FAILED(nr)) { return false; }

  nsCOMPtr<nsISupports> supports;
  nr = wrapper->GetNative(getter_AddRefs(supports));
  if (NS_FAILED(nr)) { return false; }

  // Verify unknown is a DOM HTML element
  if (!VerifyNsContent(supports)) { return false; }
  dom_html_element_ = do_QueryInterface(supports);
  if (dom_html_element_.get() == NULL) { return false; }

  is_initialized_ = true;
  return true;
}



// Security rights are gleaned from the JavaScript context.
// "UniversalFileRead" is required to get/set the file input value.
// Temporarily pushing NULL onto the JavaScript context stack lets
// the system know that native code is running and all rights are granted.
class ScopedUniversalReadRights {
 public:
  ~ScopedUniversalReadRights() {
    ReleaseRights();
  }
  bool GetRights() {
    assert(!stack_);
    stack_ = do_GetService("@mozilla.org/js/xpc/ContextStack;1");
    if (!stack_) return false;
    stack_->Push(NULL);
    return true;
  }
  void ReleaseRights() {
    if (stack_) {
      JSContext *not_used;
      stack_->Pop(&not_used);
      stack_ = NULL;
    }
  }
 private:
  nsCOMPtr<nsIJSContextStack> stack_;
};


bool JsDomElement::GetFileInputElementValue(
    std::string16 *file_name_out) {
  assert(is_initialized_);
  nsCOMPtr<nsIDOMHTMLInputElement> input = do_QueryInterface(dom_html_element_);
  if (!input) {
    return false;
  }

  nsString filepath;
  {
    ScopedUniversalReadRights rights;
    if (!rights.GetRights() ||
        input->GetValue(filepath) != NS_OK) {
      return false;
    }
  }

  // If its really a file url, handle it differently. Gecko handles file
  // input elements in this way when submitting forms.
  if (StringBeginsWith(filepath, NS_LITERAL_STRING("file:"))) {
    nsCOMPtr<nsIFile> file;
    // Converts the URL string into the corresponding nsIFile if possible.
    NSFileUtils::GetFileFromURLSpec(filepath, getter_AddRefs(file));
    if (!file || NS_FAILED(file->GetPath(filepath))) {
      return false;
    }
  }
  
  *file_name_out = filepath.get();
  return true;
}


bool JsDomElement::SetFileInputElementValue(std::string16 &file_name) {
  assert(is_initialized_);
  nsCOMPtr<nsIDOMHTMLInputElement> input = do_QueryInterface(dom_html_element_);
  if (!input) {
    return false;
  }

  ScopedUniversalReadRights rights;
  if (!rights.GetRights() ||
      input->SetValue(nsString(file_name.c_str())) != NS_OK) {
    return false;
  }
  return true;
}


#elif BROWSER_IE


bool JsDomElement::InitJsDomElement(JsContextPtr context, JsToken token) {
  assert(!is_initialized_);
  assert(!dispatch_);

  if (token.vt != VT_DISPATCH) { return false; }
  IDispatch *dispatch = token.pdispVal;

#ifdef WINCE
  CComQIPtr<IPIEHTMLElement> html_element(dispatch);
#else
  CComQIPtr<IHTMLElement> html_element(dispatch);
#endif
  if (!html_element) { return false; }

  dispatch_ = dispatch;
  is_initialized_ = true;
  return true;
}


bool JsDomElement::GetFileInputElementValue(
    std::string16 *file_name_out) {
  assert(is_initialized_);
#ifdef WINCE
  // If it implements the IPIEHTMLInputTextElement interface, and has type
  // 'file', then accept it.
  CComQIPtr<IPIEHTMLInputTextElement> input(dispatch_);
  CComBSTR type;
  if (input && (FAILED(input->get_type(&type)) || type != L"file")) {
    input.Release();
  }
#else
  // If it implements the IHTMLInputFileElemement interface, then accept it.
  CComQIPtr<IHTMLInputFileElement> input(dispatch_);
#endif
  if (!input) {
    return false;
  }
  CComBSTR filepath;
  if (FAILED(input->get_value(&filepath))) {
    return false;
  }
  if (filepath.m_str) {
    file_name_out->assign(filepath);
  } else {
    file_name_out->clear();
  }
  return true;
}


bool JsDomElement::SetFileInputElementValue(std::string16 &file_name) {
  assert(is_initialized_);
  // On IE, we don't seem to be able to set the value of a file input element,
  // presumably because it clashes with IE's security model. Consequently,
  // any Gears code, such as LocalServer's FileSubmitter, that would normally
  // call this method also needs to implement a work-around on IE.
  return false;
}


#elif BROWSER_NPAPI


bool JsDomElement::InitJsDomElement(JsContextPtr context, JsToken token) {
  // TODO(nigeltao): implement on NPAPI.
  assert(!is_initialized_);
  return false;
}


bool JsDomElement::GetFileInputElementValue(
    std::string16 *file_name_out) {
  // TODO(nigeltao): implement on NPAPI.
  assert(is_initialized_);
  return false;
}


bool JsDomElement::SetFileInputElementValue(std::string16 &file_name) {
  // TODO(nigeltao): implement on NPAPI.
  assert(is_initialized_);
  return false;
}


#endif
