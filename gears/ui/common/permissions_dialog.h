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

#ifndef GEARS_UI_COMMON_PERMISSIONS_DIALOG_H__
#define GEARS_UI_COMMON_PERMISSIONS_DIALOG_H__

#include "gears/base/common/security_model.h"

enum PermissionState;
class JsCallContext;

class PermissionsDialog {
 public:
  // CustomContent wraps a custom icon, name, and message that can be optionally
  // displayed by the permissions dialog. The 'custome_icon_url' should be a
  // full url to a 32x32 image suitable for use in an HTML image tag, and
  // 'custom_message' should not contain HTML markup.
  struct CustomContent {
    CustomContent(const char16 *custom_icon_url_in,
                  const char16 *custom_name_in,
                  const char16 *custom_message_in);
    std::string16 custom_icon_url;
    std::string16 custom_name;
    std::string16 custom_message;
  };

  // Utility method that creates a CustomContent object from a JsCallContext,
  // expecting that the JsCallContext contains three string arguments in the   
  // following order: site_name, image_url, extra_message.
  static CustomContent* CreateCustomContent(JsCallContext *context);    

  // Displays a modal dialog asking the user whether to let the given origin
  // use Gears.  Updates the PermissionsDB if the user's choice is permanent.
  // Returns a PermissionState value indicating the user's choice.
  // The custom_content parameter is optional. If NULL, a default icon and
  // message will be shown.
  static PermissionState Prompt(const SecurityOrigin &origin,
                                PermissionsDB::PermissionType type,
                                const CustomContent *custom_content);
 private:
  // Private constructor. Use static Prompt() methods instead.
  PermissionsDialog() {}
};

#endif  // GEARS_UI_COMMON_PERMISSIONS_DIALOG_H__
