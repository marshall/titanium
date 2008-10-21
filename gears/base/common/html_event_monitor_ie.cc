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

#include "gears/base/common/html_event_monitor.h"


// Provides an IDispatch interface for events to invoke.
//
// Based on sample code from Igor Tandetnik at:
// http://www.eggheadcafe.com/software/aspnet/29873444/default-implementation-fo.aspx
class HtmlEventMonitorHook
    : public IDispEventSimpleImpl<0, HtmlEventMonitorHook, &IID_IDispatch> {
 public:
  HtmlEventMonitorHook(HtmlEventMonitor::HtmlEventCallback function,
                       void *user_param)
      : function_(function),
        user_param_(user_param) {
  }

  IDispatch* GetIDispatch() {
    // suitable for passing to attachEvent()
    return reinterpret_cast<IDispatch*>(this);
  }

  VARIANT_BOOL STDMETHODCALLTYPE HandleEvent(IDispatch *event) {
    // 'event' implements IHTMLEventObj[2|3|4]
    function_(user_param_);  // invoke user callback
    return VARIANT_TRUE; // return VARIANT_FALSE to cancel default action
  }

  BEGIN_SINK_MAP(HtmlEventMonitorHook)
    SINK_ENTRY_INFO(0, IID_IDispatch, DISPID_VALUE, HandleEvent, &EventInfo)
  END_SINK_MAP()

 private:
  HtmlEventMonitor::HtmlEventCallback function_;
  void *user_param_;

  static _ATL_FUNC_INFO EventInfo;

  DISALLOW_EVIL_CONSTRUCTORS(HtmlEventMonitorHook);
};

_ATL_FUNC_INFO HtmlEventMonitorHook::EventInfo = {
    CC_STDCALL, VT_BOOL, 1, { VT_DISPATCH }
};


//
// HtmlEventMonitor
//

bool HtmlEventMonitor::Start(IHTMLWindow3 *event_source) {
  assert(!event_hook_);
  assert(!event_source_);

  // create the event hook
  scoped_ptr<HtmlEventMonitorHook> event_hook(
      new HtmlEventMonitorHook(function_, user_param_));
  event_hook->GetIDispatch()->AddRef();

  // connect to the event source
  CComBSTR event_name_bstr(event_name_.c_str());
  VARIANT_BOOL attached;
  HRESULT hr = event_source->attachEvent(event_name_bstr,
                                         event_hook->GetIDispatch(),
                                         &attached);
  if (FAILED(hr) || !attached) { return false; }

  // only modify data members if everything succeeded
  event_hook_ = event_hook.release();
  event_source_ = event_source;
  return true;
}

void HtmlEventMonitor::Stop() {
  assert(event_source_);
  assert(event_hook_);

  // disconnect from the event source
  CComBSTR event_name_bstr(event_name_.c_str());
  event_source_->detachEvent(event_name_bstr,
                             event_hook_->GetIDispatch());
  event_source_ = NULL;

  // destroy the event hook
  event_hook_->Release();
  event_hook_ = NULL;
}
