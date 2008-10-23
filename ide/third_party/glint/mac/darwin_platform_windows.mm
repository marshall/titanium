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

// Implementation of window/view related parts of Platform interface for Darwin

#import <assert.h>
#import <Cocoa/Cocoa.h>

#import "glint/crossplatform/core_util.h"  // ASSERT
#import "glint/include/rectangle.h"
#import "glint/mac/darwin_platform.h"
#import "glint/mac/glint_bitmap_view.h"
#import "glint/mac/glint_window.h"
#import "glint/mac/glint_work_item_dispatcher.h"

namespace glint_darwin {

using glint::Point;
using glint::Rectangle;
using glint::Size;

// Finds a screen which contains a specified point and
// returns a height of that screen. Used in translation of Glint
// coordinates that have (0,0) at left-top corner into Mac
// system whihc has (0,0) at left-bottom.
void GetDisplayHeightFromPoint(int x, int y, int* height) {
  ASSERT(height);

  NSPoint point = NSMakePoint(x, y);

  // Find the NSScreen that matches this point.
  // If there is no such screen, default to the screen that has menu bar,
  // which is [[NSScreen screens] objectAtIndex:0]
  NSRect bounds = NSZeroRect;

  NSArray* screens = [NSScreen screens];

  if ([screens count] == 0) {
    *height = 0;
    return;
  }
  // Now we know [screens count] > 0.
  for (size_t i = [screens count] - 1; i >= 0 ; --i) {
    NSScreen* screen = [screens objectAtIndex:i];
    bounds = [screen frame];
    if (NSPointInRect(point, bounds)) {
      break;
    }
  }

  *height = static_cast<int>(NSHeight(bounds));
}

GlintWindow* DarwinPlatform::ValidateWindow(PlatformWindow* window) {
  GlintWindow* glint_window = reinterpret_cast<GlintWindow*>(window);
  if (!glint_window || [all_windows_ containsObject:glint_window] == NO) {
    assert(false && "Invalid platform window");
    return nil;
  }
  return glint_window;
}

// Creates invisible (transparent) window. This kind of window does not have
// borders and backgrounds - rather it is presenting the fully-custom UI.
PlatformWindow* DarwinPlatform::CreateInvisibleWindow(RootUI *ui,
                                                      bool topmost,
                                                      bool taskbar) {
  // Our window is initially placed at the bottom left corner, and is tiny.
  // Glint will later on call UpdateInvisibleWindow to give us more information.
  NSRect frame = NSMakeRect(0, 0, 1, 1);
  GlintWindow* window =
      [[[GlintWindow alloc] initWithContentRect:frame
                                        glintUI:ui] autorelease];

  [window setReleasedWhenClosed:NO];

  if (!window)
    return NULL;

  // give the new window a content view
  GlintBitmapView* bitmap_view =
      [[[GlintBitmapView alloc] initWithFrame:frame
                                      glintUI:ui
                                      topmost:topmost] autorelease];
  [window setContentView:bitmap_view];
  [window setInitialFirstResponder:bitmap_view];

  assert([all_windows_ containsObject:window] == NO);
  [all_windows_ addObject:window];

  if (topmost) {
    [window setLevel:NSStatusWindowLevel];
  }

  return reinterpret_cast<PlatformWindow*>(window);
}

// Update invisible window. It may be moved, resized or its bitmap updated
// all at the same time - the actual operation is specified through all the
// arguments.
// screen_origin gives new position of the window
// screen_size is the new size of the window
// bitmap holds pixels to update
// bitmap_window_offset is the offset of (0,0) point of bitmap_area relative
// to (0,0) corner of the window.
// bitmap_area is the rect in the bitmap that will participate in the update
bool DarwinPlatform::UpdateInvisibleWindow(PlatformWindow* window,
                                           Point *screen_origin,
                                           Size screen_size,
                                           PlatformBitmap* bitmap,
                                           Point *bitmap_window_offset,
                                           Rectangle *bitmap_area) {
  // Validate the window and bitmap parameters
  GlintWindow* glint_window = ValidateWindow(window);
  if (!glint_window) {
    return false;
  }

  NSBitmapImageRep* ns_bitmap = reinterpret_cast<NSBitmapImageRep*>(bitmap);
  if (!ns_bitmap || [all_bitmaps_ containsObject:ns_bitmap] == NO) {
    assert(false && "Invalid platform bitmap");
    return false;
  }

  // Get the content view, validate it as well.
  NSView* view = [glint_window contentView];
  if (![view isKindOfClass:[GlintBitmapView class]]) {
    assert(false && "Bad content view.");
    return false;
  }
  GlintBitmapView* glint_view = static_cast<GlintBitmapView*>(view);

  // Get the current content rect of the window, we might change this
  // depending on other parameters passed in.
  NSRect content_frame =
      [glint_window contentRectForFrameRect:[glint_window frame]];

  if (screen_origin) {
    // Need to change the position of the window. The X coordinate is the same
    // as what was passed in, but adjust the Y coordinate to make it bottom-left
    // relative rather than top-left relative. Need to take window size into
    // account as well.
    int height;
    GetDisplayHeightFromPoint(screen_origin->x, screen_origin->y, &height);
    content_frame.origin =
        NSMakePoint(screen_origin->x,
                    ((height - screen_origin->y) - screen_size.height));
  } else {
    // Screen origin didn't change, but we still need to adjust the content
    // origin if size has changed.
    content_frame.origin.y -= (screen_size.height - content_frame.size.height);
  }

  // Adjust the size of the window
  content_frame.size = NSMakeSize(screen_size.width, screen_size.height);

  // Set the view's frame first, with origin 0, 0 relative to window
  NSRect view_frame = NSMakeRect(0, 0, 0, 0);
  view_frame.size = content_frame.size;
  [view setFrame:view_frame];

  // The bitmap goes at 0,0 in the window unless an offset was passed in
  NSPoint offset = NSMakePoint(0, 0);
  if (bitmap_window_offset) {
    offset.x = bitmap_window_offset->x;
    offset.y = bitmap_window_offset->y;
  }

  // We use all of the bitmap unless an area was passed in.
  int width = [ns_bitmap pixelsWide];
  int height = [ns_bitmap pixelsHigh];

  NSRect area = NSMakeRect(0, 0, width, height);
  if (bitmap_area) {
    area = NSMakeRect(bitmap_area->left(), bitmap_area->top(),
                      bitmap_area->size().width, bitmap_area->size().height);
  }

  // Tell the view about this new bitmap. This will also send setNeedsDisplay.
  [glint_view updateViewWithBitmap:ns_bitmap
                            offset:offset
                              area:area];

  // Now set the window's frame as well. This causes immediate redraw.
  NSRect window_frame = [glint_window frameRectForContentRect:content_frame];
  [glint_window setFrame:window_frame
                 display:YES];

  return true;
}

// Attach/detach invisible window to/from the taskbar.
bool DarwinPlatform::AttachInvisibleWindowToTaskbar(
    PlatformWindow *window, bool attach) {
  GlintWindow* glint_window = ValidateWindow(window);
  if (!glint_window) {
    return false;
  }

  // No taskbar on OSX. Noop.
  // TODO(pankaj): Add a Windows menu to the app; then we can add the window to
  // the menu here.
  return true;
}

bool DarwinPlatform::MinimizeWindow(PlatformWindow* window) {
  GlintWindow* glint_window = ValidateWindow(window);
  if (!glint_window) {
    return false;
  }

  [glint_window miniaturize:nil];
  return true;
}

bool DarwinPlatform::HideWindow(PlatformWindow* window) {
  GlintWindow* glint_window = ValidateWindow(window);
  if (!glint_window) {
    return false;
  }

  [glint_window orderOut:nil];
  return true;
}

bool DarwinPlatform::ShowWindow(PlatformWindow* window) {
  GlintWindow* glint_window = ValidateWindow(window);
  if (!glint_window) {
    return false;
  }

  [glint_window deminiaturize:nil];
  return true;
}

bool DarwinPlatform::ShowInteractiveWindow(PlatformWindow* window) {
  GlintWindow* glint_window = ValidateWindow(window);
  if (!glint_window) {
    return false;
  }

  [glint_window makeKeyAndOrderFront:nil];
  return true;
}

void DarwinPlatform::DeleteInvisibleWindow(PlatformWindow *window) {
  GlintWindow* glint_window = ValidateWindow(window);

  // Cancel all work items associated with this window
  NSMutableSet* work_items = [work_items_for_window_ objectForKey:glint_window];
  // We have a callback scheduled on every dispatcher object. It is not
  // possible to cancel the callback, so instead we mark the dispatcher as
  // 'canceled' so when callback is eventually dispatched, it does nothing
  // but releases the dispatcher.
  [work_items makeObjectsPerformSelector:@selector(cancel)];

  // The following removes the work_items set from the dictionary, and causes
  // a release on it, which then removes all items in the set and causes them
  // to be released as well. The items are likely retained by NSApplication
  // (via performSelectorOnMainThread) and will be finally released
  // after dispatching.
  [work_items_for_window_ removeObjectForKey:glint_window];
  
  [glint_window close];
  
  // Remove last reference to glint_window, must be done after close to ensure
  // the window isn't deallocated before close is called.
  [all_windows_ removeObject:glint_window];  
}

void* DarwinPlatform::GetWindowNativeHandle(PlatformWindow *window) {
  GlintWindow* glint_window = ValidateWindow(window);
  return reinterpret_cast<void*>(glint_window);
}

bool DarwinPlatform::SetWindowCaptionText(PlatformWindow *window,
                                          const std::string& text) {
  GlintWindow* glint_window = ValidateWindow(window);
  if (!glint_window) {
    return false;
  }

  // Noop; window doesn't have a title bar
  return true;
}

// Same as ShowInteractiveWindow? dimich says this method is a Window-ism and
// should be removed from the platform interface, but it is still being called
// on OSX.
bool DarwinPlatform::BringWindowToForeground(PlatformWindow *window) {
  return ShowInteractiveWindow(window);
}

bool DarwinPlatform::StartWindowFlash(PlatformWindow *window) {
  GlintWindow* glint_window = ValidateWindow(window);
  if (!glint_window) {
    return false;
  }

  // Even though we validate the window parameter, we don't really "flash"
  // individual windows on OSX. The way to get the user's attention is to
  // bounce the dock icon.

  // Glint depends on the delegates in GlintWindow to tell it whether or not
  // the window has focus, and therefore whether or not to flash the window.
  // This method is only called if we don't already have focus.

  cancel_attention_parameter_ = [NSApp requestUserAttention:NSCriticalRequest];
  return true;
}

bool DarwinPlatform::StopWindowFlash(PlatformWindow* window) {
  GlintWindow* glint_window = ValidateWindow(window);
  if (!glint_window) {
    return false;
  }

  // Same comments as last method
  [NSApp cancelUserAttentionRequest:cancel_attention_parameter_];
  return true;
}

bool DarwinPlatform::SetFocus(PlatformWindow *window) {
  GlintWindow* glint_window = ValidateWindow(window);
  if (!glint_window) {
    return false;
  }

  [glint_window makeKeyWindow];
  return true;
}

}  // namespace glint_darwin
