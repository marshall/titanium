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

#ifndef GEARS_FACTORY_FACTORY_UTILS_H__
#define GEARS_FACTORY_FACTORY_UTILS_H__

#include "gears/base/common/permissions_db.h"
#include "gears/base/common/string16.h"

class SecurityOrigin;


// The 'classVersion' parameter to factory.create() is reserved / deprecated.
// Currently only '1.0' is allowed.
extern const char16 *kAllowedClassVersion;

// The message string for the exception that is thrown when a module
// cannot be created because the appropriate permissions could not be
// acquired.
extern const char16 *kPermissionExceptionString;

// Appends information about the Gears build to the string provided.
void AppendBuildInfo(std::string16 *s);

// Sets a usage-tracking bit once per instantiation of Gears module. On
// machines that have the Google Update Service available, this bit is
// periodically reported and then reset. Currently Windows-only.
void SetActiveUserFlag();

// Checks if the module requires PERMISSION_LOCAL_DATA to be created.
bool RequiresLocalDataPermissionType(const std::string16 &module_name);

#endif // GEARS_FACTORY_FACTORY_UTILS_H__
