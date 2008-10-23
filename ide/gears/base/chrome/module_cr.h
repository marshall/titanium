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

#ifndef GEARS_BASE_CHROME_MODULE_CR_H__
#define GEARS_BASE_CHROME_MODULE_CR_H__

#include <assert.h>
#include "gears/base/common/message_queue.h"
#include "gears/base/common/js_types.h"
#include "gears/base/common/serialization.h"
#include "third_party/chrome/chrome_plugin_api.h"
#include "third_party/chrome/gears_api.h"

// TODO(michaeln): remove the externs, use class CP
extern bool g_cpwas_initialized;
extern bool g_cpinitialization_failed;
extern ThreadId g_cpthread_id;
extern CPID g_cpid;
extern CPProcessType g_cpprocess_type;
extern CPBrowserFuncs g_cpbrowser_funcs;
extern CPRequestFuncs g_cprequest_funcs;
extern CPResponseFuncs g_cpresponse_funcs;
extern std::string16 g_plugin_data_dir;
bool IsPluginThread();

class JsCallContext;


class CP {
 public:
  static CPError Initialize(CPID id, const CPBrowserFuncs *bfuncs,
                            CPPluginFuncs *pfuncs);

  static bool WasInitialized() {
    return g_cpwas_initialized;
  }

  static bool DidInitializationFail() {
    return g_cpinitialization_failed;
  }

  static CPID cpid() {
    assert(WasInitialized());
    return g_cpid;
  }

  static bool IsBrowserProcess() { 
    return WasInitialized() && g_cpprocess_type == CP_PROCESS_BROWSER;
  }

  static bool IsPluginProcess() {
    // For our purposes, the renderer and plugin processes are the same: each
    // represents the "other half" of the browser+other split.  We're in the
    // renderer if Chrome was run with --gears-in-renderer.
    return WasInitialized() &&
      (g_cpprocess_type == CP_PROCESS_PLUGIN ||
       g_cpprocess_type == CP_PROCESS_RENDERER);
  }

  static bool IsPluginThread() {
    return WasInitialized() &&
      g_cpthread_id == ThreadMessageQueue::GetInstance()->GetCurrentThreadId();
  }

  static ThreadId plugin_thread_id() {
    assert(WasInitialized());
    return g_cpthread_id; 
  }

  // Returns the browser's CPAPI version.
  static uint16 version() {
    assert(WasInitialized());
    return g_cpbrowser_funcs.version;
  }

  static const CPBrowserFuncs &browser_funcs() {
    assert(WasInitialized());
    assert(IsPluginThread());
    return g_cpbrowser_funcs;
  }

  static const CPRequestFuncs &request_funcs() {
    assert(WasInitialized());
    assert(IsPluginThread());
    return g_cprequest_funcs;
  }

  static const CPResponseFuncs &response_funcs() {
    assert(WasInitialized());
    assert(IsPluginThread());
    return g_cpresponse_funcs;
  }

  // Returns the base data dir under which we should put our "Gears" data dir.
  static const std::string16 &plugin_data_dir() {
    assert(WasInitialized());
    return g_plugin_data_dir;
  }

  // Returns the locale code given to us by the browser (eg, "en-US").
  static const std::string16 &locale() {
    return g_locale;
  }

  // Returns the current CPBrowsingContext.  This should only be called if we're
  // within a callback from JavaScript (ie, we have a JsContextPtr).
  static CPBrowsingContext GetBrowsingContext(JsContextPtr context);

  // Utility function to allocate space for |count| objects using CPB_Alloc.
  template<class T>
  static T *Alloc(int count) {
    return static_cast<T*>(CP::browser_funcs().alloc(sizeof(T) * count));
  }

  // Utility function to strdup a string using the CPB_Alloc.
  static char *StringDup(const std::string &str);

  // Utility function to strdup a string16 (returning UTF8) using the CPB_Alloc.
  static char *String16ToUTF8Dup(const std::string16 &str);

  static bool offline_debug_mode() {
    return g_offline_debug_mode;
  }
  static void set_offline_debug_mode(bool offline);

  // True if Gears was loaded in the renderer process (experimental).  Note:
  // This does not mean that the current process is the renderer process; only
  // that the scriptable component of Gears will be loaded directly in the
  // renderer, instead of spawning a plugin process.
  static bool gears_in_renderer() {
    return g_gears_in_renderer;
  }

  // Generally, chrome terminiates plugin processes after a brief
  // delay once all np instances have been destroyed. We alter this
  // behavior to support long running tasks that aren't directly
  // associated with np instances embedded in pages. If the keep-alive
  // count is greater than zero, our plugin process will not be terminated.
  // These methods may be called from any thread of control.
  static void IncrementProcessKeepAliveCount();
  static void DecrementProcessKeepAliveCount();

 private:
  class AdjustProcessKeepAliveOnPluginThread;
  static void AdjustProcessKeepAlive();

  static bool g_offline_debug_mode;
  static std::string16 g_locale;
  static int g_process_keep_alive_count;
  static bool g_gears_in_renderer;
  //static bool g_cpwas_initialized;
  //static ThreadId g_cpthread_id;
  //static CPID g_cpid;
  //static CPProcessType g_cpprocess_type;
  //static CPBrowserFuncs g_cpbrowser_funcs;
  //static CPRequestFuncs g_cprequest_funcs;
  //static CPResponseFuncs g_cpresponse_funcs;
};


// This class can only be used on the plugin thread
class PluginMessage : public Serializable {
 public:
  bool Send();
  virtual void OnMessageReceived() = 0;
};

#endif  // GEARS_BASE_CHROME_MODULE_CR_H__
