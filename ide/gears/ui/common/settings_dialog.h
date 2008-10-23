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

#ifndef GEARS_UI_COMMON_SETTINGS_DIALOG_H__
#define GEARS_UI_COMMON_SETTINGS_DIALOG_H__

#include "gears/base/common/permissions_db.h"
#include "third_party/jsoncpp/json.h"

// For now this is very simple and only gives a way to revoke decisions made
// with the "remember me" checkbox on the capabilities prompt.
class SettingsDialog {
 public:
  // Show the settings dialog and apply any changes the user makes.
  static void Run(void *platform_data);

  static bool IsVisible() { return visible_; }

 private:
  // Private constructor. Use static Run() method instead.
  SettingsDialog() {}

  // Helper method. Populates the permissions object passed to dialogArguments.
  static bool PopulatePermissions(Json::Value *json_object);
  
  // Helper method. Populates the shortcut array passed to dialogArguments.
  static bool PopulateShortcuts(Json::Value *json_array);

  // Helper method. Process the results of the dialog.
  static void ProcessResult(Json::Value *dialogResult);
  
  // Set the visibility flag for the dialog.
  static void SetVisible(bool visible) { visible_ = visible; }
 
  // Is the settings dialog currently visible.
  static bool visible_;

 private:
  friend void ProcessResultsCallback(Json::Value *result, void *closure);
};

#endif  // GEARS_UI_COMMON_SETTINGS_DIALOG_H__
