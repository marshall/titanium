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

#import <pwd.h>
#import <sys/stat.h>
#import <unistd.h>

#import <SystemConfiguration/SystemConfiguration.h>

#import "third_party/breakpad_osx/src/common/mac/HTTPMultipartUpload.h"
#import "gears/base/common/exception_handler_osx/nshost_macaddress.h"

#import "gears/crash_sender/crash_sender_osx.h"

#define kLastSubmission @"LastSubmission"

// Gears: Increase default crash limit of 200K -> 800K.
const int kMinidumpFileLengthLimit = 800000;

#define kApplePrefsSyncExcludeAllKey @"com.apple.PreferenceSync.ExcludeAllSyncKeys"

@interface Reporter(PrivateMethods)
+ (uid_t)consoleUID;

- (id)initWithConfigurationFD:(int)fd;

- (NSString *)readString;
- (NSData *)readData:(size_t)length;

- (BOOL)readConfigurationData;
- (BOOL)readMinidumpData;

- (BOOL)askUserPermissionToSend:(BOOL)shouldSubmitReport;
- (BOOL)shouldSubmitReport;
@end

@implementation Reporter
//=============================================================================
+ (uid_t)consoleUID {
  SCDynamicStoreRef store =
    SCDynamicStoreCreate(kCFAllocatorDefault, CFSTR("Reporter"), NULL, NULL);
  uid_t uid = -2;  // Default to "nobody"
  if (store) {
    CFStringRef user = SCDynamicStoreCopyConsoleUser(store, &uid, NULL);

    if (user)
      CFRelease(user);
    else
      uid = -2;

    CFRelease(store);
  }

  return uid;
}

//=============================================================================
- (id)initWithConfigurationFD:(int)fd {
  if ((self = [super init])) {
    configFile_ = fd;
  }
  
  // Because the reporter is embedded in the framework (and many copies
  // of the framework may exist) its not completely certain that the OS
  // will obey the com.apple.PreferenceSync.ExcludeAllSyncKeys in our
  // Info.plist. To make sure, also set the key directly if needed.
  NSUserDefaults *ud = [NSUserDefaults standardUserDefaults];
  if (![ud boolForKey:kApplePrefsSyncExcludeAllKey]) {
    [ud setBool:YES forKey:kApplePrefsSyncExcludeAllKey];
  }

  return self;
}

//=============================================================================
- (NSString *)readString {
  NSMutableString *str = [NSMutableString stringWithCapacity:32];
  char ch[2] = { 0 };

  while (read(configFile_, &ch[0], 1) == 1) {
    if (ch[0] == '\n') {
      // Break if this is the first newline after reading some other string
      // data.
      if ([str length])
        break;
    } else {
      [str appendString:[NSString stringWithUTF8String:ch]];
    }
  }

  return str;
}

//=============================================================================
- (NSData *)readData:(size_t)length {
  NSMutableData *data = [NSMutableData dataWithLength:length];
  char *bytes = (char *)[data bytes];

  if (read(configFile_, bytes, length) != length)
    return nil;

  return data;
}

//=============================================================================
- (BOOL)readConfigurationData {
  parameters_ = [[NSMutableDictionary alloc] init];

  while (1) {
    NSString *key = [self readString];

    if (![key length])
      break;

    // Read the data.  Try to convert to a UTF-8 string, or just save
    // the data
    NSString *lenStr = [self readString];
    size_t len = [lenStr intValue];
    NSData *data = [self readData:len];
    id value = [[NSString alloc] initWithData:data
                                     encoding:NSUTF8StringEncoding];

    [parameters_ setObject:value ? value : data forKey:key];
    [value release];
  }
 
#if 0
  // !!@ JUST FOR TESTING
  [parameters_ setObject:@"******** THIS IS NOT A REAL CRASH - JUST TESTING!! *******************"
    forKey:@"TestingMessage1"];
  [parameters_ setObject:@"******** THIS IS NOT A REAL CRASH - IGNORE!! *******************"
    forKey:@"TestingMessage2"];
  [parameters_ setObject:@"******** THIS IS NOT A REAL CRASH - IGNORE!! *******************"
    forKey:@"TestingMessage3"];
  [parameters_ setObject:@"******** THIS IS NOT A REAL CRASH - IGNORE!! *******************"
    forKey:@"TestingMessage4"];
#endif

  // generate a unique client ID based on this host's MAC address
  // then add a key/value pair for it
  NSString *clientID = [[NSHost currentHost] obfuscatedMACAddress];
  [parameters_ setObject:clientID forKey:@"guid"];

  close(configFile_);
  configFile_ = -1;

  return YES;
}

//=============================================================================
- (BOOL)readMinidumpData {
  NSString *minidumpDir = [parameters_ objectForKey:@kReporterMinidumpDirectoryKey];
  NSString *minidumpID = [parameters_ objectForKey:@kReporterMinidumpIDKey];

  if (![minidumpID length])
    return NO;

  NSString *path = [minidumpDir stringByAppendingPathComponent:minidumpID];
  path = [path stringByAppendingPathExtension:@"dmp"];

  // check the size of the minidump and limit it to a reasonable size
  // before attempting to load into memory and upload
  const char *fileName = [path fileSystemRepresentation];
  struct stat fileStatus;
  
  BOOL success = YES;

  if (!stat(fileName, &fileStatus)) {
    if (fileStatus.st_size > kMinidumpFileLengthLimit) {
      fprintf(stderr, "GoogleBreakpad Reporter: minidump file too large to upload : %d\n",
        (int)fileStatus.st_size);
      success = NO;
    }
  } else {
      fprintf(stderr, "GoogleBreakpad Reporter: unable to determine minidump file length\n");
      success = NO;
  }

  if (success) {
    minidumpContents_ = [[NSData alloc] initWithContentsOfFile:path];
    success = ([minidumpContents_ length] ? YES : NO);
  }
  
  if (!success) {
    // something wrong with the minidump file -- delete it
    unlink(fileName);
  }
  
  return success;
}

//=============================================================================
- (BOOL)askUserPermissionToSend:(BOOL)shouldSubmitReport {
  // Send without confirmation
  if ([[parameters_ objectForKey:@GOOGLE_BREAKPAD_SKIP_CONFIRM] isEqualToString:@"YES"])
    return YES;

  NSString *display = [parameters_ objectForKey:@GOOGLE_BREAKPAD_PRODUCT_DISPLAY];

  if (![display length])
    display = [parameters_ objectForKey:@GOOGLE_BREAKPAD_PRODUCT];

  NSBundle *bundle = [NSBundle mainBundle];
  NSString *header = [NSString stringWithFormat:
    NSLocalizedStringFromTableInBundle(@"headerFmt", nil, bundle, @""), display];

  // Setup the localized dialog
  NSMutableDictionary *noteDict = [NSMutableDictionary dictionary];
  [noteDict setObject:header
               forKey:(NSString *)kCFUserNotificationAlertHeaderKey];

  // If we're going to submit a report, prompt the user if this is okay.
  // Otherwise, just let them know that the app crashed.
  float timeout = 60.0;  // timeout value for the user notification
  
  if (shouldSubmitReport) {
    [noteDict setObject:NSLocalizedStringFromTableInBundle(@"msg", nil, bundle, @"")
                 forKey:(NSString *)kCFUserNotificationAlertMessageKey];
    [noteDict setObject:NSLocalizedStringFromTableInBundle(@"sendReportButton", nil, bundle, @"")
                 forKey:(NSString *)kCFUserNotificationDefaultButtonTitleKey];
    [noteDict setObject:NSLocalizedStringFromTableInBundle(@"cancelButton", nil, bundle, @"")
                 forKey:(NSString *)kCFUserNotificationAlternateButtonTitleKey];

    // Nominally use the report interval
    timeout = [[parameters_ objectForKey:@GOOGLE_BREAKPAD_REPORT_INTERVAL]
    floatValue];
  } else {
    [noteDict setObject:NSLocalizedStringFromTableInBundle(@"msgNoSend", nil, bundle, @"")
                 forKey:(NSString *)kCFUserNotificationAlertMessageKey];
    [noteDict setObject:NSLocalizedStringFromTableInBundle(@"noSendButton", nil, bundle, @"")
                 forKey:(NSString *)kCFUserNotificationDefaultButtonTitleKey];
    timeout = 60.0;
  }

  // show the notification for at least one minute
  if (timeout < 60.0) {
    timeout = 60.0;
  }
  
  // Show the notification
  CFOptionFlags flags = kCFUserNotificationCautionAlertLevel;
  SInt32 error;
  CFUserNotificationRef note =
    CFUserNotificationCreate(NULL, timeout, flags, &error, (CFDictionaryRef)noteDict);
  CFUserNotificationReceiveResponse(note, timeout, &flags);

  return flags == 0;
}

//=============================================================================
- (BOOL)shouldSubmitReport {
  float interval = [[parameters_ objectForKey:@GOOGLE_BREAKPAD_REPORT_INTERVAL]
    floatValue];
  NSString *program = [parameters_ objectForKey:@GOOGLE_BREAKPAD_PRODUCT];
  NSUserDefaults *ud = [NSUserDefaults standardUserDefaults];
  NSMutableDictionary *programDict =
    [NSMutableDictionary dictionaryWithDictionary:[ud dictionaryForKey:program]];
  NSNumber *lastTimeNum = [programDict objectForKey:kLastSubmission];
  NSTimeInterval lastTime = lastTimeNum ? [lastTimeNum floatValue] : 0;
  NSTimeInterval now = CFAbsoluteTimeGetCurrent();
  NSTimeInterval spanSeconds = (now - lastTime);

  [programDict setObject:[NSNumber numberWithFloat:now] forKey:kLastSubmission];
  [ud setObject:programDict forKey:program];
  [ud synchronize];

  // If we've specified an interval and we're within that time, don't ask the
  // user if we should report
  if (interval > spanSeconds)
    return NO;

  return YES;
}

//=============================================================================
- (void)report {
  NSURL *url = [NSURL URLWithString:[parameters_ objectForKey:@GOOGLE_BREAKPAD_URL]];
  HTTPMultipartUpload *upload = [[HTTPMultipartUpload alloc] initWithURL:url];
  NSMutableDictionary *uploadParameters = [NSMutableDictionary dictionary];

  // Set the known parameters.  This should be kept up to date with the
  // parameters defined in the GoogleBreakpad.h list of parameters.  The intent
  // is so that if there's a parameter that's not in this list, we consider
  // it to be a "user-defined" parameter and we'll upload it to the server.
  NSSet *knownParameters =
    [NSSet setWithObjects:@kReporterMinidumpDirectoryKey,
      @kReporterMinidumpIDKey, @GOOGLE_BREAKPAD_PRODUCT_DISPLAY,
      @GOOGLE_BREAKPAD_PRODUCT, @GOOGLE_BREAKPAD_VERSION, @GOOGLE_BREAKPAD_URL,
      @GOOGLE_BREAKPAD_REPORT_INTERVAL, @GOOGLE_BREAKPAD_SKIP_CONFIRM,
      @GOOGLE_BREAKPAD_SEND_AND_EXIT, nil];

  // Add parameters
  [uploadParameters setObject:[parameters_ objectForKey:@GOOGLE_BREAKPAD_PRODUCT]
                 forKey:@"prod"];
  [uploadParameters setObject:[parameters_ objectForKey:@GOOGLE_BREAKPAD_VERSION]
                 forKey:@"ver"];

  // Add any user parameters
  NSArray *parameterKeys = [parameters_ allKeys];
  int keyCount = [parameterKeys count];
  int i;
  for (i = 0; i < keyCount; ++i) {
    NSString *key = [parameterKeys objectAtIndex:i];
    if (![knownParameters containsObject:key])
      [uploadParameters setObject:[parameters_ objectForKey:key] forKey:key];
  }

  [upload setParameters:uploadParameters];

  // Add minidump file
  [upload addFileContents:minidumpContents_ name:@"upload_file_minidump"];

  // Send it
  NSError *error = nil;
  NSData *data = [upload send:&error];
  NSString *result = [[NSString alloc] initWithData:data
                                           encoding:NSUTF8StringEncoding];
  const char *reportID = "ERR";

  if (error)
    fprintf(stderr, "GoogleBreakpad Reporter: Send Error: %s\n",
            [[error description] UTF8String]);
  else
    reportID = [result UTF8String];

  // rename the minidump file according to the id returned from the server
  NSString *minidumpDir = [parameters_ objectForKey:@kReporterMinidumpDirectoryKey];
  NSString *minidumpID = [parameters_ objectForKey:@kReporterMinidumpIDKey];

  NSString *srcString = [NSString stringWithFormat:@"%@/%@.dmp",
    minidumpDir, minidumpID];
  NSString *destString = [NSString stringWithFormat:@"%@/%s.dmp",
    minidumpDir, reportID];

  const char *src = [srcString fileSystemRepresentation];
  const char *dest = [destString fileSystemRepresentation];
  
  if (rename(src, dest) == 0) {
    fprintf(stderr, "GoogleBreakpad Reporter: Renamed %s to %s after successful upload\n",
            src, dest);
  }
  else {
    // can't rename - don't worry - it's not important for users
    fprintf(stderr, "GoogleBreakpad Reporter: successful upload report ID = %s\n",
            reportID );
  }

  [result release];
  [upload release];
}

//=============================================================================
- (void)dealloc {
  [parameters_ release];
  [minidumpContents_ release];
  [super dealloc];
}
@end

//=============================================================================
int main(int argc, const char *argv[]) {
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  NSLog(@"CrashSender launched.");

  // The expectation is that there will be one argument which is the path
  // to the configuration file
  if (argc != 2) {
    fprintf(stderr, "Missing configuration file\n");
    exit(1);
  }

  // Open the file before (potentially) switching to console user
  int configFile = open(argv[1], O_RDONLY, 0600);
  
  // we want to avoid a build-up of old config files even if they
  // have been incorrectly written by the framework
  unlink(argv[1]); 

  if (configFile == -1) {
    fprintf(stderr, "Unable to open configuration file: %s\n", argv[1]);
    exit(1);
  }

  Reporter *reporter = [[Reporter alloc] initWithConfigurationFD:configFile];

  // Gather the configuration data
  if (![reporter readConfigurationData]) {
    fprintf(stderr, "Unable to load configuration data from %s\n", argv[1]);
    exit(1);
  }

  // Read the minidump into memory before we (potentially) switch from the
  // root user
  if (![reporter readMinidumpData]) {
    fprintf(stderr, "Unable to read minidump data\n");
    exit(1);
  }

  // only submit a report if we have not recently crashed in the past
  BOOL shouldSubmitReport = [reporter shouldSubmitReport];
  BOOL okayToSend = NO;
  
  // ask user if we should send
  if (shouldSubmitReport) {
    okayToSend = [reporter askUserPermissionToSend:shouldSubmitReport];
  }
  
  // TODO(waylonis): What is the behavior when there is no console user?
  // Also, what about the global "Send statistics / metrics"?

  // If we're running as root, switch over to nobody
  if (getuid() == 0 || geteuid() == 0) {
    struct passwd *pw = getpwnam("nobody");

    // If we can't get a non-root uid, don't send the report
    if (!pw)
      exit(0);

    if (setgid(pw->pw_gid) == -1)
      exit(0);

    if (setuid(pw->pw_uid) == -1)
      exit(0);
  }

  if (okayToSend && shouldSubmitReport) {
    [reporter report];
  }

  // Cleanup
  [reporter release];
  [pool release];

  return 0;
}
