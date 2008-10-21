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

#include "gears/ui/common/settings_dialog.h"

#include "gears/base/common/string_utils.h"
#include "gears/ui/common/html_dialog.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

static const char *kPermissionsString = "permissions";
static const char *kNameString = "name";
static const char *kLocalStorageString = "localStorage";
static const char *kLocationDataString = "locationData";
static const char *kPermissionStateString = "permissionState";
static const char16 *kSettingsDialogString = STRING16(L"settings_dialog.html");

// Helper that populates a permission state if one is set in the map.
// Otherwise, it does nothing.
static void AddPermissionState(const PermissionsDB::OriginPermissions &map,
                               PermissionsDB::PermissionType type,
                               Json::Value *entry_out);

bool SettingsDialog::visible_ = false;

#ifdef BROWSER_WEBKIT
void ProcessResultsCallback(Json::Value *result, void *closure) {
  scoped_ptr<HtmlDialog> dialog(static_cast<HtmlDialog *>(closure));
  if (result) {
    SettingsDialog::ProcessResult(result);
  }
  SettingsDialog::SetVisible(false);
}
#endif

void SettingsDialog::Run(void *platform_data) {
  scoped_ptr<HtmlDialog> settings_dialog(new HtmlDialog(platform_data));

  // Populate the arguments property.
  settings_dialog->arguments[kPermissionsString] =
      Json::Value(Json::arrayValue);
  if (!PopulatePermissions(&(settings_dialog->arguments[kPermissionsString]))) {
    return;
  }

  // Show the dialog.
  const int kSettingsDialogWidth = 400;
  const int kSettingsDialogHeight = 350;
  HtmlDialogReturnValue dialog_result;

#ifdef BROWSER_WEBKIT
  visible_ = true;
  dialog_result = settings_dialog->DoModeless(STRING16(kSettingsDialogString),
                                   kSettingsDialogWidth, kSettingsDialogHeight,
                                   ProcessResultsCallback, 
                                   settings_dialog.get());
  if (dialog_result != HTML_DIALOG_SUCCESS) {
    return;
  }
  settings_dialog.release();
#else
  dialog_result = settings_dialog->DoModal(STRING16(kSettingsDialogString),
                                           kSettingsDialogWidth,
                                           kSettingsDialogHeight);

  if (dialog_result == HTML_DIALOG_SUCCESS) {
    // Process the result() property and update any permissions or shortcuts.
    ProcessResult(&settings_dialog->result);
  }
#endif
}


// static
bool SettingsDialog::PopulatePermissions(Json::Value *json_object) {
  // Populate the supplied object with permissions for all origins. The object
  // is an array of objects. Each object has a property with the domain name and
  // properties named for each class of permissions. Each of these is also
  // an object, with a 'permissionState' property and other properties specific
  // to the class. For example, the localStorage permission class may have
  // members describing current disk space use and whether the storage should be
  // cleared.
  // eg.
  // permissions: [
  //   {
  //     name: "http://www.google.com",
  //     localStorage: { permissionState: 1 },
  //     locationData: { permissionState: 2 }
  //   },
  //   { 
  //     name: "http://www.evil.org", 
  //     localStorage: { permissionState: 2 }
  //   }
  // ]
  //
  // TODO(aa): Pass list of created shortcuts here.

  PermissionsDB *capabilities = PermissionsDB::GetDB();
  if (!capabilities) {
    return false;
  }

  PermissionsDB::PermissionsList list;
  if (!capabilities->GetPermissionsSorted(&list)) {
    LOG(("SettingsDialog::PopulatePermissions: Could not get origins."));
    return false;
  }

  assert(json_object->isArray());
  for (PermissionsDB::PermissionsList::size_type i = 0; i < list.size(); ++i) {
    // JSON needs UTF-8.
    std::string origin_string;
    if (!String16ToUTF8(list[i].first.c_str(), &origin_string)) {
      LOG(("SettingsDialog::PopulatePermissions: Could not convert origin "
           "string."));
      return false;
    }
    Json::Value entry;
    entry[kNameString] = origin_string.c_str();
    AddPermissionState(list[i].second,
                       PermissionsDB::PERMISSION_LOCAL_DATA,
                       &entry);

    AddPermissionState(list[i].second,
                       PermissionsDB::PERMISSION_LOCATION_DATA,
                       &entry);

    json_object->append(entry);
  }

  return true;
}

void SettingsDialog::ProcessResult(Json::Value *dialog_result) {
  // Note: the logic here is coupled to the way the JS inside of
  // settings_dialog.html.m4 packages it's results. As implemented now, the
  // dialog should only ever return null or an object containing ...
  // - modifiedOrigins : an array where each element gives the permission
  //                     classes modified for a given origin.
  // - TODO            : an array containing zero or more shortcuts to remove.

  if (Json::nullValue == dialog_result->type()) {
    // Nothing to do, the user pressed cancel.
    return;
  }

  if (!dialog_result->isObject()) {
    LOG(("SettingsDialog::ProcessResult: Invalid dialog_result type."));
    assert(false);
    return;
  }

  PermissionsDB *capabilities = PermissionsDB::GetDB();
  if (!capabilities) {
    return;
  }

  const Json::Value modified_origins = (*dialog_result)["modifiedOrigins"];
  if (!modified_origins.isArray()) {
    LOG(("SettingsDialog::ProcessResult: Invalid modified_origins type."));
    assert(false);
    return;
  }

  for (int i = 0; i < static_cast<int>(modified_origins.size()); ++i) {
    std::string16 origin;
    Json::Value entry = modified_origins[i];
    if (!UTF8ToString16(entry[kNameString].asCString(), &origin)) {
      LOG(("SettingsDialog::ProcessResult: Could not convert name."));
      continue;
    }

    SecurityOrigin security_origin;
    if (!security_origin.InitFromUrl(origin.c_str())) {
      continue;
    }

    if (!entry.isObject()) {
      LOG(("SettingsDialog::ProcessResult: Invalid entry type."));
      assert(false);
      continue;
    }

    Json::Value local_storage_permission =
        entry[kLocalStorageString][kPermissionStateString];
    if (local_storage_permission.isInt()) {
      PermissionsDB::PermissionValue local_storage_permission_value =
          static_cast<PermissionsDB::PermissionValue>(
          local_storage_permission.asInt());
      capabilities->SetPermission(security_origin,
                                  PermissionsDB::PERMISSION_LOCAL_DATA,
                                  local_storage_permission_value);
    }

    Json::Value location_data_permission =
        entry[kLocationDataString][kPermissionStateString];
    if (location_data_permission.isInt()) {
      PermissionsDB::PermissionValue location_data_permission_value =
          static_cast<PermissionsDB::PermissionValue>(
          location_data_permission.asInt());
      capabilities->SetPermission(security_origin,
                                  PermissionsDB::PERMISSION_LOCATION_DATA,
                                  location_data_permission_value);
    }
  }
}

void AddPermissionState(const PermissionsDB::OriginPermissions &map,
                        PermissionsDB::PermissionType type,
                        Json::Value *entry_out) {
  assert(entry_out);
  PermissionsDB::OriginPermissions::const_iterator iter = map.find(type);
  if (iter != map.end()) {
    const char* type_string = NULL;
    switch (type) {
      case PermissionsDB::PERMISSION_LOCAL_DATA:
        type_string = kLocalStorageString;
        break;
      case PermissionsDB::PERMISSION_LOCATION_DATA:
        type_string = kLocationDataString;
        break;
      default:
        assert(false);
    }
    Json::Value permission;
    permission[kPermissionStateString] = iter->second;
    (*entry_out)[type_string] = permission;
  }
}
