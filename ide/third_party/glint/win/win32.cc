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

// Implementation of Platform interface for Win32

#include <iostream>
#include <map>
#include <sstream>

#undef NOMINMAX
#include <windows.h>
#include <wtypes.h>
#pragma warning(push)
#pragma warning(disable:4061)
#pragma warning(disable:4263)
#include <gdiplus.h>
#pragma warning(pop)

#include "glint/include/array.h"
#include "glint/include/bitmap.h"
#include "glint/include/message.h"
#include "glint/include/node.h"
#include "glint/include/platform.h"
#include "glint/include/root_ui.h"
#include "glint/include/simple_text.h"
#include "glint/include/work_item.h"

using namespace glint;

namespace glint_win32 {

#ifdef DEBUG
#define ASSERT2(expr, message) do { \
  if (!(expr)) CrashWithMessage( \
      "Failed ASSERT '%s' with messaqe '%s' in file '%s', line %d\n", \
      #expr, message, __FILE__, __LINE__); \
} while (0)
#else
#define ASSERT2(expr, message) do {} while (0)
#endif

static const char* kResProtocol = "res://";
static const int kResProtocolLength = 6;  // the length of the previous string
static const char* kGlintResourceType = "GLINT";

typedef struct {
  DWORD win32_message;
  GlintMessages glint_message;
} MessageMap;

// Private messages
static const DWORD WM_GLINT_WORK_ITEM = WM_USER + 153;
static const DWORD WM_SELECT_ALL = WM_GLINT_WORK_ITEM + 1;
static const DWORD WM_CLOSE_DOCUMENT = WM_SELECT_ALL + 1;

// TranslateMessage converts the popular clipboard keystrokes
// to WM_CHAR with wparam encoded like this:
static const int VK_CTRL_A = 'A' & 0x3F;
static const int VK_CTRL_C = 'C' & 0x3F;
static const int VK_CTRL_V = 'V' & 0x3F;
static const int VK_CTRL_W = 'W' & 0x3F;
static const int VK_CTRL_X = 'X' & 0x3F;

static const MessageMap win32_mappings[] = {
  { WM_SELECT_ALL,   GL_MSG_SELECT_ALL },
  { WM_PASTE,   GL_MSG_PASTE },
  { WM_COPY,   GL_MSG_COPY },
  { WM_CUT,   GL_MSG_CUT },
  { WM_KEYUP,   GL_MSG_KEYUP },
  { WM_KEYDOWN, GL_MSG_KEYDOWN },
  { WM_CHAR,    GL_MSG_KEYDOWN },
  { WM_LBUTTONDOWN, GL_MSG_LBUTTONDOWN },
  { WM_LBUTTONUP,   GL_MSG_LBUTTONUP },
  { WM_RBUTTONDOWN, GL_MSG_RBUTTONDOWN },
  { WM_RBUTTONUP,   GL_MSG_RBUTTONUP },
  { WM_MBUTTONDOWN, GL_MSG_MBUTTONDOWN },
  { WM_MBUTTONUP,   GL_MSG_MBUTTONUP },
  { WM_MOUSEMOVE,   GL_MSG_MOUSEMOVE },
  { WM_MOUSELEAVE,  GL_MSG_MOUSELEAVE },
  { WM_CAPTURECHANGED,  GL_MSG_CAPTURELOST },
  { WM_GLINT_WORK_ITEM, GL_MSG_WORK_ITEM },
  { WM_TIMER, GL_MSG_IDLE },
  { WM_SETCURSOR, GL_MSG_SETCURSOR },
  { WM_SETFOCUS, GL_MSG_SETFOCUS },
  { WM_KILLFOCUS, GL_MSG_KILLFOCUS },
  { WM_WINDOWPOSCHANGED, GL_MSG_WINDOW_POSITION_CHANGED },
  { WM_DISPLAYCHANGE, GL_MSG_DISPLAY_SETTINGS_CHANGED },
  // From MSDN documentation for WM_SETTINGCHANGE:
  // "In general, when you receive this message, you should check and
  // reload any system parameter settings that are used by your application."
  // They have various parameters sent as wparam/lparam, but in reality,
  // OS itself does not always follow the rules. For example, XPSP2 sends
  // WM_SETTINGCHANGE with wparam=0 and lparam="Windows" on FontSize change
  // in DisplayProperties/Appearance dialog. So just signal generic
  // GL_MSG_DISPLAY_SETTINGS_CHANGED and cause the app to re-query everything
  // related.
  { WM_SETTINGCHANGE, GL_MSG_DISPLAY_SETTINGS_CHANGED },
  { WM_QUIT,  GL_MSG_QUIT },
  { WM_CLOSE_DOCUMENT, GL_MSG_CLOSE_DOCUMENT },
};

typedef struct {
  Cursors glint_cursor;
  LPWSTR win32_cursor;
} CursorTypes;

static const CursorTypes kCursors[] = {
  { CURSOR_NONE, IDC_NO },
  { CURSOR_POINTER, IDC_ARROW },
  { CURSOR_CROSS, IDC_CROSS },
  { CURSOR_HAND, IDC_HAND },
  { CURSOR_MOVE, IDC_ARROW },
  { CURSOR_IBEAM, IDC_IBEAM },
  { CURSOR_WAIT, IDC_WAIT },
  { CURSOR_HELP, IDC_HELP },
  { CURSOR_EAST_RESIZE, IDC_SIZEWE },
  { CURSOR_NORTH_RESIZE, IDC_SIZENS },
  { CURSOR_NORTH_EAST_RESIZE, IDC_SIZENESW },
  { CURSOR_NORTH_WEST_RESIZE, IDC_SIZENWSE },
  { CURSOR_SOUTH_RESIZE, IDC_SIZENS },
  { CURSOR_SOUTH_EAST_RESIZE, IDC_SIZENWSE },
  { CURSOR_SOUTH_WEST_RESIZE, IDC_SIZENESW },
  { CURSOR_WEST_RESIZE, IDC_SIZEWE },
  { CURSOR_NORTH_SOUTH_RESIZE, IDC_SIZENS },
  { CURSOR_EAST_WEST_RESIZE, IDC_SIZEWE },
  { CURSOR_NORTH_EAST_SOUTH_WEST_RESIZE, IDC_SIZENESW },
  { CURSOR_NORTH_WEST_SOUTH_EAST_RESIZE, IDC_SIZENWSE },
  { CURSOR_COLUMN_RESIZE, IDC_SIZEWE },
  { CURSOR_ROW_RESIZE, IDC_SIZENS },
  { CURSOR_VERTICAL_TEXT, IDC_ARROW },
  { CURSOR_CELL, IDC_ARROW },
  { CURSOR_CONTEXT_MENU, IDC_ARROW },
  { CURSOR_NO_DROP, IDC_ARROW },
  { CURSOR_NOT_ALLOWED, IDC_ARROW },
  { CURSOR_PROGRESS, IDC_ARROW },
  { CURSOR_ALIAS, IDC_ARROW },
  { CURSOR_ZOOMIN, IDC_ARROW },
  { CURSOR_ZOOMOUT, IDC_ARROW },
  { CURSOR_COPY, IDC_ARROW },
};

std::wstring Utf8ToUtf16(const char* utf8_string) {
  std::wstring result;

  if (!utf8_string)
    return result;

  size_t length = strlen(utf8_string);

  if (length == 0) {
    return result;
  }

  int required =
      ::MultiByteToWideChar(CP_UTF8, 0, utf8_string, length, NULL, 0);

  if (required == 0) {
    return result;
  }

  result.resize(required);
  if (!::MultiByteToWideChar(CP_UTF8,
                             0,
                             utf8_string,
                             length,
                             &result.at(0),
                             required)) {
    result.clear();
  }

  return result;
}

std::string Utf16ToUtf8(const wchar_t* wide_string) {
  std::string result;

  if(!wide_string)
    return result;

  size_t length = wcslen(wide_string);
  if (length == 0)
    return result;

  int required = ::WideCharToMultiByte(CP_UTF8, 0, wide_string, length,
                                       NULL, 0, NULL, NULL);
  if (required == 0)
    return result;

  result.resize(required);
  if (!::WideCharToMultiByte(CP_UTF8, 0, wide_string, length,
                             &result.at(0), required, NULL, NULL)) {
    result.clear();
  }

  return result;
}

// If Glint code is packaged into a DLL with resources rather then in a
// single EXE, then resource names are to be resolved in that DLL (at least
// this is more common scenario). Find out the handle to the module that
// contains given executable address and use it for resource loading.
HMODULE GetCurrentModuleHandle() {
  MEMORY_BASIC_INFORMATION mbi = {0};
  DWORD result = ::VirtualQuery(&GetCurrentModuleHandle, &mbi, sizeof(mbi));
  if (result != sizeof(mbi))
    return NULL;
  return reinterpret_cast<HMODULE>(mbi.AllocationBase);
}

class Win32Platform : public Platform {
 public:
  Win32Platform()
      : gdiplus_initialized_(false),
        nesting_count_(0) {
    ::QueryPerformanceFrequency(&timer_frequency_);
    ::QueryPerformanceCounter(&timer_start_);
    window_class_name_ = NULL;
  }

  real64 GetCurrentSeconds() {
    LARGE_INTEGER time_end;
    ::QueryPerformanceCounter(&time_end);
    LONGLONG diff = time_end.QuadPart - timer_start_.QuadPart;
    real64 elapsed = static_cast<real64>(diff) / timer_frequency_.QuadPart;
    return elapsed;
  }

  void* AllocateMemory(size_t byte_count) {
    return ::HeapAlloc(GetProcessHeap(), 0, byte_count);
  }

  void* ReallocateMemory(void* memory_block, size_t byte_count) {
    return ::HeapReAlloc(GetProcessHeap(), 0, memory_block, byte_count);
  }

  void FreeMemory(void* memory_block) {
    ::HeapFree(GetProcessHeap(), 0, memory_block);
  }

  void CopyMemoryBlock(void* to, const void* from, size_t byte_count) {
    ::CopyMemory(to, from, byte_count);
  }

  void MoveMemoryBlock(void* to, const void* from, size_t byte_count) {
    ::MoveMemory(to, from, byte_count);
  }

  void* GetDynamicLibraryFunction(const char* library_name,
                                  const char* function_name) {
    std::wstring wide_library_name = Utf8ToUtf16(library_name);
    HMODULE library_handle = ::LoadLibraryW(wide_library_name.c_str());
    if (library_handle == NULL)
      return NULL;

    return ::GetProcAddress(library_handle, function_name);
  }

  void CrashWithMessage(const char* format, ...) {
    va_list args;
    va_start(args, format);
    TraceImplementation(format, args);
    va_end(args);
    // crash now
    ::DebugBreak();
  }

  int Sprintf(char* buffer,
              int buffer_size,
              const char* format,
              va_list args) {
#if _MSC_VER >= 1400
    // this is Visual C++ 2005
    int bytes_written = ::_vsnprintf_s(
                              reinterpret_cast<char*>(buffer),
                              buffer_size,
                              _TRUNCATE,
                              reinterpret_cast<const char*>(format),
                              args);
#else
    int bytes_written = ::_vsnprintf(
                              reinterpret_cast<char*>(buffer),
                              buffer_size,
                              reinterpret_cast<const char*>(format),
                              args);
#endif
    return bytes_written;
  }

  void TraceImplementation(const char* format, va_list args) {
    char buf[4096];
#if _MSC_VER >= 1400
    // this is Visual C++ 2005
    int i = ::_vsnprintf_s(buf, sizeof(buf), _TRUNCATE, format, args);
#else
    int i = ::_vsnprintf(buf, sizeof(buf), format, args);
#endif


    if (i != -1) {  // out of buffer space
      ::OutputDebugStringA(buf);
      std::cerr << buf;
    } else {
      ::OutputDebugStringA("GLINT DEBUG TRACE FAILURE: trace text is too long");
    }
  }

  // Creates platform-specific bitmap, in ARGB format
  // it can be simply pre-allocated block of memory or DIBSection
  PlatformBitmap* CreateBitmap(int width, int height) {
    Win32Bitmap* bitmap = new Win32Bitmap();
    if (!bitmap)
      return NULL;

    BITMAPINFO bitmap_info;
    int header_size = sizeof(BITMAPINFOHEADER);
    memset(&bitmap_info, 0, header_size);

    BITMAPINFOHEADER &bitmap_info_header = bitmap_info.bmiHeader;

    bitmap_info_header.biSize = header_size;
    bitmap_info_header.biWidth = width;
    bitmap_info_header.biHeight = -(static_cast<int>(height));
    bitmap_info_header.biPlanes = 1;
    bitmap_info_header.biBitCount = 32;
    bitmap_info_header.biCompression = BI_RGB;

    HDC hdc = ::GetDC(NULL);
    bitmap->bitmap_handle = ::CreateDIBSection(hdc,
                              &bitmap_info,
                              DIB_RGB_COLORS,
                              reinterpret_cast<void**>(&(bitmap->pixels)),
                              NULL, 0);
    ::ReleaseDC(NULL, hdc);

    all_bitmaps_[bitmap->bitmap_handle] = bitmap;

    return reinterpret_cast<PlatformBitmap*>(bitmap->bitmap_handle);
  }

  Color* LockPixels(PlatformBitmap* bitmap) {
    HBITMAP bitmap_handle = reinterpret_cast<HBITMAP>(bitmap);
    ASSERT2(all_bitmaps_.count(bitmap_handle) > 0, "Invalid PlatformBitmap");
    BitmapIterator found = all_bitmaps_.find(bitmap_handle);
    if (found == all_bitmaps_.end())
      return NULL;
    Win32Bitmap* win32_bitmap = found->second;
    return win32_bitmap->pixels;
  }

  void UnlockPixels(PlatformBitmap* bitmap, Color* pixels) {
    // noop in Win32
  }

  void DeleteBitmap(PlatformBitmap* bitmap) {
    HBITMAP bitmap_handle = reinterpret_cast<HBITMAP>(bitmap);
    ASSERT2(all_bitmaps_.count(bitmap_handle) > 0, "Invalid PlatformBitmap");

    BitmapIterator found = all_bitmaps_.find(bitmap_handle);
    if (found == all_bitmaps_.end())
      return;
    ASSERT2(found->second->bitmap_handle == bitmap_handle, "Incorrect handle");

    // delete Win32Bitmap struct allocated in CreateBitmap
    delete found->second;
    all_bitmaps_.erase(found);
    ::DeleteObject(bitmap_handle);
  }

  PlatformWindow* CreateInvisibleWindow(RootUI* ui,
                                        bool topmost,
                                        bool taskbar) {
    if (!ui)
      return NULL;

    Win32InvisibleWindow* win32_window = new Win32InvisibleWindow();
    win32_window->tracking_mouse = false;

    EnsureWindowClass();

    DWORD extended_style = WS_EX_LAYERED;
    if (topmost) extended_style |= WS_EX_TOPMOST;
    if (!taskbar) extended_style |= WS_EX_TOOLWINDOW;
    win32_window->is_attached_to_taskbar = taskbar;

    win32_window->window_handle = ::CreateWindowEx(
                                      extended_style,
                                      window_class_name_,
                                      L"glint",
                                      WS_POPUP | WS_SYSMENU | WS_MINIMIZEBOX,
                                      0,       // Initial X
                                      0,       // Initial Y
                                      1,       // Initial width
                                      1,       // Initial height
                                      NULL,
                                      NULL,
                                      NULL,
                                      NULL);

    HMENU sys_menu = ::GetSystemMenu(win32_window->window_handle, false);
    ::RemoveMenu(sys_menu, SC_MAXIMIZE, MF_BYCOMMAND);  // Remove 'Maximize'
    ::RemoveMenu(sys_menu, SC_SIZE, MF_BYCOMMAND);  // Remove 'Size'
    ::RemoveMenu(sys_menu, SC_MOVE, MF_BYCOMMAND);  // Remove 'Move'

    if (win32_window->window_handle) {
      win32_window->ui = ui;
      all_invisible_windows_.Add(win32_window);
      ::ShowWindow(win32_window->window_handle, SW_HIDE);
      ::SetTimer(win32_window->window_handle, 0, 0, NULL);
    } else {
      delete win32_window;
      win32_window = NULL;
    }

    return reinterpret_cast<PlatformWindow*>(win32_window);
  }

  bool UpdateInvisibleWindow(PlatformWindow* window,
    Point* screen_origin,
    Size screen_size,
    PlatformBitmap* bitmap,
    Point* bitmap_window_offset,
    glint::Rectangle* bitmap_area) {

    Win32InvisibleWindow* win32_window = ValidateWindow(window);
    if (!win32_window)
      return false;

    HBITMAP bitmap_handle = reinterpret_cast<HBITMAP>(bitmap);
    ASSERT2(all_bitmaps_.count(bitmap_handle) > 0, "Invalid PlatformBitmap");
    BitmapIterator found = all_bitmaps_.find(bitmap_handle);
    if (found == all_bitmaps_.end())
      return false;

    // Only process full-screen updates for now (check for bitmap_area)
    // TODO(dimich): add partial update logic (go to WM_PAINT and then back to
    // ULW - or try to GetDC/Blit)
    if (bitmap_area == NULL) {
      HWND window_handle = win32_window->window_handle;

      POINT window_position;
      if (screen_origin) {
        window_position.x = screen_origin->x;
        window_position.y = screen_origin->y;
      } else {
        window_position.x = window_position.y = 0;
      }

      SIZE window_size;
      window_size.cx = screen_size.width;
      window_size.cy = screen_size.height;

      POINT bitmap_offset;
      if (bitmap_window_offset) {
        bitmap_offset.x = bitmap_window_offset->x;
        bitmap_offset.y = bitmap_window_offset->y;
      } else {
        bitmap_offset.x = bitmap_offset.y = 0;
      }

      BLENDFUNCTION blend_function = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };

      HDC compatible_dc = ::CreateCompatibleDC(NULL);
      HBITMAP old_bitmap = static_cast<HBITMAP>(::SelectObject(compatible_dc,
                                                               bitmap_handle));

      ::UpdateLayeredWindow(window_handle,
        NULL,
        (screen_origin ? &window_position : NULL),
        &window_size,
        compatible_dc,
        &bitmap_offset,
        RGB(0, 0, 0),
        &blend_function,
        ULW_ALPHA);

      ::SelectObject(compatible_dc, old_bitmap);
      ::DeleteDC(compatible_dc);

      return true;
    }
    return false;
  }

  bool MinimizeWindow(PlatformWindow* window) {
    return AttachInvisibleWindowToTaskbar(window, true) &&
           SetShowWindow(window, SW_SHOWMINNOACTIVE);
  }

  bool HideWindow(PlatformWindow* window) {
    return AttachInvisibleWindowToTaskbar(window, false) &&
           SetShowWindow(window, SW_HIDE);
  }

  bool ShowWindow(PlatformWindow* window) {
    return AttachInvisibleWindowToTaskbar(window, false) &&
           SetShowWindow(window, SW_SHOWNOACTIVATE);
  }

  bool ShowInteractiveWindow(PlatformWindow* window) {
    return AttachInvisibleWindowToTaskbar(window, true) &&
           SetShowWindow(window, SW_SHOWNORMAL);
  }

  void DeleteInvisibleWindow(PlatformWindow* window) {
    Win32InvisibleWindow* win32_window =
        reinterpret_cast<Win32InvisibleWindow*>(window);
    int index = all_invisible_windows_.FindSlowly(win32_window);
    ASSERT2(index >= 0, "Invalid PlatformWindow");
    if (index == -1)
      return;

    DestroyWindow(win32_window->window_handle);
    all_invisible_windows_.RemoveAt(index);
  }

  void* GetWindowNativeHandle(PlatformWindow* window) {
    Win32InvisibleWindow* win32_window = ValidateWindow(window);
    if (!win32_window)
      return NULL;

    return reinterpret_cast<void*>(win32_window->window_handle);
  }

  bool PostWorkItem(RootUI* ui, WorkItem* work_item) {
    if (!ui) {
      windowless_tasks_.Add(work_item);
      return true;
    }

    for (int i = 0; i < all_invisible_windows_.length(); i++) {
      Win32InvisibleWindow* window = all_invisible_windows_[i];
      if (window->ui == ui) {
        ::PostMessage(window->window_handle, WM_GLINT_WORK_ITEM, 0,
                      reinterpret_cast<LPARAM>(work_item));
        return true;
      }
    }
    return false;
  }

  bool StartMouseCapture(RootUI* ui) {
    for (int i = 0; i < all_invisible_windows_.length(); i++) {
      Win32InvisibleWindow* window = all_invisible_windows_[i];
      if (window->ui == ui) {
        ::SetCapture(window->window_handle);
        return true;
      }
    }
    return false;
  }

  void EndMouseCapture() {
    ::ReleaseCapture();
  }

  // Is called by RootUI in response to WM_SETCURSOR
  bool SetCursor(Cursors cursor_type) {
    static CursorTypes current_cursor = { CURSOR_POINTER, IDC_ARROW };
    if (cursor_type != current_cursor.glint_cursor) {
      int cursor_count = sizeof(kCursors) / sizeof(kCursors[0]);
      for (int i = 0; i < cursor_count; ++i) {
        if (cursor_type == kCursors[i].glint_cursor) {
          current_cursor = kCursors[i];
          break;
        }
        if (i + 1 == cursor_count)
          return false;
      }
    }
    HCURSOR cursor_handle = ::LoadCursor(0, current_cursor.win32_cursor);
    ::SetCursor(cursor_handle);
    return true;
  }

  bool SetIcon(PlatformWindow* window, const char* icon_name) {
    Win32InvisibleWindow* win32_window = ValidateWindow(window);
    if (!win32_window)
      return false;

    if (!icon_name)
      return false;

    const wchar_t* wide_icon_name = NULL;
    std::wstring wide_icon_name_str;
    if (IS_INTRESOURCE(icon_name)) {
      wide_icon_name = MAKEINTRESOURCE(reinterpret_cast<WORD>(icon_name));
    } else {
      wide_icon_name_str = Utf8ToUtf16(icon_name);
      wide_icon_name = wide_icon_name_str.c_str();
    }

    int cx = ::GetSystemMetrics(SM_CXICON);
    int cy = ::GetSystemMetrics(SM_CYICON);
    HICON icon = reinterpret_cast<HICON>(::LoadImage(GetCurrentModuleHandle(),
        wide_icon_name, IMAGE_ICON, cx, cy, LR_SHARED | LR_DEFAULTCOLOR));
    if (!icon)
      return false;
    ::SendMessage(win32_window->window_handle, WM_SETICON,
                  ICON_BIG, reinterpret_cast<LPARAM>(icon));

    cx = ::GetSystemMetrics(SM_CXSMICON);
    cy = ::GetSystemMetrics(SM_CYSMICON);
    icon = reinterpret_cast<HICON>(::LoadImage(GetCurrentModuleHandle(),
        wide_icon_name, IMAGE_ICON, cx, cy, LR_SHARED | LR_DEFAULTCOLOR));
    if (!icon)
      return false;

    ::SendMessage(win32_window->window_handle, WM_SETICON,
                  ICON_SMALL, reinterpret_cast<LPARAM>(icon));

    return true;
  }

  bool SetFocus(PlatformWindow* window) {
    Win32InvisibleWindow* win32_window = ValidateWindow(window);
    if (!win32_window)
      return NULL;

    return ::SetFocus(win32_window->window_handle) != NULL;
  }

  void RunMessageLoop() {
    nesting_count_++;
    if (nesting_count_ == 1) {
      EnsureGdiplus();
      if (gdiplus_initialized_) {
        gdiplus_startup_output_.NotificationHook(&gdiplus_hook_token_);
      }
    }

    // Pump it up!
    MSG msg;
    // Main message loop:
    while (::GetMessage(&msg, NULL, 0, 0)) {
      // NOTE! this is important - the webkit looks for WM_CHAR as well
      ::TranslateMessage(&msg);
      ::DispatchMessage(&msg);
    }

    if (nesting_count_ == 1 && gdiplus_initialized_) {
      gdiplus_startup_output_.NotificationUnhook(gdiplus_hook_token_);
      ShutdownGdiplus();
    }
    nesting_count_--;
  }

  bool SetWindowCaptionText(PlatformWindow* window,
                            const std::string& text) {
    Win32InvisibleWindow* win32_window = ValidateWindow(window);
    if (!win32_window)
      return NULL;

    std::wstring text_utf16 = Utf8ToUtf16(text.c_str());
    return TRUE == ::SetWindowText(win32_window->window_handle,
                                   text_utf16.c_str());
  }

  bool BringWindowToForeground(PlatformWindow* window) {
    Win32InvisibleWindow* win32_window = ValidateWindow(window);
    if (!win32_window)
      return NULL;

    return ::SetForegroundWindow(win32_window->window_handle) == TRUE;
  }

  bool StartWindowFlash(PlatformWindow* window) {
    // For some reason, if the FLASHW_TIMER is set, specifying a non-default
    // (=non-zero) flash_duration leads to non-constant flashing behavior.
    return SetWindowFlash(window, FLASHW_TIMER | FLASHW_TRAY, 0, 0);
  }

  bool StopWindowFlash(PlatformWindow* window) {
    return SetWindowFlash(window, FLASHW_STOP, 0, 0);
  }

  bool LoadBitmapFromResource(const std::string& name, Bitmap** result) {
    if (!result)
      return false;

    if (!EnsureGdiplus())
      return false;

    void* data = NULL;
    int size = 0;

    if (!GetResourceByName(name.c_str(), &data, &size))
      return false;

    HGLOBAL buffer = ::GlobalAlloc(GMEM_MOVEABLE, size);
    if (buffer) {
      bool success = false;
      void* buffer_data = ::GlobalLock(buffer);
      if (buffer_data) {
        ::CopyMemory(buffer_data, data, size);

        IStream* stream = NULL;
        Gdiplus::Bitmap* bitmap = NULL;

        if (::CreateStreamOnHGlobal(buffer, FALSE, &stream) == S_OK) {
            bitmap = Gdiplus::Bitmap::FromStream(stream);
            stream->Release();
        }

        if (bitmap && bitmap->GetLastStatus() == Gdiplus::Ok) {
          success = GetBitsFromBitmap(bitmap, result);
        }

        delete bitmap;
        ::GlobalUnlock(buffer);
      }
      ::GlobalFree(buffer);
      return success;
    }
    return false;
  }

  bool LoadBitmapFromFile(const std::string& file_name, Bitmap** result) {
    if (!result)
      return false;

    if (!EnsureGdiplus())
      return false;

    // In case of "res://..", load from resources
    if (file_name.compare(0,
                          kResProtocolLength,
                          kResProtocol,
                          kResProtocolLength) == 0) {
      // strip 'res://'
      std::string resource_name = file_name.substr(kResProtocolLength);
      return LoadBitmapFromResource(resource_name, result);
    }

    std::wstring file_name_utf16 = Utf8ToUtf16(file_name.c_str());
    Gdiplus::Bitmap bitmap(file_name_utf16.c_str());
    return GetBitsFromBitmap(&bitmap, result);
  }

  bool ReadFileAsUtf8(const std::string& filename, std::string* content) {
    if (!content)
      return false;

    std::wstring filename_utf16 = Utf8ToUtf16(filename.c_str());
    HANDLE handle = ::CreateFile(filename_utf16.c_str(),
                                 GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (handle == INVALID_HANDLE_VALUE)
      return false;

    DWORD file_size = ::GetFileSize(handle, NULL);
    if (file_size == INVALID_FILE_SIZE) {
      ::CloseHandle(handle);
      return false;
    }

    // note not "+ 1" since STL string is an STL container, "resize" is in
    // terms of actual elements and trailing \0 is accounted for.
    content->resize(file_size);
    DWORD bytes_read;
    char* buffer = &content->at(0);
    if (!buffer ||
        !::ReadFile(handle, buffer, file_size, &bytes_read, NULL) ||
        file_size != bytes_read) {
      ::CloseHandle(handle);
      return false;
    }

    buffer[file_size] = '\0';
    ::CloseHandle(handle);
    return true;
  }

  bool GetResourceByName(const char* name,
                         void** data,
                         int* size) {
    if (!data || !size || !name)
      return false;

    HMODULE executable = GetCurrentModuleHandle();
    HRSRC resource = ::FindResourceA(executable, name, kGlintResourceType);
    if (!resource)
      return false;

    HANDLE resource_handle = ::LoadResource(executable, resource);
    if (!resource_handle)
      return false;

    *data = ::LockResource(resource_handle);
    if (!*data)
      return false;

    *size = ::SizeofResource(executable, resource);
    return true;
  }

  bool GetDefaultFontDescription(FontDescription* font) {
    if (!font)
      return false;

    NONCLIENTMETRICS metrics;
    if (!GetNonClientMetrics(&metrics))
      return false;

    std::string family_name =
      Utf16ToUtf8(metrics.lfMessageFont.lfFaceName);
    font->family_name = family_name;

    HDC screen_dc = ::GetDC(NULL);
    if (screen_dc == NULL)
      return false;
    font->height = MulDiv(metrics.lfMessageFont.lfHeight,
                          72,
                          ::GetDeviceCaps(screen_dc, LOGPIXELSY));
    if (font->height < 0)
      font->height = -font->height;
    ::ReleaseDC(NULL, screen_dc);

    font->bold = (metrics.lfMessageFont.lfWeight > 400);
    font->italic = (metrics.lfMessageFont.lfItalic == TRUE);
    font->underline = (metrics.lfMessageFont.lfUnderline == TRUE);
    font->strike_out = (metrics.lfMessageFont.lfStrikeOut == TRUE);
    return true;
  }

  PlatformFont* GetDefaultFont() {
    NONCLIENTMETRICS metrics;
    if (!GetNonClientMetrics(&metrics))
      return NULL;

    HFONT font_handle = ::CreateFontIndirect(&metrics.lfMessageFont);
    if (!font_handle)
      return NULL;

    font_handles_[font_handle] = 1;

    return reinterpret_cast<PlatformFont*>(font_handle);
  }

  PlatformFont* CreateFontFromDescription(const FontDescription &font) {
    LOGFONT logfont;
    ::ZeroMemory(&logfont, sizeof(logfont));

    std::wstring name = Utf8ToUtf16(font.family_name.c_str());

#if _MSC_VER >= 1400
    // this is Visual C++ 2005
    wcsncpy_s(logfont.lfFaceName, LF_FACESIZE, name.c_str(), _TRUNCATE);
#else
    wcsncpy(logfont.lfFaceName, name.c_str(), LF_FACESIZE);
#endif

    logfont.lfFaceName[LF_FACESIZE - 1] = 0;

    HDC screen_dc = ::GetDC(NULL);
    if (screen_dc == NULL)
      return false;
    logfont.lfHeight = -MulDiv(font.height,
                               ::GetDeviceCaps(screen_dc, LOGPIXELSY),
                               72);
    ::ReleaseDC(NULL, screen_dc);
    logfont.lfWeight = font.bold ? 700 : 400;
    logfont.lfItalic = font.italic;
    logfont.lfUnderline = font.underline;
    logfont.lfStrikeOut = font.strike_out;
    logfont.lfCharSet = DEFAULT_CHARSET;
    logfont.lfQuality = ANTIALIASED_QUALITY;

    HFONT font_handle = ::CreateFontIndirect(&logfont);
    if (!font_handle)
      return NULL;

    font_handles_[font_handle] = 1;

    return reinterpret_cast<PlatformFont*>(font_handle);
  }

  void ReleaseFont(PlatformFont* font) {
    if (!font)
      return;
    HFONT font_handle = reinterpret_cast<HFONT>(font);
    ASSERT2(font_handles_.count(font_handle) > 0,
            "Invalid PlatformFont");
    if (font_handles_.count(font_handle) <= 0)
      return;
    font_handles_.erase(font_handle);
    ::DeleteObject(font_handle);
  }

  bool MeasureSimpleText(PlatformFont* platform_font,
                         const std::string &text,
                         int wrapping_width,
                         bool single_line,
                         bool use_ellipsis,
                         glint::Rectangle* bounds) {
    if (!platform_font || !bounds)
      return false;

    HFONT font_handle = reinterpret_cast<HFONT>(platform_font);
    ASSERT2(font_handles_.count(font_handle) > 0, "Invalid PlatformFont");
    if (font_handles_.count(font_handle) <= 0)
      return false;

    std::wstring wide_string = Utf8ToUtf16(text.c_str());
    UINT format = DT_CALCRECT;
    if (use_ellipsis)
      format |= DT_END_ELLIPSIS;
    if (single_line)
      format |= DT_SINGLELINE;
    else
      format |= DT_WORDBREAK;

    RECT rect = { 0, 0, wrapping_width, 0 };
    HDC compatible_dc = ::CreateCompatibleDC(NULL);
    HFONT old_font = static_cast<HFONT>(::SelectObject(compatible_dc,
                                                       font_handle));

    int result = DrawText(compatible_dc,
                          wide_string.c_str(),
                          wide_string.length(),
                          &rect,
                          format);
    ::SelectObject(compatible_dc, old_font);
    ::DeleteDC(compatible_dc);

    if (result == 0)
      return false;

    bounds->Set(glint::Rectangle(0, 0, rect.right, rect.bottom));
    return true;
  }

  bool DrawSimpleText(PlatformFont* platform_font,
                      const std::string &text,
                      Bitmap* target,
                      const glint::Rectangle& clip,
                      Color foreground,
                      bool single_line,
                      bool use_ellipsis) {
    if (!platform_font || !target)
      return false;

    if (clip.IsEmpty())
      return true;

    HFONT font_handle = reinterpret_cast<HFONT>(platform_font);
    ASSERT2(font_handles_.count(font_handle) > 0, "Invalid PlatformFont");
    if (font_handles_.count(font_handle) <= 0)
      return false;

    std::wstring wide_string = Utf8ToUtf16(text.c_str());

    UINT format = 0;
    if (use_ellipsis)
      format |= DT_END_ELLIPSIS;
    if (single_line)
      format |= DT_SINGLELINE;
    else
      format |= DT_WORDBREAK;

    int width = clip.size().width;
    int height = clip.size().height;

    ASSERT2(target->size().width == width, "Wrong target size for text.");
    ASSERT2(target->size().height == height, "Wrong target size for text.");

    // Prepare background color to be maximally different from the foreground.
    // Only need that on the red channel - see the post-process comment below.
    Color background = foreground;
    background.set_red(255 - background.red());
    target->Fill(background);

    HBITMAP bitmap_handle = reinterpret_cast<HBITMAP>(
        target->platform_bitmap());
    ASSERT2(all_bitmaps_.count(bitmap_handle) > 0, "Invalid PlatformBitmap");
    BitmapIterator found = all_bitmaps_.find(bitmap_handle);
    if (found == all_bitmaps_.end())
      return false;

    HDC compatible_dc = ::CreateCompatibleDC(NULL);
    if (!compatible_dc)
      return false;

    HFONT old_font = static_cast<HFONT>(::SelectObject(compatible_dc,
                                                       font_handle));
    HBITMAP old_bitmap = static_cast<HBITMAP>(::SelectObject(compatible_dc,
                                                             bitmap_handle));

    ::SetTextColor(compatible_dc, RGB(foreground.red(),
                                      foreground.green(),
                                      foreground.blue()));
    ::SetBkMode(compatible_dc, TRANSPARENT);

    RECT rect = { 0, 0, width, height };
    int result = ::DrawText(compatible_dc,
                            wide_string.c_str(),
                            wide_string.length(),
                            &rect,
                            format);

    ::GdiFlush();
    ::SelectObject(compatible_dc, old_font);
    ::SelectObject(compatible_dc, old_bitmap);
    ::DeleteDC(compatible_dc);

    if (result == 0)
      return false;

    // Post-process the bitmap, replacing solid background with transparent
    // and premultiplying the text color with its own alpha
    // We need to recreate an alpha value computed by the GDI (alpha_gdi) for
    // antialiazed fonts. We do it using equation:
    // result = (1 - alpha_gdi) * background + alpha_gdi * foreground
    // solving for alpha_gdi, we have:
    // alpha_gdi = (background - result) / (background - foreground)
    // for any color channel (we use the red).
    // Since we prepared background red channel so it's as far from foreground
    // as possible, the result of division should be quite precise.
    // Considering background = (255 - foreground) (see the comment above about
    // preparing the background red channel), the expression is:
    // alpha_gdi = (255 - foreground.r - result.r) / (255 - 2 * foreground.r),
    // or alpha_gdi = (C1 - result.r) / C2, where C1 and C2 are:

    int C1 = 255 - foreground.red();
    int C2 = 255 - 2 * foreground.red();

    for (int y = 0; y < height; ++y) {
        Color* scanline = target->GetPixelAt(0, y);
        for (int x = 0; x < width; ++x) {
          Color pixel = *scanline;
          if (pixel.alpha() == 0xFF) {
            scanline->SetARGB(0x00000000);
          } else {
            int alpha_gdi = 255 * (C1 - pixel.red()) / C2;
            pixel = foreground;
            pixel.set_alpha((pixel.alpha() * alpha_gdi) >> 8);
            pixel.Premultiply();
            *scanline = pixel;
          }
          ++scanline;
        }
    }

    return true;
  }

 private:
  struct Win32InvisibleWindow {
    Win32InvisibleWindow()
      : window_handle(NULL),
        ui(NULL),
        tracking_mouse(false),
        is_attached_to_taskbar(false) {
    }

    HWND window_handle;
    RootUI* ui;
    bool tracking_mouse;
    bool is_attached_to_taskbar;
  };

  // Basically, performs a cast. Also, checks if the passed window actually
  // is a valid pointer to one of our windows.
  Win32InvisibleWindow* ValidateWindow(PlatformWindow* window) {
    Win32InvisibleWindow* win32_window =
        reinterpret_cast<Win32InvisibleWindow*>(window);
    int index = all_invisible_windows_.FindSlowly(win32_window);
    ASSERT2(index >= 0, "Invalid PlatformWindow");
    return (index == -1 ? NULL : win32_window);
  }

  LONG GetExtendedStyle(HWND window_handle) {
    return ::GetWindowLong(window_handle, GWL_EXSTYLE);
  }

  bool IsWindowOnTaskbar(HWND window_handle) {
    LONG extended_style = GetExtendedStyle(window_handle);
    if (!extended_style) {
      return true;
    } else {
      return !(extended_style & WS_EX_TOOLWINDOW);
    }
  }

  bool AttachInvisibleWindowToTaskbar(PlatformWindow* window, bool attach) {
    Win32InvisibleWindow* win32_window = ValidateWindow(window);
    if (!win32_window)
      return false;

    if (attach == win32_window->is_attached_to_taskbar) {
      return true;
    }
    win32_window->is_attached_to_taskbar = attach;

    HWND window_handle = win32_window->window_handle;
    LONG extended_style = GetExtendedStyle(window_handle);
    if (!extended_style) {
      return false;
    }
    if (attach) {
      extended_style &= ~WS_EX_TOOLWINDOW;
    } else {
      extended_style |= WS_EX_TOOLWINDOW;
    }

    // Update the taskbar by hiding and showing the window - doesn't
    // happen with only SetWindowLong...
    ::ShowWindow(window_handle, SW_HIDE);
    if (!::SetWindowLong(window_handle, GWL_EXSTYLE, extended_style)) {
      return false;
    }
    ::ShowWindow(window_handle, SW_SHOWNA);
    return true;
  }

  bool SetShowWindow(PlatformWindow* window, int show_command) {
    Win32InvisibleWindow* win32_window = ValidateWindow(window);

    if (win32_window) {
      return TRUE == ::ShowWindow(win32_window->window_handle, show_command);
    } else {
      return false;
    }
  }

  // Validates the window before setting the window's flash behavior.
  bool SetWindowFlash(PlatformWindow* window,
                      int flash_flags,
                      int number_of_flashes,
                      int flash_duration) {
    Win32InvisibleWindow* win32_window = ValidateWindow(window);
    return SetWindowFlash(win32_window, flash_flags, number_of_flashes,
        flash_duration);
  }

  // Sets the flash behavior of an already-validated window.
  bool SetWindowFlash(Win32InvisibleWindow* win32_window,
                      int flash_flags,
                      int number_of_flashes,
                      int flash_duration) {
    if (!win32_window) {
      return false;
    }

    ::FLASHWINFO flash_info;
    flash_info.cbSize = sizeof(flash_info);
    flash_info.hwnd = win32_window->window_handle;
    flash_info.dwFlags = flash_flags;
    flash_info.uCount = number_of_flashes;  // number of flashes
    flash_info.dwTimeout = flash_duration;  // in milliseconds

    // Non-zero return value means that the function succeeded in changing the
    // flashing state of the window.
    return (::FlashWindowEx(&flash_info) != 0);
  }

  // Helper to wrap header discrepancy between XP and Vista. More info:
  // http://forums.microsoft.com/MSDN/ShowPost.aspx?PostID=930256&SiteID=1
  bool GetNonClientMetrics(NONCLIENTMETRICS* metrics) {
    int metrics_size = sizeof(*metrics);
    if ((::GetVersion() & 0xFF) == 5) {  // Win XP
      metrics_size = 500;  // this is what it is in WinXP headers.
    }

    metrics->cbSize = metrics_size;
    if (!::SystemParametersInfo(SPI_GETNONCLIENTMETRICS,
                                metrics_size,
                                metrics,
                                0)) {
      ASSERT2(false, "NONCLIENTMETRICS mismatch");
      return false;
    }

    return true;
  }

  // TODO(dimich): perhaps need to call Gdiplus::GdiplusShutdown(token);
  // Currently the platform services do not have determined lifetime,
  // need to define proper shutdown sequence with the core and
  // implement this (and some resource tracking for unittests).

  bool EnsureGdiplus() {
    if (!gdiplus_initialized_) {
      Gdiplus::GdiplusStartupInput input;
      input.SuppressBackgroundThread  = true;
      Gdiplus::Status status = Gdiplus::GdiplusStartup(
          &gdiplus_token_, &input, &gdiplus_startup_output_);

      if (status != Gdiplus::Ok)
        return false;

      gdiplus_initialized_ = true;
    }
    return true;
  }

  void ShutdownGdiplus() {
    if (gdiplus_initialized_) {
      Gdiplus::GdiplusShutdown(gdiplus_token_);
      gdiplus_initialized_ = false;
    }
  }

  bool GetBitsFromBitmap(Gdiplus::Bitmap* bitmap, Bitmap** result) {
    if (!bitmap || !result)
      return false;

    Gdiplus::BitmapData bitmap_data;
    Gdiplus::Rect rect(0, 0, bitmap->GetWidth(), bitmap->GetHeight());

    if (Gdiplus::Ok != bitmap->LockBits(&rect,
                                        Gdiplus::ImageLockModeRead,
                                        PixelFormat32bppARGB,
                                        &bitmap_data))
      return false;

    ASSERT2(bitmap_data.Stride == bitmap_data.Width * 4, "Unexpected Stride");

    *result = new Bitmap(Point(), Size(bitmap_data.Width, bitmap_data.Height));

    ::CopyMemory((*result)->GetPixelAt(0, 0),
                 bitmap_data.Scan0,
                 bitmap_data.Width * bitmap_data.Height * 4);

    (*result)->PremultiplyAlpha();

    bitmap->UnlockBits(&bitmap_data);
    return true;
  }

  void FillMouseLocation(Message* message) {
    DWORD location;
    location = ::GetMessagePos();
    int x = static_cast<short int>(location & 0xFFFF);
    int y = static_cast<short int>(location >> 16);
    message->mouse_position = Vector(static_cast<real32>(x),
                                     static_cast<real32>(y));
    // store a copy of screen position. The mouse_position will
    // transform to local space as the message traverses the node tree.
    message->screen_mouse_position = message->mouse_position;
  }

  // Called during WM_CHAR
  void CheckForSpecialCommands(HWND window_handle,
                               WPARAM wparam,
                               bool shift,
                               bool ctrl) {
    int message_code = 0;

    if (shift) {
      switch (wparam) {
        case VK_INSERT:
          message_code = WM_PASTE;
          break;
        case VK_DELETE:
          message_code = WM_CUT;
        default:
          break;
      }
    } else if (ctrl) {
      switch (wparam) {
        case VK_CTRL_A:
          message_code = WM_SELECT_ALL;
          break;
        case VK_CTRL_C:
          message_code = WM_COPY;
          break;
        case VK_CTRL_V:
          message_code = WM_PASTE;
          break;
        case VK_CTRL_X:
          message_code = WM_CUT;
          break;
        case VK_CTRL_W:
          message_code = WM_CLOSE_DOCUMENT;
          break;
        default:
          break;
      }
    }
    if (message_code) {
      ::PostMessage(window_handle, message_code, 0, 0);
    }
  }

  bool MapMessage(HWND window_handle,
                  UINT message_code,
                  WPARAM wparam,
                  LPARAM lparam,
                  Message* message) {
    bool found = false;
    for (int i = 0; i < sizeof(win32_mappings)/sizeof(MessageMap); i++) {
      const MessageMap* message_map = &(win32_mappings[i]);
      if (message_map->win32_message == message_code) {
        message->code = message_map->glint_message;
        found = true;
        break;
      }
    }
    if (!found)
      return false;

    // Do additional processing for some messages.
    switch (message_code) {
      case WM_KEYDOWN:
      case WM_KEYUP:
      case WM_CHAR:
        message->shift_key_pressed = ::GetKeyState(VK_SHIFT) < 0;
        message->ctrl_key_pressed = ::GetKeyState(VK_CONTROL) < 0;
        message->alt_key_pressed = ::GetKeyState(VK_MENU) < 0;
        FillMouseLocation(message);
        if (message_code == WM_CHAR) {
          message->character = wparam;
          CheckForSpecialCommands(window_handle,
                                  wparam,
                                  message->shift_key_pressed,
                                  message->ctrl_key_pressed);
        } else {
          message->virtual_key = wparam;
          if (message_code == WM_KEYUP) {
            // Handle keys that aren't translated into WM_CHAR
            if (message->ctrl_key_pressed && wparam == VK_F4) {
              ::PostMessage(window_handle, WM_CLOSE_DOCUMENT, 0, 0);
            }
          }
        }
        break;

      case WM_LBUTTONDOWN:
      case WM_LBUTTONUP:
      case WM_RBUTTONDOWN:
      case WM_RBUTTONUP:
      case WM_MBUTTONDOWN:
      case WM_MBUTTONUP:
      case WM_MOUSEMOVE: {
        message->shift_key_pressed = (wparam & MK_SHIFT) != 0;
        message->ctrl_key_pressed = (wparam & MK_CONTROL) != 0;
        message->alt_key_pressed = ::GetKeyState(VK_MENU) < 0;
        message->mouse_button_down = (wparam & MK_LBUTTON) != 0;
        FillMouseLocation(message);
        break;
      }

      case WM_CAPTURECHANGED:
        // Don't send GL_MSG_CAPTURELOST if it's our window that gains capture.
        return message->platform_window != reinterpret_cast<void*>(lparam);

      case WM_TIMER:
        // Take a chance to process some of those "windowless" work items.
        if (windowless_tasks_.length() > 0) {
          // First remove the task from the list to avoid infinite loop
          // if one of tasks causes message pumping and we get another WM_TIMER.
          WorkItem* item = windowless_tasks_.EraseAt(0);
          if (item) {
            item->Run();
            delete item;
          }
        }
        break;

      case WM_GLINT_WORK_ITEM:
        message->work_item = reinterpret_cast<WorkItem*>(lparam);
        break;

      case WM_SETTINGCHANGE:
      case WM_DISPLAYCHANGE: {
        RECT rect = { 0, 0, 0, 0 };
        ::GetWindowRect(window_handle, &rect);
        message->window_position = Point(rect.left, rect.top);
        break;
      }

      case WM_WINDOWPOSCHANGED: {
        WINDOWPOS* window_position = reinterpret_cast<WINDOWPOS*>(lparam);
        if (!window_position ||
            (window_position->flags & SWP_NOMOVE) != 0 ||
            ::IsIconic(window_handle)) {
          return false;
        }
        message->window_position =
            Point(window_position->x, window_position->y);
        break;
      }

      default:
        break;
    }
    return true;
  }

  static LRESULT CALLBACK WindowProc(HWND window_handle,
                                     UINT message_code,
                                     WPARAM wparam,
                                     LPARAM lparam) {
    switch (message_code) {
      case WM_DESTROY:
        return 0;
      case WM_ERASEBKGND: {
        return 1;
      }
    }

    // Check if it is a known message for a glint window.
    // If yes, map it into glint format and return for processing.
    Message message;
    Win32Platform* win32_platform = static_cast<Win32Platform*>(platform());
    if (win32_platform) {
      for (int i = 0;
           i < win32_platform->all_invisible_windows_.length();
           i++) {
        Win32InvisibleWindow* window =
            win32_platform->all_invisible_windows_[i];
        if (window->window_handle == window_handle &&
            window->ui != NULL) {
          if (message_code == WM_MOUSEMOVE && !window->tracking_mouse) {
            ::TRACKMOUSEEVENT info = { 0 };
            info.cbSize = sizeof(info);
            info.dwFlags = TME_LEAVE;
            info.hwndTrack = window->window_handle;
            ::TrackMouseEvent(&info);
            window->tracking_mouse = true;
          } else if (message_code == WM_MOUSELEAVE) {
            window->tracking_mouse = false;
          } else if (message_code == WM_SYSCOMMAND) {
            // Respond to requests to restore, minimize, close
            if (!win32_platform->IsWindowOnTaskbar(window_handle))
              return 0;
            switch (wparam) {
              case SC_CLOSE:
                window->ui->InteractiveClose();
                // It seems that DefWindowProc will clear the GWL_EXSTYLE in
                // response to the SC_CLOSE if we do not return early.
                return 0;
              case SC_MINIMIZE:
                window->ui->Minimize();
                break;
              case SC_RESTORE:
                window->ui->ShowInteractive();
                break;
              default:
                break;
            }
          }
          message.ui = window->ui;
          message.platform_window = window;
          if (win32_platform->MapMessage(window_handle,
                                         message_code,
                                         wparam,
                                         lparam,
                                         &message)) {
            if (window->ui->HandleMessage(message) == MESSAGE_HANDLED)
              return 1;
          }
        }
      }
    }

    return ::DefWindowProc(window_handle, message_code, wparam, lparam);
  }

  void EnsureWindowClass() {
    if (!window_class_name_) {
      WNDCLASSEX window_class;

      window_class.cbSize = sizeof(WNDCLASSEX);
      window_class.style = 0;
      window_class.lpfnWndProc = WindowProc;
      window_class.cbClsExtra = 0;
      window_class.cbWndExtra = 0;
      window_class.hInstance = NULL;
      window_class.hIcon = NULL;
      window_class.hCursor = NULL;
      window_class.hbrBackground = NULL;
      window_class.lpszMenuName = NULL;
      window_class.lpszClassName = L"Glint Invisible Window";
      window_class.hIconSm = NULL;

      if (::RegisterClassEx(&window_class)) {
        window_class_name_ = window_class.lpszClassName;
      }
    }
  }

  struct Win32Bitmap {
    HBITMAP bitmap_handle;
    Color* pixels;
  };

  const WCHAR* window_class_name_;
  LARGE_INTEGER timer_frequency_;
  LARGE_INTEGER timer_start_;

  std::map<const HBITMAP, Win32Bitmap*> all_bitmaps_;
  typedef std::map<const HBITMAP, Win32Bitmap*>::iterator BitmapIterator;

  std::map<const HFONT, int> font_handles_;

  Array<Win32InvisibleWindow> all_invisible_windows_;

  Array<WorkItem> windowless_tasks_;

  bool gdiplus_initialized_;
  Gdiplus::GdiplusStartupOutput gdiplus_startup_output_;
  DWORD gdiplus_token_;
  ULONG_PTR gdiplus_hook_token_;
  int nesting_count_;

  DISALLOW_EVIL_CONSTRUCTORS(Win32Platform);
};

}  // namespace glint_win32

namespace glint {

Platform* Platform::Create() {
  return new glint_win32::Win32Platform();
}

}  // namespace glint
