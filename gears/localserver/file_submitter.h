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

#ifndef GEARS_LOCALSERVER_FILE_SUBMITTER_H__
#define GEARS_LOCALSERVER_FILE_SUBMITTER_H__

#ifdef WINCE
  // FileSubmitter is not implemented for WinCE.
#else

#include "gears/base/common/base_class.h"
#include "gears/base/common/common.h"
#include "gears/base/common/js_types.h"
#include "gears/base/common/security_model.h"
#include "gears/localserver/common/resource_store.h"

class JsDomElement;



#if BROWSER_IE
class ATL_NO_VTABLE FileSubmitterBehaviorFactory
    : public CComObjectRootEx<CComMultiThreadModel>,
      public CComCoClass<FileSubmitterBehaviorFactory>,
      public IElementBehaviorFactory {
 public:
  DECLARE_NOT_AGGREGATABLE(FileSubmitterBehaviorFactory)
  DECLARE_PROTECT_FINAL_CONSTRUCT()

  BEGIN_COM_MAP(FileSubmitterBehaviorFactory)
    COM_INTERFACE_ENTRY(IElementBehaviorFactory)
  END_COM_MAP()

  FileSubmitterBehaviorFactory() {}

  STDMETHOD(FindBehavior)(BSTR name, BSTR url,
                          IElementBehaviorSite* behavior_site,
                          IElementBehavior** behavior_out);

 private:
  std::string16 filename_;

  friend class GearsFileSubmitter;
  DISALLOW_EVIL_CONSTRUCTORS(FileSubmitterBehaviorFactory);
};
#endif



// Facilitates the inclusion of captured local files in form submissions by
// manipulating <input type=file> elements to refer to local files that were
// previously captured via store.CaptureFile().
class GearsFileSubmitter : public ModuleImplBaseClass {
 public:
  GearsFileSubmitter()
      : ModuleImplBaseClass("GearsFileSubmitter")
#if BROWSER_IE
        ,html_element2_(static_cast<IHTMLElement2*>(NULL))
        ,behavior_cookie_(0)
#endif
      {}

  // IN: HtmlElement file_input_element, string captured_url_key
  // OUT: -
  void SetFileInputElement(JsCallContext *context);

 private:
  ResourceStore store_;
  std::string16 name_of_temporary_file_;

#if BROWSER_IE
  CComQIPtr<IHTMLElement2> html_element2_;
  LONG behavior_cookie_;
#endif

  bool CreateTempFile(const std::string16 &in_filename,
                      const WebCacheDB::PayloadInfo &payload);

  bool CaptureInputElement(JsDomElement &dom_element);

  friend class GearsResourceStore;
  DISALLOW_EVIL_CONSTRUCTORS(GearsFileSubmitter);
};

#endif // WINCE
#endif // GEARS_LOCALSERVER_FILE_SUBMITTER_H__
