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

// Definition of Platform interface - the OS abstraction used by Glint
// Normally, Glint does not call anything from OS directly - so there should be
// an OS-specific implementatino of Platform for each OS.

#ifndef GLINT_INCLUDE_PLATFORM_H__
#define GLINT_INCLUDE_PLATFORM_H__

// for va_list, va_start and va_end
#include <stdarg.h>
#include "glint/include/color.h"
#include "glint/include/cursors.h"
#include "glint/include/size.h"
#include "glint/include/types.h"

namespace glint {

class Bitmap;
struct FontDescription;
struct Point;
class Rectangle;
class RootUI;
class WorkItem;

// Opaque types to represent platform resources. They are useful for type
// checks but do not give glint code any visibility into implementation.
// These classes are never defined, since only pointers to them are used.
class PlatformFont;
class PlatformBitmap;
class PlatformWindow;

class Platform {
 public:

  // Timer functions. The Timer should return real64 value that is in seconds.
  // The 0 point does not matter, the only assumption is that the value does
  // not overflow during reasonable life expectancy of the app.
  virtual real64 GetCurrentSeconds() = 0;

  // Memory allocation functions
  virtual void *AllocateMemory(size_t byte_count) = 0;
  virtual void *ReallocateMemory(void *memory_block, size_t byte_count) = 0;
  virtual void FreeMemory(void *memory_block) = 0;
  virtual void CopyMemoryBlock(void *to,
                               const void *from,
                               size_t byte_count) = 0;
  virtual void MoveMemoryBlock(void *to,
                               const void *from,
                               size_t byte_count) = 0;

  // Gets a function pointer given the library name and function name.
  // On Win32, it corresponds to LoadLibrary/GetProcAddress pair.
  virtual void* GetDynamicLibraryFunction(const char* library_name,
                                          const char* function_name) = 0;


  // Creates platform-specific bitmap, in ARGB format
  // it can be simply pre-allocated block of memory or DIBSection
  virtual PlatformBitmap *CreateBitmap(int width, int height) = 0;
  virtual Color *LockPixels(PlatformBitmap *bitmap) = 0;
  virtual void UnlockPixels(PlatformBitmap *bitmap, Color *pixels) = 0;
  virtual void DeleteBitmap(PlatformBitmap *bitmap) = 0;
  // TODO(dimich): remove this, have a lib linked in,
  // have file IO operations instead.

  virtual bool LoadBitmapFromFile(const std::string &file_name,
                                  Bitmap** bitmap) = 0;

  virtual bool ReadFileAsUtf8(const std::string& filename,
                              std::string* content) = 0;

  virtual bool GetResourceByName(const char* name,
                                 void** data,
                                 int* size) = 0;


  // Does the platform support alpha values on the window itself?
  // Without this, the user should not expect to have top level windows
  // that blend with the windows behind them.  (The alpha component will
  // not be respected.)
  virtual bool is_compositing_supported(PlatformWindow* window) const {
    // Platforms that don't always support compositing should override this.
    return true;
  }

  // Creates invisible window. This kind of window does not have
  // borders and backgrounds - rather it is presenting the fully-custom UI.
  // On Windows, the implementation is goign to be a layered window
  virtual PlatformWindow *CreateInvisibleWindow(RootUI *ui,
                                               bool topmost,
                                               bool taskbar) = 0;

  // Update invisible window. It may be moved, resized or its bitmap updated
  // all at the same time - the actual operation is specified as combination
  // of flags.
  // screen_origin gives new position of the window
  // screen_size is the new size of the window
  // bitmap holds pixels to update
  // bitmap_window_offset is the offset of (0,0) point of bitmap_area relative
  // to (0,0) corner of the window.
  // bitmap_area is the rect in the bitmap that will participate in the update
  virtual bool UpdateInvisibleWindow(PlatformWindow *window,
                                     Point *screen_origin,
                                     Size screen_size,
                                     PlatformBitmap *bitmap,
                                     Point *bitmap_window_offset,
                                     Rectangle *bitmap_area) = 0;

  // Removes the window from the screen and leaves "unminimize" OS controls.
  virtual bool MinimizeWindow(PlatformWindow* window) = 0;

  // Removes the window from the screen and from the taskbar/dock.
  virtual bool HideWindow(PlatformWindow* window) = 0;

  // Shows window but does not switch input into it (no focus change).
  virtual bool ShowWindow(PlatformWindow* window) = 0;

  // Shows window and switches input into it.
  virtual bool ShowInteractiveWindow(PlatformWindow* window) = 0;

  virtual void DeleteInvisibleWindow(PlatformWindow *window) = 0;

  // Sometimes, we need to get an OS-specific 'handle', for example
  // HWND on Windows or NSWindow* on Mac to pass through Glint
  // for plugins that are hosted in Glint. Glint itself is
  // platform-independent, so it never needs to know what is behind
  // the handle - thus the type is 'void*'.
  virtual void* GetWindowNativeHandle(PlatformWindow *window) = 0;

  virtual bool SetWindowCaptionText(PlatformWindow *window,
                                    const std::string& text) = 0;

  virtual bool BringWindowToForeground(PlatformWindow *window) = 0;

  virtual bool StartWindowFlash(PlatformWindow *window) = 0;

  virtual bool StopWindowFlash(PlatformWindow* window) = 0;

  virtual bool SetFocus(PlatformWindow *window) = 0;

  virtual void RunMessageLoop() = 0;

  // This will invoke the work item asynchronously on the same thread,
  // preferably with an 'empty' callstack.
  virtual bool PostWorkItem(RootUI *ui, WorkItem *work_item) = 0;

  virtual bool StartMouseCapture(RootUI *ui) = 0;
  virtual void EndMouseCapture() = 0;

  virtual bool SetCursor(Cursors cursor_type) = 0;

  virtual bool SetIcon(PlatformWindow *window, const char* icon_name) = 0;

  // Font and Text functions
  //
  // Fills in FontDescription struct with the info about current default font.
  // Normally, it corresponds to the font used in the default OS message boxes.
  virtual bool GetDefaultFontDescription(FontDescription *font) = 0;

  // Returns a default font.
  // Normally, it corresponds to the font used in the default OS message boxes.
  // Use ReleaseFont to free the internal resources.
  virtual PlatformFont *GetDefaultFont() = 0;

  // Creates a platform-specific font object and returns it as an opaque void
  // pointer. Use ReleaseFont to free the internal resources.
  virtual PlatformFont *CreateFontFromDescription(
    const FontDescription &font) = 0;

  // Releases the font previously created/obtained by GetDefaultFont or
  // CreateFontFromDescription.
  virtual void ReleaseFont(PlatformFont *font) = 0;

  // Measures the simple (single-font) text. Returns a bounding rectangle.
  virtual bool MeasureSimpleText(PlatformFont *platform_font,
                                 const std::string &text,
                                 int wrapping_width,
                                 bool single_line,
                                 bool use_ellipsis,
                                 Rectangle *bounds) = 0;

  // Draws the simple (single-font) text into specified bitmap.
  // Clips the output to specified rectangle.
  virtual bool DrawSimpleText(PlatformFont *platform_font,
                              const std::string &text,
                              Bitmap *target,
                              const Rectangle& clip,
                              Color foreground,
                              bool single_line,
                              bool use_ellipsis) = 0;

  // Diagnostics and debug utility functions
  virtual void CrashWithMessage(const char *format, ...) = 0;

  // Debugging trace.
  void Trace(const char *format, ...) {
#ifdef DEBUG
    va_list args;
    va_start(args, format);
    TraceImplementation(format, args);
    va_end(args);
#endif
    }

  // Traces in both debug and release
  void TraceAlways(const char *format, ...) {
    va_list args;
    va_start(args, format);
    TraceImplementation(format, args);
    va_end(args);
  }

  // Sprintf (apparently, also platform-specific implementation, due to
  // memory/security issues).
  virtual int Sprintf(char *buffer,
                      int buffer_size,
                      const char *format,
                      va_list args) = 0;

  // Actual implementation of platform-dependent debug tracing. It is used by
  // debug-only and release versions of Trace above.
  virtual void TraceImplementation(const char *format, va_list args) = 0;

  // Creates one of derived classes. Implemented by platform-specific code.
  // Caller will never release obtained instance.
  static Platform *Create();
};

// Returns a pointer to a singleton, platform-specific implementation instance.
Platform *platform();

}  // namespace glint

#endif  // GLINT_INCLUDE_PLATFORM_H__
