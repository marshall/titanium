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
//
// Definitions for detecting user activity.

#ifdef OFFICIAL_BUILD
  // The notification API has not been finalized for official builds.
#else
#ifdef OS_MACOSX

#import "gears/notifier/user_activity.h"

#import <Cocoa/Cocoa.h>
#import <IOKit/pwr_mgt/IOPM.h>
#import <IOKit/pwr_mgt/IOPMLib.h>
#import <ScreenSaver/ScreenSaver.h>

#import "gears/base/common/common_osx.h"

// Interesting class descriptions extracted from ScreenSaver.framework using
// class-dump. Note that these are "not documented".
@protocol ScreenSaverControl
- (BOOL)screenSaverIsRunning;
- (BOOL)screenSaverCanRun;
- (void)setScreenSaverCanRun:(BOOL)fp8;
- (void)screenSaverStartNow;
- (void)screenSaverStopNow;
- (void)restartForUser:(id)fp8;
- (double)screenSaverTimeRemaining;
- (void)screenSaverDidFade;
- (BOOL)screenSaverIsRunningInBackground;
- (void)screenSaverDidFadeInBackground:(BOOL)fp8
                                 psnHi:(unsigned int)fp12
                                psnLow:(unsigned int)fp16;
@end

@interface ScreenSaverController : NSObject <ScreenSaverControl>
{
  NSConnection *_connection;
  id _daemonProxy;
  void *_reserved;
}
+ (id)controller;
+ (id)monitor;
+ (id)daemonConnectionName;
+ (id)enginePath;
- (void)_connectionClosed:(id)fp8;
- (id)init;
- (void)dealloc;
- (BOOL)screenSaverIsRunning;
- (BOOL)screenSaverCanRun;
- (void)setScreenSaverCanRun:(BOOL)fp8;
- (void)screenSaverStartNow;
- (void)screenSaverStopNow;
- (void)restartForUser:(id)fp8;
- (double)screenSaverTimeRemaining;
- (void)screenSaverDidFade;
- (BOOL)screenSaverIsRunningInBackground;
- (void)screenSaverDidFadeInBackground:(BOOL)fp8 
                                 psnHi:(unsigned int)fp12
                                psnLow:(unsigned int)fp16;

@end

// end of extraction


class MacUserActivityMonitor : public UserActivityMonitor {
 public:
  MacUserActivityMonitor();
  virtual ~MacUserActivityMonitor();
  
 protected:
  virtual UserMode PlatformDetectUserMode();
  virtual uint32 GetMonitorPowerOffTimeSec();
  virtual uint32 GetUserIdleTimeMs();
  virtual bool IsScreensaverRunning();
  virtual bool IsWorkstationLocked() ;
  virtual bool IsFullScreenMode();

 private:
  static OSStatus HandleAppEvent(EventHandlerCallRef myHandler,
                                 EventRef event,
                                 void *user_data);

  bool in_presentation_mode_;
  EventHandlerRef event_handler_;
  DISALLOW_EVIL_CONSTRUCTORS(MacUserActivityMonitor);
};

MacUserActivityMonitor::MacUserActivityMonitor()
    : in_presentation_mode_(false),
      event_handler_(NULL) {
    
  // Check if the user is in presentation mode initially. We do this by checking
  // if the menu bar is visible.
  NSArray *screens = [NSScreen screens];
  if ([screens count] > 0) {
    NSScreen *screen = [screens objectAtIndex:0];
    NSRect full_bounds = [screen frame];
    NSRect work_bounds = [screen visibleFrame];
    if (NSMaxY(full_bounds) == NSMaxY(work_bounds)) {
      in_presentation_mode_ = true;        
    }
  }

  // Register a Carbon event to receive the notification about the login
  // session's UI mode change.
  EventTypeSpec events[] =
      {{ kEventClassApplication, kEventAppSystemUIModeChanged }};
  OSStatus res = InstallApplicationEventHandler(
      NewEventHandlerUPP(MacUserActivityMonitor::HandleAppEvent),
      GetEventTypeCount(events),
      events,
      this,
      &event_handler_);
  assert(res == noErr);
  (void) res;             // Work around unused variable warning.
}

MacUserActivityMonitor::~MacUserActivityMonitor() {
  if (event_handler_) {
    RemoveEventHandler(event_handler_);
  }
}

OSStatus MacUserActivityMonitor::HandleAppEvent(EventHandlerCallRef myHandler,
                                                EventRef event,
                                                void *user_data) {
  assert(user_data);
  
  MacUserActivityMonitor *this_ptr =
      reinterpret_cast<MacUserActivityMonitor*>(user_data);
    
  UInt32 mode = 0;
  (void) GetEventParameter(event,
                           kEventParamSystemUIMode,
                           typeUInt32,
                           /*outActualType*/ NULL,
                           sizeof(UInt32),
                           /*outActualSize*/ NULL,
                           &mode);
  this_ptr->in_presentation_mode_ = (mode == kUIModeAllHidden);
  return noErr;
}

UserMode MacUserActivityMonitor::PlatformDetectUserMode() {
  // No platform specific detection on OSX - returning USER_MODE_UNKNOWN
  // will make UserActivityMonitor to call "specific" detection methods below.
  return USER_MODE_UNKNOWN;
}

uint32 MacUserActivityMonitor::GetMonitorPowerOffTimeSec() {
  static mach_port_t master_port = 0;
  static io_connect_t power_mgt = 0;

  if (power_mgt == 0) {
    if (master_port == 0) {
      IOMasterPort(MACH_PORT_NULL, &master_port);
      if (master_port == 0) {
        LOG(("Couldn't get master port"));
        return 0;
      }
    }
    power_mgt = IOPMFindPowerManagement(master_port);
    if (power_mgt == 0) {
      LOG(("Couldn't find power management class"));
      return 0;
    }
  }

  size_t dim_time = 0;
  IOReturn ret = IOPMGetAggressiveness(power_mgt, kPMMinutesToDim, &dim_time);
  if (ret != kIOReturnSuccess) {
    LOG(("Couldn't get dim time"));
    return dim_time;
  }

  return dim_time * 60;
}

uint32 MacUserActivityMonitor::GetUserIdleTimeMs() {
  return CGEventSourceSecondsSinceLastEventType(
      kCGEventSourceStateCombinedSessionState, kCGAnyInputEventType) * 1000;
}

bool MacUserActivityMonitor::IsScreensaverRunning() {
  BOOL answer = NO;
  ScreenSaverController *controller = nil;

  // We're calling into an "undocumented" framework here, so we are going to
  // step rather carefully.

  Class screenSaverControllerClass =
      NSClassFromString(@"ScreenSaverController");
  assert(screenSaverControllerClass);
  if ([screenSaverControllerClass respondsToSelector:@selector(controller)]) {
    controller = [ScreenSaverController controller];
    if (controller) {
      if ([controller respondsToSelector:@selector(screenSaverIsRunning)]) {
        answer = [controller screenSaverIsRunning];
      }
    }
  }
  
  if (!controller) {
    // If we can't get the controller, chances are we are being run from the
    // command line and don't have access to the window server. As such we are
    // going to fallback to the older method of figuring out if a screen saver
    // is running.
    
    // Check if the saver is already running
    ProcessSerialNumber psn;
    if (GetFrontProcess(&psn) == noErr) {
      CFDictionaryRef cfProcessInfo = ProcessInformationCopyDictionary(
          &psn, kProcessDictionaryIncludeAllInformationMask);
      NSDictionary *processInfo = [(id)cfProcessInfo autorelease];
      
      NSString *bundlePath = [processInfo objectForKey:@"BundlePath"];

      // ScreenSaverEngine is the frontmost app if the screen saver is actually
      // running Security Agent is the frontmost app if the "enter password"
      // dialog is showing
      answer =
          [[bundlePath lastPathComponent] isEqual:@"ScreenSaverEngine.app"] ||
          [[bundlePath lastPathComponent] isEqual:@"SecurityAgent.app"];
    }
  }
  return answer;
}

bool MacUserActivityMonitor::IsWorkstationLocked() {
  // Same as screensaver running. So we have nothing to do here.
  return false;
}

bool MacUserActivityMonitor::IsFullScreenMode() {
  // Check if the main display has been captured (game in particular).
  if (CGDisplayIsCaptured(CGMainDisplayID())) {
    return true;
  }

  return in_presentation_mode_;
}

UserActivityMonitor *UserActivityMonitor::Create() {
  return new MacUserActivityMonitor();
}

#endif  // OS_MACOSX

#endif  // OFFICIAL_BUILD
