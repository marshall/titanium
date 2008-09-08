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

#import <Cocoa/Cocoa.h>

#import <assert.h>
#import <vector>

#import "glint/mac/glint_window.h"
#import "glint/include/message.h"
#import "glint/include/root_ui.h"

namespace glint {
  class PlatformWindow;
}  // namespace glint

static const NSTimeInterval kTimerIntervalSecs = (NSTimeInterval)0.05;

@implementation GlintWindow

- (id)initWithContentRect:(NSRect)contentRect
                  glintUI:(glint::RootUI*)ui {
  self = [super initWithContentRect:contentRect
                          styleMask:NSBorderlessWindowMask
                            backing:NSBackingStoreBuffered
                              defer:YES];
  if (self) {
    assert(ui != NULL);
    ui_ = ui;

    // We're transparent
    [self setOpaque:NO];

    // We're our own delegate
    [self setDelegate:self];

    // Start a timer for this window; glint/webkit use timer-triggered messages
    // to flash cursors and so on.
    timer_ = [[NSTimer scheduledTimerWithTimeInterval:kTimerIntervalSecs
                                               target:self
                                             selector:@selector(timerFired:)
                                             userInfo:nil
                                              repeats:YES] retain];

    [self setAcceptsMouseMovedEvents:YES];
  }

  // Set up observers for changes in screen resolution, work area etc.
  // This looks for screen configuration/resolution changes.
  [[NSNotificationCenter defaultCenter]
      addObserver:self
         selector:@selector(workAreaDidChange:)
             name:NSApplicationDidChangeScreenParametersNotification
           object:nil];

  // This looks for dock moving around/changing size.
  // "http://www.cocoadev.com/index.pl?DockSize" suggests using the string
  // "com.apple.dock.prefchanged" as ID of a notification sent by the OSX Dock
  // when its settings change. I didn't find documentation for it but it works
  // at least on OSX 10.5.
  [[NSDistributedNotificationCenter defaultCenter]
      addObserver:self
         selector:@selector(workAreaDidChange:)
             name:@"com.apple.dock.prefchanged"
           object:nil];

  return self;
}

- (void)dealloc {
  [[NSNotificationCenter defaultCenter] removeObserver:self];
  [[NSDistributedNotificationCenter defaultCenter] removeObserver:self];

  // timer_ could be nil, but that's OK
  [timer_ invalidate];
  [timer_ release];

  [super dealloc];
}

// Need to override this; the default version returns NO for a borderless
// window
- (BOOL)canBecomeKeyWindow {
  return YES;
}

- (void)close {
  // Don't need the timer to fire anymore
  [timer_ invalidate];
  [timer_ release];
  timer_ = nil;

  // Don't need any delegate now
  [self setDelegate:nil];

  [super close];
}

- (void)sendGlintMessageOfType:(glint::GlintMessages)type {
  glint::Message message;

  message.ui = ui_;
  message.platform_window =
      reinterpret_cast<glint::PlatformWindow*>(self);
  message.code = type;

  ui_->HandleMessage(message);
}

- (void)timerFired:(NSTimer*)timer {
  [self sendGlintMessageOfType:(glint::GL_MSG_IDLE)];
}

// We need this because we're going to use this object as a key in an
// NSDictionary.
- (id)copyWithZone:(NSZone*)zone {
  return [self retain];
}

// Simple hash function, but better than just using the pointer as the hash
- (int)hash {
  return reinterpret_cast<int>([GlintWindow class]);
}

// Delegate methods
- (void)windowDidBecomeKey:(NSNotification*)notification {
  [self sendGlintMessageOfType:(glint::GL_MSG_SETFOCUS)];
}

- (void)windowDidResignKey:(NSNotification*)notification {
  [self sendGlintMessageOfType:(glint::GL_MSG_KILLFOCUS)];
}

// Observer method (registered with NSNotificationCenter).
- (void)workAreaDidChange:(NSNotification*)notification {
  [self sendGlintMessageOfType:(glint::GL_MSG_DISPLAY_SETTINGS_CHANGED)];
}

@end
