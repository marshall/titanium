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

#ifndef GEARS_BASE_COMMON_HTML_EVENT_MONITOR_H__
#define GEARS_BASE_COMMON_HTML_EVENT_MONITOR_H__

#if BROWSER_FF
#include <gecko_sdk/include/nsXPCOM.h>
#include <gecko_sdk/include/nsIDOMEventTarget.h>
#elif BROWSER_IE
#include <mshtml.h>
#include "gears/base/ie/atl_headers.h"
#endif

#include "gears/base/common/base_class.h"
#include "gears/base/common/basictypes.h"  // for DISALLOW_EVIL_CONSTRUCTORS
#include "gears/base/common/string16.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

extern const char16 *kEventUnload;

class HtmlEventMonitorHook;  // defined in html_event_monitor_<browser>.cc


// Monitors an event on the top-level _window_ of an HTML page.
//
// The callback function AND THE DATA pointed at by user_param
// must be static (to exist for the lifetime of the monitor).
class HtmlEventMonitor {
 public:
  typedef void (*HtmlEventCallback)(void *user_param);

  HtmlEventMonitor(const char16 *event_name,
                   HtmlEventCallback function, void *user_param);
  ~HtmlEventMonitor();

#if BROWSER_FF
  bool Start(nsIDOMEventTarget *event_source);
#elif BROWSER_IE
  bool Start(IHTMLWindow3 *event_source);
#elif BROWSER_NPAPI
  bool Start(NPP context, NPObject *event_source);
#endif
  void Stop();

 private:
  std::string16 event_name_;
  HtmlEventCallback function_;
  void *user_param_;

  HtmlEventMonitorHook *event_hook_;

#if BROWSER_FF
  nsCOMPtr<nsIDOMEventTarget> event_source_;
#elif BROWSER_IE
  CComPtr<IHTMLWindow3> event_source_;
#elif BROWSER_NPAPI
  NPP event_context_;
  ScopedNPVariant event_source_;
#endif

  DISALLOW_EVIL_CONSTRUCTORS(HtmlEventMonitor);
};


#endif  // GEARS_BASE_COMMON_HTML_EVENT_MONITOR_H__
