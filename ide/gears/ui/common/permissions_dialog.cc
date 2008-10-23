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

#include "gears/base/common/common.h"
#include "gears/base/common/js_types.h"
#include "gears/base/common/permissions_db.h"
#include "gears/base/common/string_utils.h"
#include "gears/base/common/url_utils.h"
#include "third_party/jsoncpp/json.h"
#include "gears/ui/common/html_dialog.h"
#include "gears/ui/common/permissions_dialog.h"

const char16 *kLocalDataDialogType = STRING16(L"localData");
const char16 *kLocationDataDialogType = STRING16(L"locationData");

// Internal helper functions.
static bool ToJsonStringValue(const char16 *str, Json::Value *json_value);
static bool PopulateArguments(const SecurityOrigin &origin,
                              const char16* dialog_type,
                              const char16 *custom_icon,
                              const char16 *custom_name,
                              const char16 *custom_message,
                              Json::Value *args);
static const char16* DialogTypeForPermission(
    PermissionsDB::PermissionType type);

//------------------------------------------------------------------------------
// PermissionsDialog

PermissionState PermissionsDialog::Prompt(
    const SecurityOrigin &origin,
    PermissionsDB::PermissionType type,
    const CustomContent *custom_content) {
  // Note: Arguments and results are coupled to the values that
  // permissions_dialog.html.m4 is expecting.
  const char16* dialog_type = DialogTypeForPermission(type);
  HtmlDialog dialog;
  const char16 *custom_icon = NULL;
  const char16 *custom_name = NULL;
  const char16 *custom_message = NULL;
  if (custom_content) {
    custom_icon = custom_content->custom_icon_url.c_str();
    custom_name = custom_content->custom_name.c_str();
    custom_message = custom_content->custom_message.c_str();
  }

  if (!PopulateArguments(origin,
                         dialog_type,
                         custom_icon,
                         custom_name,
                         custom_message,
                         &dialog.arguments)) {
    return NOT_SET;
  }

#ifdef WINCE
  const int kDialogWidth = 230;
  const int kDialogHeight = 240;
#else
  const int kDialogWidth = 360;
  const int kDialogHeight = 220;
#endif
  const char16 *kDialogFile = STRING16(L"permissions_dialog.html");

  HtmlDialogReturnValue retval =
      dialog.DoModal(kDialogFile, kDialogWidth, kDialogHeight);

  if (retval == HTML_DIALOG_FAILURE) {
    LOG(("PermissionsDialog::Prompt() - Unexpected result."));
    return NOT_SET;
  }

  // Extract the dialog results.
  bool allow;
  bool permanently;

  if (retval == HTML_DIALOG_SUPPRESSED) {
    allow = true;
    permanently = true;
  } else if (dialog.result == Json::Value::null) {
    // A null dialog.result is okay; it means the user closed the dialog
    // instead of pressing any button.
    allow = false;
    permanently = false;
  } else {
    if (!dialog.result["allow"].isBool() ||
        !dialog.result["permanently"].isBool()) {
      assert(false);
      LOG(("PermissionsDialog::Prompt() - Unexpected result."));
      return NOT_SET;
    }

    allow = dialog.result["allow"].asBool();
    permanently = dialog.result["permanently"].asBool();
  }

  // Store permanent results in the DB.
  if (permanently) {
    PermissionsDB::PermissionValue value = 
                       allow ? PermissionsDB::PERMISSION_ALLOWED
                             : PermissionsDB::PERMISSION_DENIED;
    PermissionsDB *permissions = PermissionsDB::GetDB();
    if (permissions) {
      permissions->SetPermission(origin, type, value);
    }
  }

  // Convert the results to a PermissionState value.
  if (allow) {
    if (permanently) return ALLOWED_PERMANENTLY;
    else return ALLOWED_TEMPORARILY;
  } else {
    if (permanently) return DENIED_PERMANENTLY;
    else return DENIED_TEMPORARILY;
  }
}

PermissionsDialog::CustomContent* PermissionsDialog::CreateCustomContent(
      JsCallContext *context) {
  std::string16 site_name;
  std::string16 image_url;
  std::string16 extra_message;

  JsArgument argv[] = {
    { JSPARAM_OPTIONAL, JSPARAM_STRING16, &site_name },
    { JSPARAM_OPTIONAL, JSPARAM_STRING16, &image_url },
    { JSPARAM_OPTIONAL, JSPARAM_STRING16, &extra_message },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  
  if (context->is_exception_set())
    return NULL;

  return new CustomContent(image_url.c_str(),
                           site_name.c_str(),
                           extra_message.c_str());
}

//------------------------------------------------------------------------------
// PermissionsDialog::CustomContent
 
PermissionsDialog::CustomContent::CustomContent(
    const char16 *custom_icon_url_in,
    const char16 *custom_name_in,
    const char16 *custom_message_in) 
    : custom_icon_url(custom_icon_url_in),
      custom_name(custom_name_in),
      custom_message(custom_message_in) {
}

//------------------------------------------------------------------------------
// Internal

static bool ToJsonStringValue(const char16 *str, Json::Value *json_value) {
  assert(str);
  assert(json_value);

  std::string str8;
  if (!String16ToUTF8(str, &str8)) {
    LOG(("PermissionsDialog::ToJsonStringValue: Could not convert string.\n"));
    return false;
  }
  *json_value = Json::Value(str8);
  return true;
}


static bool PopulateArguments(const SecurityOrigin &origin,
                              const char16* dialog_type,
                              const char16 *custom_icon,
                              const char16 *custom_name,
                              const char16 *custom_message,
                              Json::Value *args) {
  assert(args);
  assert(args->isObject());
  assert(args->size() == 0);
  assert(dialog_type && dialog_type[0]);

  // Show something more user-friendly for kUnknownDomain.
  // TODO(aa): This is needed by settings dialog too. Factor this out into a
  // common utility.
  std::string16 display_origin(origin.url());
  if (origin.host() == kUnknownDomain) {
    ReplaceAll(display_origin,
               std::string16(kUnknownDomain),
               std::string16(STRING16(L"<no domain>")));
  }

  if (!ToJsonStringValue(display_origin.c_str(), &((*args)["origin"]))) {
    return false;
  }

  if (!ToJsonStringValue(dialog_type, &((*args)["dialogType"]))) {
    return false;
  }

  if (custom_icon && custom_icon[0]) {
    std::string16 full_icon_url;
    if (!ResolveAndNormalize(origin.full_url().c_str(),
                             custom_icon,
                             &full_icon_url) ||
        !ToJsonStringValue(full_icon_url.c_str(), &((*args)["customIcon"]))) {
      return false;
    }
  }

  if (custom_name && custom_name[0] &&
      !ToJsonStringValue(custom_name, &((*args)["customName"]))) {
    return false;
  }

  if (custom_message && custom_message[0] &&
      !ToJsonStringValue(custom_message, &((*args)["customMessage"]))) {
    return false;
  }

  return true;
}

static const char16* DialogTypeForPermission(
    PermissionsDB::PermissionType type) {
  switch (type) {
    case PermissionsDB::PERMISSION_LOCAL_DATA:
      return kLocalDataDialogType;
    case PermissionsDB::PERMISSION_LOCATION_DATA:
      return kLocationDataDialogType;
    default:
      LOG(("Invalid permission type.\n"));
      assert(false);
      return NULL;
  };
}
