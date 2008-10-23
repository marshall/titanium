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
#include "gears/ui/firefox/ui_utils.h"

// Boilerplate.
NS_IMPL_ADDREF(GearsUiUtils)
NS_IMPL_RELEASE(GearsUiUtils)
NS_INTERFACE_MAP_BEGIN(GearsUiUtils)
  NS_INTERFACE_MAP_ENTRY(GearsUiUtilsInterface)
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

// Object identifiers
const char *kGearsUiUtilsContractId = "@google.com/gears/ui-utils;1";
const char *kGearsUiUtilsClassName = "GearsUiUtils";
const nsCID kGearsUiUtilsClassId = {0x79e432c2, 0xe802, 0x4670, {0x90, 0x77,
                                    0x42, 0x46, 0x64, 0x11, 0x9b, 0xb9}};
                                   // {79E432C2-E802-4670-9077-424664119BB9}

NS_IMETHODIMP GearsUiUtils::ShowSettingsDialog() {
  SettingsDialog::Run(NULL);
  return NS_OK;
}
