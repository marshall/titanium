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

#ifndef GEARS_INSTALLER_IEMOBILE_CAB_UPDATER_H__
#define GEARS_INSTALLER_IEMOBILE_CAB_UPDATER_H__

#include <windows.h>
#include <piedocvw.h>

#include "gears/base/common/common.h"
#include "gears/base/common/message_service.h"
#include "gears/base/common/sqlite_wrapper.h"
#include "gears/base/ie/atl_headers.h"
#include "gears/localserver/common/async_task.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

// This class takes care of periodic version updates. It does this as follows:
// 1. When the Gears BHO is loaded, CAB updater is started immediately after
// the HTTP handler is registered.
// 2. The CAB updater sets up a timer and goes to sleep for some time
// 3. When the CAB updater wakes up, it does a HTTP GET to the upgrade URL
// and aborts after the first redirect.
// 4. When the request completes, the updater inspects the Location header
// and extracts the latest version number.
// 5. The CAB updater checks the latest version number vs. the current version
// number. If the two are different,
// 6. The CAB updater asks the user if it should proceed with the upgrade. If
// the user agrees,
// 7. The CAB updater uses the IWebBrowser2 pointer to make the browser
// download the new CAB.
// 8. The updater waits for 24 hours, then continues at step 3.

// Forward declarations
class PeriodicChecker;

class CabUpdater : public MessageObserverInterface {
 public:
  CabUpdater();
  virtual ~CabUpdater();
  // Starts the updater. Sleep a few seconds and work out if an update check
  // is due.
  void SetSiteAndStart(IWebBrowser2* browser);

  // MessageObserverInterface

  // Listens and responds to update events (sent as SerializableString16).
  // The NotificationData event object will hold the URL the browser
  // should be pointed to.
  virtual void OnNotify(MessageService *service,
                        const char16 *topic,
                        const NotificationData *data);

 private:
  // Stop method called when the DLL unloads.
  static void Stop(void* self);
  // Shows the update dialog to the user.
  bool ShowUpdateDialog(HWND browser_window);
  // We drive the browser through this IWebBrowser2 pointer. Not owned.
  IWebBrowser2* browser_;
  // The periodic update checker. Owned.
  PeriodicChecker* checker_;
  // Is the update dialog showing?
  bool is_showing_update_dialog_;

  DISALLOW_EVIL_CONSTRUCTORS(CabUpdater);
};

#endif  // GEARS_INSTALLER_IEMOBILE_CAB_UPDATER_H__
