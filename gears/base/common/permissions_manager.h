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

#ifndef GEARS_BASE_COMMON_PERMISSIONS_MANAGER_H__
#define GEARS_BASE_COMMON_PERMISSIONS_MANAGER_H__

#include <map>

#include "gears/base/common/permissions_db.h"
#include "gears/ui/common/permissions_dialog.h"

class PermissionsManager {
 public:
  PermissionsManager(const SecurityOrigin &security_origin, bool is_worker);

  // Attempts to acquire the given type of permission. If the permission is not
  // currently set (either temporarily or in the database), it prompts the user.
  bool AcquirePermission(PermissionsDB::PermissionType type);

  // Attempts to acquire the given type of permission. If the permission is not
  // currently set (either temporarily or in the database), it prompts the user.
  // The permissions prompt is customized with the contents of the 'custom'
  // object.
  bool AcquirePermission(PermissionsDB::PermissionType type,
                         const PermissionsDialog::CustomContent *custom);

  // Returns true if the owning module has the given permission type and false
  // otherwise.
  bool HasPermission(PermissionsDB::PermissionType type);

  // Copies the permission state from the given permission manager.
  void ImportPermissions(const PermissionsManager &other_manager);

 private:
  PermissionState GetPriorDecision(PermissionsDB::PermissionType type);

  std::map<PermissionsDB::PermissionType, PermissionState> permission_state_;
  const SecurityOrigin security_origin_;
  const bool is_worker_;

  DISALLOW_EVIL_CONSTRUCTORS(PermissionsManager);
};

#endif  // GEARS_BASE_COMMON_PERMISSIONS_MANAGER_H__
