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

#import <WebKit/WebKit.h>

#import "gears/base/common/string_utils.h"
#import "gears/base/common/common_sf.h"
#import "gears/base/safari/browser_utils_sf.h"
#import "gears/base/safari/nsstring_utils.h"

//------------------------------------------------------------------------------
static NSString *StringWithLocalizedKeyAndList(NSString *key, va_list list) {
  Class factory = NSClassFromString(@"SafariGearsFactory");
  NSBundle *bundle = [NSBundle bundleForClass:factory];
  NSString *str = [bundle localizedStringForKey:key value:@"" table:nil];
  NSString *msg = [[NSString alloc] initWithFormat:str arguments:list];

  return [msg autorelease];
}

//------------------------------------------------------------------------------
NSString *StringWithLocalizedKey(NSString *key, ...) {
  va_list list;
  NSString *msg;

  va_start(list, key);
  msg = StringWithLocalizedKeyAndList(key, list);
  va_end(list);

  return msg;
}

//------------------------------------------------------------------------------
void ThrowExceptionKey(NSString *key, ...) {
  va_list list;
  NSString *msg;

  va_start(list, key);
  msg = StringWithLocalizedKeyAndList(key, list);
  va_end(list);

  [WebScriptObject throwException:msg];
}

//------------------------------------------------------------------------------
// NPN_SetException is buggy in WebKit, see
// http://bugs.webkit.org/show_bug.cgi?id=16829
void WebKitNPN_SetException(NPObject* obj, const char *message)
{
  NSString *msg = [NSString stringWithCString:message];
  [WebScriptObject throwException:msg];
}

//------------------------------------------------------------------------------
// Returns 0 if there was an error.
// Otherwise output is as returned from gestaltSystemVersion.
static SInt32 SystemVersion() {
  static SInt32 os_version = 0;
  if (os_version) return os_version;

  if (Gestalt(gestaltSystemVersion, &os_version) == noErr) {
    // Do nothing if an error occured.
  }

  return os_version;
}

//------------------------------------------------------------------------------
bool IsLeopardOrGreater() {
  // We only support Leopard >= 10.5.3.
  return SystemVersion() >= 0x1053; // 10.5.3
}
