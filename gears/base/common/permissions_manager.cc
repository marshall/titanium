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

#include "gears/base/common/permissions_manager.h"

// Internal conversion functions
static bool ConvertStateToBool(PermissionState);
static PermissionState ConvertValueToState(PermissionsDB::PermissionValue);

//------------------------------------------------------------------------------
// PermissionsManager

PermissionsManager::PermissionsManager(const SecurityOrigin &security_origin,
                                       bool is_worker)
    : security_origin_(security_origin), is_worker_(is_worker) {
}

bool PermissionsManager::AcquirePermission(PermissionsDB::PermissionType type) {
  return AcquirePermission(type, NULL);
}

bool PermissionsManager::AcquirePermission(
    PermissionsDB::PermissionType type,
    const PermissionsDialog::CustomContent *custom) {

  // Check if we already have a decision.
  if (GetPriorDecision(type) == NOT_SET && !is_worker_) {
    // Could not find a prior decision. Ask the user.
    permission_state_[type] = PermissionsDialog::Prompt(security_origin_,
                                                        type,
                                                        custom);
  }

  // Return the boolean decision.
  return ConvertStateToBool(permission_state_[type]);
}

bool PermissionsManager::HasPermission(PermissionsDB::PermissionType type) {
    return ConvertStateToBool(GetPriorDecision(type));
}

void PermissionsManager::ImportPermissions(
    const PermissionsManager &other_manager) {
  permission_state_ = other_manager.permission_state_;
}

//------------------------------------------------------------------------------
// Internal

PermissionState PermissionsManager::GetPriorDecision(
    PermissionsDB::PermissionType type) {
  // Check for a cached state.
  if (permission_state_.find(type) == permission_state_.end()) {
    // Try loading the state from database.
    PermissionsDB *permissions = PermissionsDB::GetDB();
    if (!permissions) { return NOT_SET; }

    permission_state_[type] =
      ConvertValueToState(permissions->GetPermission(security_origin_, type));
  }

  return permission_state_[type];
}

static bool ConvertStateToBool(PermissionState state) {
  switch(state) {
    case ALLOWED_PERMANENTLY:
    case ALLOWED_TEMPORARILY:
      return true;
    case DENIED_PERMANENTLY:
    case DENIED_TEMPORARILY:
    case NOT_SET:
      return false;
    default:
      LOG(("ConvertStateToBool: impossible permission state"));
      assert(false);
      return false;
  }
}

static PermissionState ConvertValueToState(PermissionsDB::PermissionValue value) {
  switch (value) {
    case PermissionsDB::PERMISSION_ALLOWED:
      return ALLOWED_PERMANENTLY;
    case PermissionsDB::PERMISSION_DENIED:
      return DENIED_PERMANENTLY;
    case PermissionsDB::PERMISSION_NOT_SET:
      return NOT_SET;
    // All other values are unexpected.
    default:
      LOG(("ConvertValueToState: impossible permission value"));
      assert(false);
      return NOT_SET;
  }
}
