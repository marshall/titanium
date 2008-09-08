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

#ifndef GEARS_DESKTOP_DESKTOP_H__
#define GEARS_DESKTOP_DESKTOP_H__

#include <vector>

#include "gears/base/common/base_class.h"
#include "gears/base/common/browsing_context.h"
#include "gears/base/common/common.h"
#include "gears/base/common/permissions_db.h"
#include "gears/base/common/security_model.h"
#include "gears/localserver/common/http_request.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

class GearsNotification;
class HtmlDialog;
class IpcMessageQueue;

// This is the backend object that implements our Desktop functionality in
// a module-neutral way.
class Desktop {
 public:
  enum {
    SHORTCUT_LOCATION_DESKTOP     = 0x00000001,
    SHORTCUT_LOCATION_QUICKLAUNCH = 0x00000002,
    SHORTCUT_LOCATION_STARTMENU   = 0x00000004,
  };

  enum DialogStyle {
    DIALOG_STYLE_STANDARD = 0,
    DIALOG_STYLE_SIMPLE,
  };

  struct IconData {
    IconData() : width(0), height(0) {}
    int width;
    int height;
    std::string16 url;
    std::vector<uint8> png_data;
    std::vector<uint8> raw_data;
    DISALLOW_EVIL_CONSTRUCTORS(IconData);
  };

  struct ShortcutInfo {
    ShortcutInfo(){}
    std::string16 app_name;
    std::string16 app_url;
    std::string16 app_description;
    IconData icon16x16;
    IconData icon32x32;
    IconData icon48x48;
    IconData icon128x128;
    DISALLOW_EVIL_CONSTRUCTORS(ShortcutInfo);
  };

  class AbortInterface {
   public:
    // When abort is called, if IconHandlerInterface::ProcessIcon has
    // not yet been called, it will not be called.
    virtual bool Abort() = 0;
  };

  class IconHandlerInterface {
   public:
    // Gets an interface that allows aborting the icon fetch.
    virtual void set_abort_interface(AbortInterface *abort_interface) = 0;

    // The icon that is being fetched.
    virtual IconData *mutable_icon() = 0;

    // Called when the icon fetch has finished.  It will not be called if
    // FetchIcon returns false.
    virtual void ProcessIcon(bool success,
                             const std::string16 &icon_error) = 0;
  };

  Desktop(const SecurityOrigin &security_origin, BrowsingContext *context);

  // Call this after setting up the ShortcutInfo to validate it and check if the
  // shortcut should be created.  If strict_filename is true, we verify that the
  // shortcut name is visible ASCII.  Returns false if the shortcut should not
  // be created, with an optional error message in error().
  bool ValidateShortcutInfo(ShortcutInfo *shortcut_info, bool strict_filename);

  // Initializes the shortcuts dialog.  Should be called after
  // validating the ShortcutInfo.  Returns false on failure, with an optional
  // error message in error().
  bool InitializeDialog(ShortcutInfo *shortcut_info,
                        HtmlDialog *shortcuts_dialog, DialogStyle style);

  // Called to handle the results of an HtmlDialog.  Returns false on failure,
  // with an optional error message in error().
  bool HandleDialogResults(ShortcutInfo *shortcut_info,
                           HtmlDialog *shortcuts_dialog);

  // Fetch icon data.  If icon_handler is NULL, we try to fetch
  // and process the icon synchronously.  Otherwise, we callback
  // the icon_handler when the data is ready.
  bool FetchIcon(IconData *icon, std::string16 *error,
                 IconHandlerInterface *icon_handler);

#if defined(OFFICIAL_BUILD) || defined(OS_ANDROID)
  // The notification API has not been finalized for official builds.
#else
  // Call this after creating a notification to validate it.  Returns false
  // if the notification fails validation checks, with an optional error
  // message in error().
  bool ValidateNotification(GearsNotification *notification);

  // Handle all the every required to actually display the notification.
  // Returns false on failure, with an optional error message in error().
  // NOTE: It takes ownership of released_notification.
  bool ValidateAndShowNotification(GearsNotification *released_notification);

  bool ValidateAndRemoveNotification(GearsNotification *released_notification);
#endif  // defined(OFFICIAL_BUILD) || defined(OS_ANDROID)

  // Error message getters.
  bool has_error() { return !error_.empty(); }
  const std::string16 &error() { return error_; }

 private:
  // NOTE: This method is implemented in desktop_<platform>.cc
  bool CreateShortcutPlatformImpl(const SecurityOrigin &origin,
                                  const ShortcutInfo &shortcut,
                                  uint32 locations,
                                  std::string16 *error);

  bool SetShortcut(ShortcutInfo *shortcut,
                   const bool allow,
                   const bool permanently,
                   uint32 locations,
                   std::string16 *error);

  bool AllowCreateShortcut(const ShortcutInfo &shortcut_info);
  bool WriteControlPanelIcon(const ShortcutInfo &shortcut);
  bool GetControlPanelIconLocation(const SecurityOrigin &origin,
                                   const std::string16 &app_name,
                                   std::string16 *icon_loc);

  bool ResolveUrl(std::string16 *url, std::string16 *error);

  SecurityOrigin security_origin_;

  std::string16 error_;

  // The context to use when fetching the icons.
  scoped_refptr<BrowsingContext> browsing_context_;

  DISALLOW_EVIL_CONSTRUCTORS(Desktop);
};

class GearsDesktop : public ModuleImplBaseClass {
 public:
  GearsDesktop();

  // IN: string name, string url, object icons, optional string description
  // OUT: void
  void CreateShortcut(JsCallContext *context);

  // IN: function callback, optional object open_file_options
  // OUT: void
  void OpenFiles(JsCallContext *context);

#if defined(OFFICIAL_BUILD) || defined(OS_ANDROID)
  // The notification API has not been finalized for official builds.
#else
  // IN: void
  // OUT: void
  void CreateNotification(JsCallContext *context);

  // IN: notification object
  // OUT: void
  void ShowNotification(JsCallContext *context);

  // IN: string id
  // OUT: void
  void RemoveNotification(JsCallContext *context);
#endif  // defined(OFFICIAL_BUILD) || defined(OS_ANDROID)

#if defined(OFFICIAL_BUILD) || defined(OS_ANDROID)
// The Drag-and-Drop API has not been finalized for official builds.
#else
  // IN: DomElement div, object callbacks
  // OUT: -
  void RegisterDropTarget(JsCallContext *context);
#endif

 private:
  DISALLOW_EVIL_CONSTRUCTORS(GearsDesktop);
};

#endif  // GEARS_DESKTOP_DESKTOP_H__
