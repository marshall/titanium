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

#ifndef GEARS_BASE_COMMON_EXCEPTION_HANDLER_OSX_NSHOST_MACADDRESS_H__
#define GEARS_BASE_COMMON_EXCEPTION_HANDLER_OSX_NSHOST_MACADDRESS_H__

#import <Cocoa/Cocoa.h>

//  Add IOKit.framework and libcrypto to your project if you use this file.

// HARDWARE REQUIREMENTS
// This category uses (and requires) there to be an Ethernet configuration.
// It will use the primary one if there are multiple ones available.

@interface NSHost (MACAddress)

// MACAddress and obfuscatedMACAddress return the MAC address of this host.
// They must be called from [NSHost currentHost];
// The result may be used as an ID which is unique to this host.

// returns six bytes as a string formatted like "xx:xx:xx:xx:xx:xx"
// where <xx> is a two character hexadecimal representation of each byte
- (NSString *)MACAddress;

// a version which returns an obfuscated version for privacy.
// The returned string is 32 characters long.
- (NSString *)obfuscatedMACAddress;

@end
#endif  // GEARS_BASE_COMMON_EXCEPTION_HANDLER_OSX_NSHOST_MACADDRESS_H__
