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

#if BROWSER_NPAPI

// HACK: The http_cookies.cc file has both common and platform-specific
// implementations, with a stub implementation for NPAPI.  We want to override
// that.  There are basically two options:
// 1. Fork the whole file.  Change the NPAPI implementation.
// 2. Include the file (undef'ing NPAPI to ensure the stub isn't included), and
// provide just the NPAPI implementation.
// I chose 2 because it allows us to pull in any modifications to the common
// stuff.

#include "gears/base/chrome/browsing_context_cr.h"
#include "gears/base/chrome/module_cr.h"
#include "gears/base/common/async_router.h"
#include "gears/base/common/event.h"
#include "gears/base/common/string_utils.h"
#include "gears/localserver/common/http_cookies.h"

#undef BROWSER_NPAPI
#include "gears/localserver/common/http_cookies.cc"

// We can only call get_cookies from the plugin thread.  So we proxy calls over
// to that thread and wait for their result.
static bool GetCookieStringOnPluginThread(
    const char16 *url, CPBrowsingContext context, std::string16 *cookies_out) {
  assert(IsPluginThread());

  std::string url_utf8;
  if (!String16ToUTF8(url, &url_utf8))
    return false;

  char *cookies_utf8 = NULL;
  g_cpbrowser_funcs.get_cookies(g_cpid, context, url_utf8.c_str(),
                                &cookies_utf8);
  if (!cookies_utf8)
    return false;

  bool rv = UTF8ToString16(cookies_utf8, cookies_out);
  g_cpbrowser_funcs.free(cookies_utf8);
  return rv;
}

class AsyncGetCookieString : public AsyncFunctor {
 public:
  AsyncGetCookieString(Event *done_event, const char16 *url,
                       CPBrowsingContext context,
                       std::string16 *cookies_out, bool *result)
      : done_event_(done_event), url_(url), context_(context),
        cookies_out_(cookies_out), result_(result) {
  }

  virtual void Run() {
    *result_ = GetCookieStringOnPluginThread(url_, context_, cookies_out_);
    done_event_->Signal();
  }

private:
  Event *done_event_;
  const char16 *url_;
  CPBrowsingContext context_;
  std::string16 *cookies_out_;
  bool *result_;
};

bool GetCookieString(const char16 *url, BrowsingContext *context,
                     std::string16 *cookies_out) {
  assert(url);
  assert(cookies_out);
#ifdef USING_CCTESTS
  if (GetFakeCookieString(url, cookies_out)) {
    return true;
  }
#endif

  CRBrowsingContext *cr_context = static_cast<CRBrowsingContext*>(context);
  CPBrowsingContext cp_context = 0;
  if (cr_context)
    cp_context = cr_context->context;
  else
    LOG16((STRING16(L"Warning: GetCookieString called with no context\n")));

  if (IsPluginThread()) {
    return GetCookieStringOnPluginThread(url, cp_context, cookies_out);
  } else {
    bool result;
    Event done_event;
    // http://b/issue?id=1227494 - sometimes mutex.Await hangs forever here.
    // Trying to figure out why.
    // TODO(mpcomplete): remove this bool.  volatile ensures it exists in opt
    // builds.
    volatile bool sent = AsyncRouter::GetInstance()->CallAsync(
        g_cpthread_id,
        new AsyncGetCookieString(&done_event, url, cp_context,
                                 cookies_out, &result));
    assert(sent);
    if (!sent)
      return false;
    done_event.Wait();
    return result;
  }
}
#endif
