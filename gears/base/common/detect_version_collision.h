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
//
// Ideally our install/upgrade story wants to be, "you never have to reboot
// your system or restart your browser."  We're not there yet. We're to the
// point of never having to reboot the system. This code helps us deal with
// a case where a browser restart is required.  When an upgrade has been
// downloaded and registered but a previous version is loaded and running,
// thing will not work properly.
// On win32 we can end up with both versions of Gears loaded into the same 
// process and the system is confused about which DLL contains CLSIDs, so 
// neither version is fully functional, yuck.  Similarly, different versions 
// may require different database schemas.
//
// Our general approach for the short term is to have the new version
// cripple itself at load time if it sees an old version still running.
// We could cripple ourselves in several ways:
//
// On win32:
// a. Refuse to load the DLL by returning E_FAIL from DllMain, but this could
//    prevent the new DLL from being registered and cause the installer to fail
// b. Refuse to create COM class factories by returning E_FAIL from
//    DllGetClassObject, but this would deny us any opportunity to let the
//    user know that a restart is required
// c. Clip our wings at the following key entry points
//    - BHO.SetSite() will not activate our HttpHandlerAPP
//    - GearsFactory.create() will not create any other GearsObjects and will
//      alert the user that a restart is required if they haven't already
//      been told
//    - The tools menu item will alert the user that a restart is required
//      instead of showing the settings dialog
//
// Doing (c) for now.
//
// A next step could be to have the new version continue to cripple itself,
// but in a different fashion such that the previous version continues to
// to function as it should. This could be accomplished at the COM class
// factory level. When the new version is queried for a class factory, call
// thru to the previous version's DllGetClassObject function. A browser restart
// would still be required in this scenario, but at least Gears continues to
// function properly.

#ifndef GEARS_BASE_COMMON_DETECT_VERSION_COLLISION_H__
#define GEARS_BASE_COMMON_DETECT_VERSION_COLLISION_H__

#include <ctype.h>

// Returns true if we detected a different version of Gears was running
// at startup. If a collision is detected, this instance of Gears will
// be crippled to prevent data corruption and general mayhem.
bool DetectedVersionCollision();

// Notifies the user if they haven't already been notified of the version
// collision problem.
void MaybeNotifyUserOfVersionCollision();

// Puts up a simple message box alerting the user about the problem
void NotifyUserOfVersionCollision();

#endif  // GEARS_BASE_COMMON_DETECT_VERSION_COLLISION_H__
