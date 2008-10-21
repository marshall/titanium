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

#import "gears/base/common/common.h"
#import "gears/base/common/exception_handler.h"
#import "gears/base/common/exception_handler_osx/google_breakpad.h"
#import "gears/base/common/paths_sf_more.h"

static GoogleBreakpadRef gBreakpadRef = 0;

NSString *kCrashReportProductName = @"Google_Gears";  // [naming]
NSString *kCrashReportProductVersion = @PRODUCT_VERSION_STRING_ASCII
                                            " (osx safari)";
NSString *kCrashReportURL = @"https://www.google.com/cr/report";


ExceptionManager::ExceptionManager(bool catch_entire_process) {
  // TODO(playmobil): Make use of catch_entire_process parameter.
  assert(catch_entire_process);
}

ExceptionManager::~ExceptionManager() {
  GoogleBreakpadRelease(gBreakpadRef);
  gBreakpadRef = NULL;
}

void ExceptionManager::StartMonitoring() {
  assert(gBreakpadRef == NULL);
  
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  GoogleBreakpadRef breakpad = 0;
  
  NSMutableDictionary *settings = [[[NSMutableDictionary alloc] init]
                                         autorelease];
  [settings setObject:kCrashReportProductName forKey:@GOOGLE_BREAKPAD_PRODUCT];
  [settings setObject:kCrashReportProductName 
               forKey:@GOOGLE_BREAKPAD_PRODUCT_DISPLAY];
  [settings setObject:kCrashReportProductVersion 
               forKey:@GOOGLE_BREAKPAD_VERSION];
  [settings setObject:kCrashReportURL forKey:@GOOGLE_BREAKPAD_URL];
  [settings setObject:@"3600" forKey:@GOOGLE_BREAKPAD_REPORT_INTERVAL];
  [settings setObject:@"YES" forKey:@GOOGLE_BREAKPAD_SKIP_CONFIRM];
  // Forward exception to Apple's Crashreporter.
  [settings setObject:@"NO" forKey:@GOOGLE_BREAKPAD_SEND_AND_EXIT];
  
#ifdef BROWSER_NONE
  // Supplementary applications, like Notifier, live in the resource path of the
  // Gears plugin, which also contains the reporter & inspector executables.
  NSString *gears_resource_dir =
      [[[NSBundle mainBundle] bundlePath] stringByDeletingLastPathComponent];
#else
  // Set paths for reporter & inspector executables.
  NSString *gears_resource_dir = [GearsPathUtilities gearsResourcesDirectory];
#endif
  NSString *inspector_path = 
      [gears_resource_dir stringByAppendingPathComponent:@"crash_inspector"];
  NSString *reporter_path = 
      [gears_resource_dir stringByAppendingPathComponent:@"crash_sender"];
 
  [settings setObject:inspector_path 
               forKey:@GOOGLE_BREAKPAD_INSPECTOR_LOCATION];
  [settings setObject:reporter_path
               forKey:@GOOGLE_BREAKPAD_REPORTER_EXE_LOCATION];
  breakpad = GoogleBreakpadCreate(settings);
  if (!breakpad) {
    NSLog(@"Breakpad init failed.");
    return;
  }
  
  [pool release];
  gBreakpadRef = breakpad;
}

bool ExceptionManager::ReportAndContinue() {
  // assert(gBreakpadRef);
  // if (!gBreakpadRef) return false;
  // TODO(playmobil): implement - Calling ReportAndContinue seems to crash the
  // process. 
  // GoogleBreakpadGenerateAndSendReport(gBreakpadRef);
  return true;
}
