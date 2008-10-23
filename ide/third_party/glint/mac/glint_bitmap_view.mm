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

#import <vector>

#import "glint/mac/glint_bitmap_view.h"
#import "glint/include/message.h"
#import "glint/include/platform.h"
#import "glint/include/point.h"
#import "glint/include/root_ui.h"
#import "glint/mac/darwin_platform.h"

using glint::GlintMessages;
using glint::Message;
using glint::RootUI;
using glint::Vector;

namespace glint_darwin {
void GetDisplayHeightFromPoint(int x, int y, int* height);
}  // namespace glint_darwin


// An instance represents a bitmap Glint asked us to draw, at a given offset.
// Only the area represented by area_ within the bitmap is to be drawn, not the
// whole bitmap.
@interface GlintBitmapInfo : NSObject {
  NSBitmapImageRep* bitmap_;
  NSPoint offset_;
  NSRect area_;
}
@end

@implementation GlintBitmapInfo

- (id)initWithBitmap:(NSBitmapImageRep*)bitmap
              offset:(NSPoint)offset
                area:(NSRect)area {
  self = [super init];

  if (self) {
    bitmap_ = [bitmap retain];
    offset_ = offset;
    area_ = area;
  }

  return self;
}

- (void)dealloc {
  [bitmap_ release];
  [super dealloc];
}

- (NSBitmapImageRep*)bitmap {
  return bitmap_;
}

- (NSPoint)offset {
  return offset_;
}

- (NSRect)area {
  return area_;
}

@end

@implementation GlintBitmapView

- (id)initWithFrame:(NSRect)frameRect
            glintUI:(RootUI*)ui
            topmost:(BOOL)topmost {
  self = [super initWithFrame:frameRect];

  if (self) {
    ui_ = ui;
    topmost_ = topmost;
    bitmapInfos_ = [[NSMutableArray alloc] init];
  }

  return self;
}

- (void)dealloc {
  [bitmapInfos_ release];
  [self disableMousePoll];
  [super dealloc];
}

- (void)updateViewWithBitmap:(NSBitmapImageRep*)nsBitmap
                      offset:(NSPoint)offset
                        area:(NSRect)area {
  NSRect frame = [self frame];
  if (NSWidth(area) >= NSWidth(frame) &&
      NSHeight(area) >= NSHeight(frame) &&
      offset.x == 0 &&
      offset.y == 0) {
    // the new bitmap covers the entire area of the view, so throw away the old
    // ones. This is currently the only way Glint invokes us, but that might
    // change.
    [bitmapInfos_ removeAllObjects];
  }

  // Add the new bitmap at the end. When we draw, we'll draw the bitmaps in the
  // same order that they were added here.
  GlintBitmapInfo* info = [[[GlintBitmapInfo alloc] initWithBitmap:nsBitmap
                                                            offset:offset
                                                              area:area]
                           autorelease];
  [bitmapInfos_ addObject:info];

  [self setNeedsDisplay:YES];

  return;
}

- (void)drawRect:(NSRect)rect {
  for (size_t i = 0; i < [bitmapInfos_ count]; i++) {
    GlintBitmapInfo* bitmapInfo =
        static_cast<GlintBitmapInfo*>([bitmapInfos_ objectAtIndex:i]);
    NSBitmapImageRep* bitmap = [bitmapInfo bitmap];
    NSPoint offset = [bitmapInfo offset];
    NSRect area = [bitmapInfo area];

    NSData* tiffRep = [bitmap TIFFRepresentation];
    NSImage* image = [[[NSImage alloc] initWithData:tiffRep] autorelease];

    NSRect imageRect;
    imageRect.origin = offset;
    imageRect.size = [image size];

    NSRect drawRect = NSIntersectionRect(imageRect, rect);

    [image drawInRect:drawRect
             fromRect:area
            operation:NSCompositeCopy
             fraction:1.0];
  }
}

// 3 Helper functions, the first used by the next 2; which are used for handling
// mouse and keyboard events.
- (void)fillInModifiersAndMouseLocationWithEvent:(NSEvent*)event
                                  inGlintMessage:(Message*)message {
  assert(message != NULL);

  message->shift_key_pressed = ([event modifierFlags] & NSShiftKeyMask);
  message->ctrl_key_pressed = ([event modifierFlags] & NSControlKeyMask);
  message->alt_key_pressed = ([event modifierFlags] & NSAlternateKeyMask);

  NSPoint mouse_position = [NSEvent mouseLocation];

  // We need to adjust the mouse position to account for the flipped coordinates
  // on OSX.
  int height;
  glint_darwin::GetDisplayHeightFromPoint(mouse_position.x,
                                          mouse_position.y,
                                          &height);
  glint::Point glint_mouse_position(
      mouse_position.x, height - mouse_position.y);

  message->mouse_position = Vector(glint_mouse_position.x,
                                   glint_mouse_position.y);
  message->screen_mouse_position = message->mouse_position;
}

- (void)handleMouseEvent:(NSEvent*)event
             ofGlintType:(GlintMessages)code {
  assert(ui_ != NULL);

  Message message;

  message.code = code;
  message.ui = ui_;
  [self fillInModifiersAndMouseLocationWithEvent:event
                                  inGlintMessage:&message];

  if (ui_->HandleMessage(message) != glint::MESSAGE_HANDLED) {
    assert(false);
  }
}

- (void)handleKeyEvent:(NSEvent*)event
           ofGlintType:(GlintMessages)code {
  assert(ui_ != NULL);

  NSString* characters = [event characters];

  Message message;
  message.code = code;
  message.ui = ui_;

  NSString* s = [event charactersIgnoringModifiers];
  if ([s length] != 1) {
    return;
  }

  message.character = [characters characterAtIndex:0];
  // TODO(dimich): might need to convert to x-platform virtual codes.
  // For example, WebKit converts to Windows VK_* codes. Might need
  // to define (or borrow) a long enum here...
  message.virtual_key = [event keyCode];

  [self fillInModifiersAndMouseLocationWithEvent:event
                                  inGlintMessage:&message];

  if (ui_->HandleMessage(message) != glint::MESSAGE_HANDLED) {
    assert(false);
  }
}

// Handle various mouse events
- (void)mouseDown:(NSEvent *)event {
  [self handleMouseEvent:event
             ofGlintType:(glint::GL_MSG_LBUTTONDOWN)];
}

- (void)mouseDragged:(NSEvent *)event {
  [self handleMouseEvent:event
             ofGlintType:(glint::GL_MSG_MOUSEMOVE)];
}

- (void)mouseUp:(NSEvent*)event {
  [self handleMouseEvent:event
             ofGlintType:(glint::GL_MSG_LBUTTONUP)];
}

- (void)rightMouseDown:(NSEvent *)event {
  [self handleMouseEvent:event
             ofGlintType:(glint::GL_MSG_RBUTTONDOWN)];
}

- (void)rightMouseDragged:(NSEvent *)event {
  [self handleMouseEvent:event
             ofGlintType:(glint::GL_MSG_MOUSEMOVE)];
}

- (void)rightMouseUp:(NSEvent*)event {
  [self handleMouseEvent:event
             ofGlintType:(glint::GL_MSG_RBUTTONUP)];
}

- (void)mouseMoved:(NSEvent*)event {
  [self handleMouseEvent:event
             ofGlintType:(glint::GL_MSG_MOUSEMOVE)];
}

// TODO(pankaj): Handle "other" mouse events. Do we map all other buttons to the
// middle button?

// Without this override, our view won't get keyboard events
- (BOOL)acceptsFirstResponder {
  return YES;
}

// Makes the first mouse click to be receied as mouse event rather then just
// setting a window as a key window only. Enable only for 'topmost' windows.
- (BOOL)acceptsFirstMouse:(NSEvent*)theEvent {
  return topmost_;
}

// Handle keyboard events
- (void)keyDown:(NSEvent*)event {
  [self handleKeyEvent:event
           ofGlintType:(glint::GL_MSG_KEYDOWN)];
}

- (void)keyUp:(NSEvent*)event {
  [self handleKeyEvent:event
           ofGlintType:(glint::GL_MSG_KEYUP)];
}

- (void)setFrame:(NSRect)frameRect {
  [super setFrame:frameRect];
  if (topmost_) {
    [self updateMouseTrackingRect];
  }
}

- (void)updateMouseTrackingRect {
  assert(topmost_);
  NSRect frameRect = [self frame];

  if (trackingRectTag_) {
    [self removeTrackingRect:trackingRectTag_];
  }

  trackingRectTag_ = [self addTrackingRect:frameRect
                                       owner:self
                                    userData:NULL
                                assumeInside:NO];
}

- (void)mouseEntered:(NSEvent*)theEvent {
  [self enableMousePoll];
  [self handleMouseEvent:nil ofGlintType:glint::GL_MSG_MOUSEMOVE];
}

- (void)mouseExited:(NSEvent*)theEvent {
  [self disableMousePoll];
  [self handleMouseEvent:nil ofGlintType:glint::GL_MSG_MOUSELEAVE];
}

- (void)enableMousePoll {
  // if 'mouseExited' wasn't sent and we are in between mouse timer callbacks,
  // the mouse poll timer may still be alive - remove it in this case
  [self disableMousePoll];
  mousePollTimer_ =
      [[NSTimer scheduledTimerWithTimeInterval:0.1
                                        target:self
                                      selector:@selector(mousePollTimerFired:)
                                      userInfo:nil
                                       repeats:YES] retain];
}

- (void)disableMousePoll {
  [mousePollTimer_ invalidate];
  [mousePollTimer_ release];
  mousePollTimer_ = nil;
}

- (void)mousePollTimerFired:(NSTimer*)theTimer {
  NSPoint mouse_location = [NSEvent mouseLocation];
  BOOL isMouseInWindow = NSPointInRect(mouse_location, [[self window] frame]);
  if (isMouseInWindow) {
    [self handleMouseEvent:nil ofGlintType:glint::GL_MSG_MOUSEMOVE];
  } else {
    // mouseExited: is unreliable - it sometimes is not send if the mouse
    // quickly moves over screen surface. Check the mouse location and simulate
    // mouseExited: if necessary.
    [self mouseExited:nil];
  }
}

@end
