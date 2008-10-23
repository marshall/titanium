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

#ifndef GEARS_DESKTOP_DROP_TARGET_IE_H__
#define GEARS_DESKTOP_DROP_TARGET_IE_H__
#ifdef OFFICIAL_BUILD
// The Drag-and-Drop API has not been finalized for official builds.
#else
#ifdef WINCE
// The Drag-and-Drop API is not implemented on Windows CE.
#else

#include <windows.h>
#include "gears/base/common/base_class.h"
#include "gears/base/common/js_runner.h"
#include "gears/base/common/js_types.h"
#include "gears/base/common/scoped_refptr.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

class ATL_NO_VTABLE DropTarget
    : public CComObjectRootEx<CComMultiThreadModel>,
      public CComCoClass<DropTarget>,
      public IDispatchImpl<IDispatch>,
      public IElementBehavior,
      public IElementBehaviorFactory,
      public JsEventHandlerInterface {
 public:
  DECLARE_NOT_AGGREGATABLE(DropTarget)
  DECLARE_PROTECT_FINAL_CONSTRUCT()

  BEGIN_COM_MAP(DropTarget)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IElementBehavior)
    COM_INTERFACE_ENTRY(IElementBehaviorFactory)
  END_COM_MAP()

  CComPtr<IElementBehaviorSite> element_behavior_site_;
  CComPtr<IHTMLWindow2> html_window_2_;

  scoped_refptr<ModuleEnvironment> module_environment_;
  scoped_ptr<JsEventMonitor> unload_monitor_;
  scoped_ptr<JsRootedCallback> on_drag_enter_;
  scoped_ptr<JsRootedCallback> on_drag_over_;
  scoped_ptr<JsRootedCallback> on_drag_leave_;
  scoped_ptr<JsRootedCallback> on_drop_;

  DropTarget() {}

  // IDispatch interface.
  STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid,
    LCID lcid, WORD wFlags, DISPPARAMS *pdispparams, VARIANT *pvarResult,
    EXCEPINFO *pexcepinfo, UINT *puArgErr);

  // IElementBehavior interface.
  STDMETHOD(Detach)(void);
  STDMETHOD(Init)(IElementBehaviorSite *pElementBehaviorSite);
  STDMETHOD(Notify)(long lEvent, VARIANT *pVar);

  // IElementBehaviorFactory interface.
  STDMETHOD(FindBehavior)(BSTR name, BSTR url,
                          IElementBehaviorSite *behavior_site,
                          IElementBehavior **behavior_out);

  HRESULT GetHtmlDataTransfer(CComPtr<IHTMLEventObj> &html_event_obj,
                              CComPtr<IHTMLDataTransfer> &html_data_transfer);
  HRESULT CancelEventBubble(CComPtr<IHTMLEventObj> &html_event_obj,
                            CComPtr<IHTMLDataTransfer> &html_data_transfer);

  HRESULT HandleOnDragEnter();
  HRESULT HandleOnDragOver();
  HRESULT HandleOnDragLeave();
  HRESULT HandleOnDragDrop();

  virtual void HandleEvent(JsEventType event_type);

 private:
  DISALLOW_EVIL_CONSTRUCTORS(DropTarget);
};

#endif  // WINCE
#endif  // OFFICIAL_BUILD
#endif  // GEARS_DESKTOP_DROP_TARGET_IE_H__
