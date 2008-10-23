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

// Definition of Platform interface for Darwin

#import <Cocoa/Cocoa.h>

#import "glint/include/array.h"
#import "glint/include/point.h"
#import "glint/include/size.h"
#import "glint/mac/glint_window.h"
#import "glint/mac/glint_work_item_dispatcher.h"
#import "glint/posix/posix.h"

namespace glint_darwin {

using namespace glint;
using glint::Point;
using glint::Size;

class DarwinPlatform : public glint_posix::PosixPlatform {
 public:
   DarwinPlatform();

  // Creates platform-specific bitmap, in a platform-dependent format
  virtual PlatformBitmap* CreateBitmap(int width, int height);
  virtual Color* LockPixels(PlatformBitmap* bitmap);
  virtual void UnlockPixels(PlatformBitmap* bitmap, Color* pixels);
  virtual void DeleteBitmap(PlatformBitmap* bitmap);

  virtual bool LoadBitmapFromFile(const std::string &file_name,
                                  Bitmap** bitmap);

  virtual bool GetResourceByName(const char* name,
                                 void** data,
                                 int* size);

  // Creates a new window. This kind of window does not have
  // borders and backgrounds - rather it is presenting the fully-custom UI.
  virtual PlatformWindow* CreateInvisibleWindow(RootUI* ui,
                                                bool topmost,
                                                bool taskbar);

  // Update invisible window. It may be moved, resized or its bitmap updated
  // all at the same time - the actual operation is specified as combination
  // of flags.
  // screen_origin gives new position of the window
  // screen_size is the new size of the window
  // bitmap holds pixels to update
  // bitmap_window_offset is the offset of (0,0) point of bitmap_area relative
  // to (0,0) corner of the window.
  // bitmap_area is the rect in the bitmap that will participate in the update
  virtual bool UpdateInvisibleWindow(PlatformWindow* window,
                                     Point* screen_origin,
                                     Size screen_size,
                                     PlatformBitmap* bitmap,
                                     Point* bitmap_window_offset,
                                     Rectangle* bitmap_area);

  // Attach/detech invisible window to/from the taskbar.
  virtual bool AttachInvisibleWindowToTaskbar(PlatformWindow* window,
                                              bool attach);

  virtual bool MinimizeWindow(PlatformWindow* window);
  virtual bool HideWindow(PlatformWindow* window);
  virtual bool ShowWindow(PlatformWindow* window);
  virtual bool ShowInteractiveWindow(PlatformWindow* window);

  virtual void DeleteInvisibleWindow(PlatformWindow* window);

  // Sometimes, we need to get an OS-specific 'handle', for example
  // HWND on Windows or NSWindow* on Mac to pass through Glint
  // for plugins that are hosted in Glint. Glint itself is
  // platform-independent, so it never needs to know what is behind
  // the handle - thus the type is 'void*'.
  virtual void* GetWindowNativeHandle(PlatformWindow* window);

  virtual bool SetWindowCaptionText(PlatformWindow* window,
                                    const std::string& text);

  virtual bool BringWindowToForeground(PlatformWindow* window);

  virtual bool StartWindowFlash(PlatformWindow* window);

  virtual bool StopWindowFlash(PlatformWindow* window);

  virtual bool SetFocus(PlatformWindow* window);

  virtual void RunMessageLoop();

  // This will schedule the work item on the same thread, to be run on the
  // thread's run loop.
  virtual bool PostWorkItem(RootUI* ui, WorkItem* work_item);

  virtual bool StartMouseCapture(RootUI* ui);

  virtual void EndMouseCapture();

  virtual bool SetCursor(Cursors cursor_type);

  virtual bool SetIcon(PlatformWindow* window, const char* icon_name);

  // Font and Text functions
  //
  // Fills in FontDescription struct with the info about current default font.
  // Normally, it corresponds to the font used in the default OS message boxes.
  virtual bool GetDefaultFontDescription(FontDescription* font);

  // Returns a default font.
  // Normally, it corresponds to the font used in the default OS message boxes.
  // Use ReleaseFont to free the internal resources.
  virtual PlatformFont* GetDefaultFont();

  // Creates a platform-specific font object and returns it as an opaque void
  // pointer. Use ReleaseFont to free the internal resources.
  virtual PlatformFont* CreateFontFromDescription(
    const FontDescription &font);

  // Releases the font previously created/obtained by GetDefaultFont or
  // CreateFontFromDescription.
  virtual void ReleaseFont(PlatformFont* font);

  // Measures the simple (single-font) text. Returns a bounding rectangle.
  virtual bool MeasureSimpleText(PlatformFont* platform_font,
                                 const std::string &text,
                                 int wrapping_width,
                                 bool single_line,
                                 bool use_ellipsis,
                                 Rectangle* bounds);

  // Draws the simple (single-font) text into specified bitmap.
  // Clips the output to specified rectangle.
  virtual bool DrawSimpleText(PlatformFont* platform_font,
                              const std::string &text,
                              Bitmap* target,
                              const Rectangle& clip,
                              Color foreground,
                              bool single_line,
                              bool use_ellipsis);

  // Diagnostics and debug utility functions
  virtual void CrashWithMessage(const char* format, ...);

  // Actual implementation of platform-dependent debug tracing. It is used by
  // debug-only and release versions of Trace above.
  virtual void TraceImplementation(const char* format, va_list args);

 private:
  // Check whether the passed in pointer is an NSFont we created/returned
  NSFont* ValidateFont(PlatformFont* platform_font);

  // Check whether the passed in pointer is a GlintWindow we created/returned
  GlintWindow* ValidateWindow(PlatformWindow* platform_window);

  // Returns a dictionary with NSAttributedString style attributes that match
  // the arguments.
  NSDictionary* GetAttributeDictionary(PlatformFont* platform_font,
                                       bool single_line,
                                       bool use_ellipsis,
                                       Color* foreground);

  NSMutableSet* all_bitmaps_;
  NSCountedSet* all_fonts_;
  NSMutableSet* all_windows_;
  NSMutableDictionary* work_items_for_window_;

  int normal_weight_;
  int bold_weight_;
  NSCursor* current_cursor_;

  Array<WorkItem> windowless_tasks_;

  int cancel_attention_parameter_;

  DISALLOW_EVIL_CONSTRUCTORS(DarwinPlatform);
};

// Automatically create and delete a bitmap in a given scope.
class scoped_PlatformBitmap {
 public:
  scoped_PlatformBitmap(int width, int height) {
    ns_bitmap_ = reinterpret_cast<NSBitmapImageRep*>(
        platform()->CreateBitmap(width, height));
  }

  ~scoped_PlatformBitmap() {
    platform()->DeleteBitmap(reinterpret_cast<PlatformBitmap*>(ns_bitmap_));
  }

  NSBitmapImageRep* get() {
    return ns_bitmap_;
  }

 private:
  NSBitmapImageRep* ns_bitmap_;

  DISALLOW_EVIL_CONSTRUCTORS(scoped_PlatformBitmap);
};

// Automatically save and restore the graphics context in a given scope.
class AutoRestoreGraphicsContext {
 public:
  AutoRestoreGraphicsContext(NSBitmapImageRep* ns_bitmap);
  ~AutoRestoreGraphicsContext();

 private:
  DISALLOW_EVIL_CONSTRUCTORS(AutoRestoreGraphicsContext);
};

}  // namespace glint_darwin
