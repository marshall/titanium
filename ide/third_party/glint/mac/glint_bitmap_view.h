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

// An NSView subclass that renders bitmaps into the view.

#import <Cocoa/Cocoa.h>

// Forward declaration
namespace glint {
class RootUI;
}  // namespace glint

@interface GlintBitmapView : NSView {
  NSMutableArray* bitmapInfos_;
  glint::RootUI* ui_;
  NSTrackingRectTag trackingRectTag_;
  NSTimer* mousePollTimer_;
  BOOL topmost_;
}

// Create a new view with the given rect. The glint ui handles mouse/keyboard
// events, either itself, or by passing them on to WebKit.
- (id)initWithFrame:(NSRect)frameRect
            glintUI:(glint::RootUI*)ui
            topmost:(BOOL)topmost;

// The given bitmap is to be drawn at the given offset in the view's
// coordinates. |area|, in the bitmap's coordinates, is the part of the bitmap
// that'll be used in this update.
- (void)updateViewWithBitmap:(NSBitmapImageRep*)bitmap
                      offset:(NSPoint)offset
                        area:(NSRect)area;

// "topmost" Glint windows get mouse messages and control cursor even if they
//  are inactive. Since they are topmost, the cursor is on top of them.
// Ensures that there is a "mouse tracking rect" set on the view.
// This makes Cocoa send enterMouse and exitMouse messages. Should be called
// every time the view's frame rect changes.
- (void)updateMouseTrackingRect;

- (void)mouseEntered:(NSEvent*)theEvent;
- (void)mouseExited:(NSEvent*)theEvent;

- (void)enableMousePoll;
- (void)disableMousePoll;
- (void)mousePollTimerFired:(NSTimer*)theTimer;

@end
