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

#ifdef WINCE
  // FileSubmitter is not implemented for WinCE.
#else

#if BROWSER_IE
#include <windows.h>
#endif

#include "gears/localserver/file_submitter.h"

#include "gears/base/common/file.h"
#include "gears/base/common/js_dom_element.h"
#include "gears/base/common/js_runner.h"
#include "gears/base/common/module_wrapper.h"
#include "gears/base/common/paths.h"
#include "gears/base/common/url_utils.h"

#if BROWSER_FF
#include "gears/base/firefox/dom_utils.h"
#elif BROWSER_IE
#include "gears/base/ie/activex_utils.h"
#include "gears/localserver/ie/file_submit_behavior.h"
#endif

static const char16 *kMissingFilename = STRING16(L"Unknown");



#if BROWSER_IE
HRESULT FileSubmitterBehaviorFactory::FindBehavior(
    BSTR name,
    BSTR url,
    IElementBehaviorSite* behavior_site,
    IElementBehavior** behavior_out) {
  // We only know how to create one kind of behavior
  if (!name || (_wcsicmp(name, L"SubmitFile") != 0))
    return E_FAIL;
  CComQIPtr<IElementBehaviorSiteOM> site_om(behavior_site);
  if (!site_om) return E_FAIL;
  if (FAILED(site_om->RegisterUrn(url))) return E_FAIL;
  CComObject<SubmitFileBehavior>* behavior = NULL;
  if (FAILED(CComObject<SubmitFileBehavior>::CreateInstance(&behavior)))
    return E_FAIL;
  CComPtr<CComObject<SubmitFileBehavior> > reference_adder(behavior);
  behavior->InitFromBehaviorFactory(filename_);
  return behavior->QueryInterface(behavior_out);
}
#endif  // BROWSER_IE



// This object cleans up any temporary files that were created. This is done
// on page unload, rather than on destruction of the GearsFileSubmitter
// object, because that GearsFileSubmitter might be garbage collected before
// the files are needed.
class TempFileJanitor : public JsEventHandlerInterface {
 public:
  TempFileJanitor(const std::string16 &directory_name)
      : directory_name_(directory_name) {}

  virtual void HandleEvent(JsEventType event_type) {
    assert(event_type == JSEVENT_UNLOAD);
    if (!directory_name_.empty()) {
      File::DeleteRecursively(directory_name_.c_str());
    }
    delete this;
  }

  scoped_ptr<JsEventMonitor> unload_monitor_;
  const std::string16 directory_name_;
};



DECLARE_GEARS_WRAPPER(GearsFileSubmitter);

// static
template<>
void Dispatcher<GearsFileSubmitter>::Init() {
  RegisterMethod("setFileInputElement",
                 &GearsFileSubmitter::SetFileInputElement);
}

void GearsFileSubmitter::SetFileInputElement(JsCallContext *context) {
  if (EnvIsWorker()) {
    context->SetException(
        STRING16(L"setFileInputElement is not supported in workers."));
    return;
  }

  JsDomElement dom_element;
  std::string16 url;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_DOM_ELEMENT, &dom_element },
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &url }
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set()) {
    return;
  }

  if (url.empty()) {
    return;
  }
  std::string16 full_url;
  if (!ResolveAndNormalize(EnvPageLocationUrl().c_str(), url.c_str(),
                           &full_url)) {
    context->SetException(STRING16(L"Failed to resolve URL."));
    return;
  }
  if (!EnvPageSecurityOrigin().IsSameOriginAsUrl(full_url.c_str())) {
    context->SetException(STRING16(L"URL is not from the same origin."));
    return;
  }

  // Read data from our local store
  ResourceStore::Item item;
  if (!store_.GetItem(full_url.c_str(), &item)) {
    context->SetException(STRING16(L"Failed to ResourceStore::GetItem."));
    return;
  }
  std::string16 captured_filename;
  item.payload.GetHeader(HttpConstants::kXCapturedFilenameHeader,
                         &captured_filename);
  if (captured_filename.empty()) {
    // This handles the case where the URL didn't get into the database
    // using the captureFile API. It's arguable whether we should support this.
    captured_filename = kMissingFilename;
  }

  if (!CreateTempFile(captured_filename, item.payload)) {
    context->SetException(STRING16(L"Could not create temporary file."));
    return;
  }
  if (!CaptureInputElement(dom_element)) {
    context->SetException(STRING16(L"Could not CaptureInputElement."));
    return;
  }
}

bool GearsFileSubmitter::CreateTempFile(
    const std::string16 &in_filename,
    const WebCacheDB::PayloadInfo &payload) {
  // The in_filename should not contain a path separator (and hence should
  // not contain something like "../../../etc/passwd"). For example, a
  // std::string16 generated by File::GetBaseName should always satisfy this
  // pre-condition.
  assert(std::string16::npos == in_filename.find(kPathSeparator));

  std::string16 name_of_temporary_directory;
  if (!File::CreateNewTempDirectory(&name_of_temporary_directory)) {
    return false;
  }
  // This newly allocated TempFileJanitor deletes itself, on page unload.
  TempFileJanitor *temp_file_janitor =
      new TempFileJanitor(name_of_temporary_directory);
  temp_file_janitor->unload_monitor_.reset(
      new JsEventMonitor(GetJsRunner(), JSEVENT_UNLOAD, temp_file_janitor));

  name_of_temporary_file_ =
      name_of_temporary_directory + kPathSeparator + in_filename;

  if (!File::CreateNewFile(name_of_temporary_file_.c_str()) ||
      !File::WriteVectorToFile(name_of_temporary_file_.c_str(),
                               payload.data.get())) {
    return false;
  }
  return true;
}

bool GearsFileSubmitter::CaptureInputElement(JsDomElement &dom_element) {
  // JsDomElement::SetFileInputElementValue is unimplemented on IE, so we have
  // to implement a BROWSER_IE specific work-around.
#if BROWSER_IE
  if (behavior_cookie_) {
    // If the browser_cookie_ is set, then setFileInputElement has been
    // previously called (and succeeded). We should undo the attachment
    // of the previously created Behavior to the previously specified
    // IHTMLElement2.
    VARIANT_BOOL remove_successful;
    html_element2_->removeBehavior(behavior_cookie_, &remove_successful);
    if (remove_successful != VARIANT_TRUE) {
      return false;
    }
    behavior_cookie_ = 0;
    html_element2_ = static_cast<IHTMLElement2*>(NULL);
  }

  html_element2_ = dom_element.dispatch();
  if (!html_element2_) return false;
  CComBSTR behavior_url(L"#Google" PRODUCT_SHORT_NAME L"#SubmitFile");

  // Check if this behavior has already been attached to the element.
  CComPtr<IDispatch> dispatch;
  if (FAILED(html_element2_->get_behaviorUrns(&dispatch))) return false;
  CComQIPtr<IHTMLUrnCollection> urn_collection(dispatch);
  if (!urn_collection) return false;
  LONG num_urns;
  if (FAILED(urn_collection->get_length(&num_urns))) return false;
  for (LONG i = 0; i < num_urns; i++) {
    CComBSTR urn;
    if (FAILED(urn_collection->item(i, &urn))) return false;
    if (urn && (0 == ::StrCmpIW(urn, behavior_url))) {
      // That behavior already exists on this element, presumably attached
      // by a different GearsFileSubmitter. We don't know how to remove that
      // behavior from here, so return false to throw a JavaScript exception.
      return false;
    }
  }

  // TODO(michaeln): We can't attach our behavior to <input> elements, as we
  // would like, due to limitations in IE. For now the DHTML developer has 
  // to use GearsFileSubmitter with some element other than an <input> element
  // that is nested in a <form> element for IE.  The TODO is to allow developers
  // to pass in <input type=file> elements, and to dynamically create a new
  // element of some type with our behavior attached to it (or to dynamically
  // re-write the input element to be a different type).
  CComObject<FileSubmitterBehaviorFactory> *behavior_factory;
  if (FAILED(CComObject<FileSubmitterBehaviorFactory>::CreateInstance(
      &behavior_factory))) {
    return false;
  }
  CComPtr<CComObject<FileSubmitterBehaviorFactory> >
      reference_adder(behavior_factory);
  // We delegate most of the remaining work to the SubmitFileBehavior class.
  // Here we pass (via the behavior factory) the name of the temporary file to
  // an instance of the behavior attached to file_input_element.
  behavior_factory->filename_ = name_of_temporary_file_;
  CComQIPtr<IElementBehaviorFactory> factory(behavior_factory);
  if (!factory) return false;
  CComVariant factory_as_variant(static_cast<IUnknown*>(factory));
  if (FAILED(html_element2_->addBehavior(behavior_url,
                                         &factory_as_variant,
                                         &behavior_cookie_))) {
    return false;
  }
  return true;

#else
  return dom_element.SetFileInputElementValue(name_of_temporary_file_);
#endif
}

#endif // WINCE
