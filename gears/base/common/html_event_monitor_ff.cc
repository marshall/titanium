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

#include <gecko_sdk/include/nsIDOMEvent.h>
#include <gecko_sdk/include/nsIDOMEventListener.h>
#include <assert.h>

#include "gears/base/common/html_event_monitor.h"


// Provides an nsIDOMEventListener interface for events to invoke.
class HtmlEventMonitorHook
    : public nsIDOMEventListener {
 public:
  HtmlEventMonitorHook(HtmlEventMonitor::HtmlEventCallback function,
                       void *user_param)
      : function_(function),
        user_param_(user_param) {
  }

  NS_IMETHOD HandleEvent(nsIDOMEvent *event) {
    function_(user_param_);  // invoke user callback
    return NS_OK;
  }

  NS_DECL_ISUPPORTS

 private:
  nsIDOMEventTarget *orig_target;

  HtmlEventMonitor::HtmlEventCallback function_;
  void *user_param_;

  DISALLOW_EVIL_CONSTRUCTORS(HtmlEventMonitorHook);
};

NS_IMPL_ISUPPORTS1(HtmlEventMonitorHook, nsIDOMEventListener)


//
// HtmlEventMonitor
//

bool HtmlEventMonitor::Start(nsIDOMEventTarget *event_source) {
  assert(!event_hook_);
  assert(!event_source_);

  // create the event hook
  scoped_ptr<HtmlEventMonitorHook> event_hook(
      new HtmlEventMonitorHook(function_, user_param_));
  event_hook->AddRef();

  // connect to the event source
  nsDependentString firefox_event_name(&event_name_[2]); // skip leading "on"
  nsresult nr = event_source->AddEventListener(firefox_event_name,
                                               event_hook.get(), PR_FALSE);
  if (NS_FAILED(nr)) { return false; }

  // only modify data members if everything succeeded
  event_hook_ = event_hook.release();
  event_source_ = event_source;
  return true;
}

void HtmlEventMonitor::Stop() {
  assert(event_source_);
  assert(event_hook_);

  // disconnect from the event source
  nsDependentString firefox_event_name(&event_name_[2]); // skip leading "on"
  event_source_->RemoveEventListener(firefox_event_name,
                                     event_hook_, PR_FALSE);
  event_source_ = NULL;

  // destroy the event hook
  event_hook_->Release();
  event_hook_ = NULL;
}
