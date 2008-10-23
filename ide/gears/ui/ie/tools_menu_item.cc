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

#include "gears/base/common/detect_version_collision.h"
#include "gears/ui/common/settings_dialog.h"
#include "gears/ui/ie/tools_menu_item.h"
#include "genfiles/product_constants.h"

#ifdef WINCE

STDMETHODIMP ToolsMenuItem::QueryContextMenu(HMENU hmenu, 
                                             UINT index_menu,
                                             UINT id_cmd_first, 
                                             UINT id_cmd_last,
                                             UINT flags) {
  if (flags == CMF_DEFAULTONLY) {
    return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);
  }

  command_first_ = id_cmd_first;

  InsertMenu(hmenu, index_menu, MF_BYPOSITION, command_first_,
      L"Gears Settings");  // TODO(andreip): [naming] looks like the i18n
                           // strings are not built into the gears dll for WinCE
                           // so loading IDS_REGISTRY_MENU_TEXT does not work.

  return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 1);
}

STDMETHODIMP ToolsMenuItem::GetCommandString(UINT id_cmd, 
                                             UINT flags,
                                             UINT *reserved, 
                                             LPSTR command_name,
                                             UINT command_name_len) {
  if (command_first_ != id_cmd)
    return E_INVALIDARG;

  switch (flags) {
    case GCS_VERB:
    case GCS_HELPTEXT: {
      strncpy(command_name, "Gears Settings",  // [naming]
        command_name_len);
    } break;
    case GCS_VALIDATE:
      break;
    default:
      return E_INVALIDARG;
  }

  return S_OK;
}

STDMETHODIMP ToolsMenuItem::InvokeCommand(LPCMINVOKECOMMANDINFO command_info) {
  if (DetectedVersionCollision()) {
    NotifyUserOfVersionCollision();
    return S_OK;
  }

  SettingsDialog::Run(NULL);

  return S_OK;
}

#else

STDAPI ToolsMenuItem::QueryStatus(const GUID *command_group_id,
                                  ULONG num_commands, OLECMD *commands,
                                  OLECMDTEXT *command_text) {
  // Gears settings menu item is always enabled.
  if (command_group_id || (num_commands != 1) || !commands) {
    return E_FAIL;
  }

  commands->cmdf = OLECMDF_ENABLED;
  return S_OK;
}

STDAPI ToolsMenuItem::Exec(const GUID *command_group_id, DWORD command_id,
                           DWORD exec_options, VARIANTARG *args,
                           VARIANTARG *output) {
  if (DetectedVersionCollision()) {
    NotifyUserOfVersionCollision();
    return S_OK;
  }
  SettingsDialog::Run(NULL);
  return S_OK;
}

#endif
