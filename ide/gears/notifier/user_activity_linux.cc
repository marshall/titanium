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
#if defined(LINUX) && !defined(OS_MACOSX)

#include "gears/notifier/user_activity.h"

#include <assert.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <X11/Xatom.h>
#include <X11/Xproto.h>
#include <X11/extensions/dpms.h>
#include <X11/extensions/dpmsstr.h>

#include "gears/base/common/stopwatch.h"
#include "gears/base/common/timed_call.h"

class LinuxUserActivityMonitor : public UserActivityMonitor {
 public:
  LinuxUserActivityMonitor();
  virtual ~LinuxUserActivityMonitor();

 protected:
  virtual UserMode PlatformDetectUserMode();
  virtual uint32 GetMonitorPowerOffTimeSec();
  virtual uint32 GetUserIdleTimeMs();
  virtual bool IsScreensaverRunning();
  virtual bool IsWorkstationLocked() ;
  virtual bool IsFullScreenMode();

 private:
  void RegisterEvents(Window window, bool to_register);
  void InternalRegisterEvents(Window window, bool to_register);
  Window FindTopMostWindow(Window window);
  bool InternalIsScreensaverRunning();
  bool InternalIsFullScreenMode();

  static bool IsWindowVisible(Window window);
  static bool IsWindowFullScreen(Window window);
  static bool IsDesktopWindow(Window window);
  static bool GetWindowProperty(Window window,
                                const char *name,
                                Atom *property_type,
                                int *num_values,
                                uint8 **property_value);
  static GdkFilterReturn FilterEvent(GdkXEvent *xevent,
                                     GdkEvent *event,
                                     gpointer arg);

  int64 last_event_active_time_ms_;
  int64 last_mouse_active_time_ms_;
  GdkScreen *last_mouse_screen_;
  int last_mouse_x_;
  int last_mouse_y_;
  GdkModifierType last_mouse_mask_;
  static LinuxUserActivityMonitor *this_ptr_;
  DISALLOW_EVIL_CONSTRUCTORS(LinuxUserActivityMonitor);
};

LinuxUserActivityMonitor *LinuxUserActivityMonitor::this_ptr_ = NULL;

LinuxUserActivityMonitor::LinuxUserActivityMonitor()
    : last_event_active_time_ms_(0),
      last_mouse_active_time_ms_(0),
      last_mouse_screen_(NULL),
      last_mouse_x_(0),
      last_mouse_y_(0),
      last_mouse_mask_(static_cast<GdkModifierType>(0)) {
  assert(!this_ptr_);
  this_ptr_ = this;
  last_event_active_time_ms_ = last_mouse_active_time_ms_
                             = GetCurrentTimeMillis();

  // Initialize GDK. Multiple initialization is OK.
  gdk_init(0, NULL);

  // Register our event filtering for capturing input activity.
  gdk_window_add_filter(NULL, &LinuxUserActivityMonitor::FilterEvent, this);
  RegisterEvents(DefaultRootWindow(GDK_DISPLAY()), true);
}

LinuxUserActivityMonitor::~LinuxUserActivityMonitor() {
  RegisterEvents(DefaultRootWindow(GDK_DISPLAY()), false);
  gdk_window_remove_filter(NULL, &LinuxUserActivityMonitor::FilterEvent, this);
  this_ptr_ = NULL;
}

bool LinuxUserActivityMonitor::IsWindowVisible(Window window) {
  assert(window != None);

  XWindowAttributes attrs;
  return XGetWindowAttributes(GDK_DISPLAY(), window, &attrs) &&
         attrs.map_state == IsViewable;
}

bool LinuxUserActivityMonitor::IsWindowFullScreen(Window window) {
  assert(window != None);

  // First check if _NET_WM_STATE property contains _NET_WM_STATE_FULLSCREEN.
  Atom fullscreen_atom = XInternAtom(GDK_DISPLAY(),
                                     "_NET_WM_STATE_FULLSCREEN",
                                     true);
  bool is_fullscreen = false;
  Atom type = 0;
  int num_values = 0;
  uint8 *values = NULL;
  if (GetWindowProperty(window, "_NET_WM_STATE", &type, &num_values, &values) &&
      type == XA_ATOM &&
      num_values > 0) {
    Atom *atom_values = reinterpret_cast<Atom*>(values);
    for (int i = 0; i < num_values; ++i) {
      if (atom_values[i] == fullscreen_atom) {
        is_fullscreen = true;
        break;
      }
    }
  }
  if (values) {
    XFree(values);
  }
  if (is_fullscreen) {
    return true;
  }

  // As the last resort, check if the window size is as large as the main
  // screen.
  GdkRectangle rectangle;
  gdk_screen_get_monitor_geometry(gdk_screen_get_default(), 0, &rectangle);

  Window root_window = None;
  int x = 0, y = 0;
  unsigned int width = 0, height = 0, border_width = 0, depth = 0;
  if (!XGetGeometry(GDK_DISPLAY(),
                    window,
                    &root_window,
                    &x,
                    &y,
                    &width,
                    &height,
                    &border_width,
                    &depth)) {
    return false;
  }

  return rectangle.x == x &&
         rectangle.y == y &&
         rectangle.width == static_cast<int>(width) &&
         rectangle.height == static_cast<int>(height);
}

bool LinuxUserActivityMonitor::IsDesktopWindow(Window window) {
  assert(window);

   // Check if _NET_WM_WINDOW_TYPE property contains
   // _NET_WM_WINDOW_TYPE_DESKTOP.
  Atom desktop_atom = XInternAtom(GDK_DISPLAY(),
                                  "_NET_WM_WINDOW_TYPE_DESKTOP",
                                  true);

  bool is_desktop = false;
  Atom type = 0;
  int num_values = 0;
  uint8 *values = NULL;
  if (GetWindowProperty(window, "_NET_WM_WINDOW_TYPE",
                        &type, &num_values, &values) &&
      type == XA_ATOM &&
      num_values == 1 &&
      *(reinterpret_cast<Atom*>(values)) == desktop_atom) {
    is_desktop = true;
  }
  if (values) {
    XFree(values);
  }

  return is_desktop;
}

bool LinuxUserActivityMonitor::GetWindowProperty(Window window,
                                                 const char *name,
                                                 Atom *type,
                                                 int *num_values,
                                                 uint8 **values) {
  assert(name);

  Atom property = XInternAtom(GDK_DISPLAY(), name, true);
  if (property == 0) {
    return false;
  }

  Atom property_type = 0;
  int format = 0;
  unsigned long num_items = 0, bytes_after = 0;
  uint8 *property_values = NULL;
  XGetWindowProperty(GDK_DISPLAY(),
                     window,
                     property,
                     0,
                     512 / 4,
                     false,
                     AnyPropertyType,
                     &property_type,
                     &format,
                     &num_items,
                     &bytes_after,
                     &property_values);

  if (type) {
    *type = property_type;
  }

  if (num_values) {
    *num_values = static_cast<int>(num_items);
  }

  if (values) {
    *values = property_values;
  } else {
    if (property_values) {
      XFree(property_values);
    }
  }

  return property_type != 0;
}

UserMode LinuxUserActivityMonitor::PlatformDetectUserMode() {
  // Not supported.
  return USER_MODE_UNKNOWN;
}

uint32 LinuxUserActivityMonitor::GetMonitorPowerOffTimeSec() {
  // Use Display Power Management Signaling (DPMS) extension.
  int event = 0, error  = 0;
  if (DPMSQueryExtension(GDK_DISPLAY(), &event, &error)) {
    if (DPMSCapable(GDK_DISPLAY())) {
      // Note that a value of zero indicates that this mode has been disabled.
      CARD16 standby_sec = 0, suspend_sec = 0, off_sec = 0;
      if (DPMSGetTimeouts(GDK_DISPLAY(),
                          &standby_sec,
                          &suspend_sec,
                          &off_sec)) {
        if (standby_sec == 0) {
          if (suspend_sec == 0) {
            return off_sec;
          } else {
            return std::min(suspend_sec, off_sec);
          }
        } else if (suspend_sec == 0) {
          if (off_sec == 0) {
            return standby_sec;
          } else {
            return std::min(standby_sec, off_sec);
          }
        } else if (off_sec == 0) {
          return std::min(standby_sec, suspend_sec);
        } else {
          return std::min(std::min(standby_sec, suspend_sec), off_sec);
        }
      }
    }
  }

  return 0;
}

GdkFilterReturn LinuxUserActivityMonitor::FilterEvent(GdkXEvent *xevent,
                                                      GdkEvent *event,
                                                      gpointer arg) {
  assert(arg);

  LinuxUserActivityMonitor *this_ptr =
      reinterpret_cast<LinuxUserActivityMonitor*>(arg);

  XEvent *ev = reinterpret_cast<XEvent*>(xevent);
  switch (ev->xany.type) {
    case KeyPress:
    case KeyRelease:
    case ButtonPress:
    case ButtonRelease:
      this_ptr->last_event_active_time_ms_ = GetCurrentTimeMillis();
      break;
    case CreateNotify:
      this_ptr->RegisterEvents(ev->xcreatewindow.window, true);
      break;
    default:
      break;
  }
  return GDK_FILTER_CONTINUE;
}

void LinuxUserActivityMonitor::RegisterEvents(Window window, bool to_register) {
  // Igonres BadWindow error.
  gdk_error_trap_push();

  InternalRegisterEvents(window, to_register);
  XSync(GDK_DISPLAY(), false);

  // Restores to the original error handler.
  gdk_error_trap_pop();
}

void LinuxUserActivityMonitor::InternalRegisterEvents(Window window,
                                                      bool to_register) {
  // Get all child windows.
  Window root_window = None;
  Window parent_window = None;
  Window *child_windows = NULL;
  uint32 num_children = 0;
  if (!XQueryTree(GDK_DISPLAY(),
                  window,
                  &root_window,
                  &parent_window,
                  &child_windows,
                  &num_children)) {
    return;
  }

  // Register or unregister events.
  // Note that we can't select for ButtonPress because it is said that only one
  // client at a time may select for ButtonPress on a given window, though any
  // number can select for KeyPress.
  XWindowAttributes attrs;
  if (!XGetWindowAttributes(GDK_DISPLAY (), window, &attrs)) {
    return;
  }
  uint32 events = 0;
  if (to_register) {
    events = ((attrs.all_event_masks | attrs.do_not_propagate_mask) &
              (KeyPressMask | KeyReleaseMask | ButtonReleaseMask)) |
             SubstructureNotifyMask;
  } else {
    events = 0;
  }

  XSelectInput(GDK_DISPLAY(), window, events);

  // Go through all child windows.
  if (child_windows) {
    for (uint32 i = 0; i < num_children; ++i) {
      InternalRegisterEvents(child_windows[i], to_register);
    }
    XFree(child_windows);
  }
}

uint32 LinuxUserActivityMonitor::GetUserIdleTimeMs() {
  int64 current_time_ms = GetCurrentTimeMillis();

  // Periodically poll the mouse position since it is said to be more reliable
  // than the way used to detect keyboard.
  GdkDisplay *display = gdk_display_get_default();
  GdkScreen *mouse_screen = NULL;
  int mouse_x = 0;
  int mouse_y = 0;
  GdkModifierType mouse_mask;
  gdk_display_get_pointer(display,
                          &mouse_screen,
                          &mouse_x,
                          &mouse_y,
                          &mouse_mask);
  if (mouse_screen != last_mouse_screen_ ||
      mouse_x != last_mouse_x_ ||
      mouse_y != last_mouse_y_ ||
      mouse_mask != last_mouse_mask_) {
    last_mouse_active_time_ms_ = current_time_ms;
    last_mouse_screen_ = mouse_screen;
    last_mouse_x_ = mouse_x;
    last_mouse_y_ = mouse_y;
    last_mouse_mask_ = mouse_mask;
  }

  uint32 mouse_idle_ms =
      static_cast<uint32>(current_time_ms - last_mouse_active_time_ms_);
  uint32 event_idle_ms =
      static_cast<uint32>(current_time_ms - last_event_active_time_ms_);
  return std::min(mouse_idle_ms, event_idle_ms);
}

bool LinuxUserActivityMonitor::InternalIsScreensaverRunning() {
  // Get and check all windows.
  Window root_window = None;
  Window parent_window = None;
  Window *child_windows = NULL;
  uint32 num_children = 0;
  if (!XQueryTree(GDK_DISPLAY(),
                  DefaultRootWindow(GDK_DISPLAY()),
                  &root_window,
                  &parent_window,
                  &child_windows,
                  &num_children) ||
      num_children == 0) {
    return false;
  }

  Window screensaver_window = None;
  for (int i = static_cast<int>(num_children) - 1; i >= 0; --i) {
    Window current_window = child_windows[i];

    // Check if the window is visible and occupies the full screen.
    // The reason for doing full screen check is that we might have either
    // preferences window or the fullscreen window.
    if (!IsWindowVisible(current_window) ||
        !IsWindowFullScreen(current_window)) {
      continue;
    }

    // For xscreensaver, check if the window has _SCREENSAVER_VERSION property.
    if (GetWindowProperty(current_window, "_SCREENSAVER_VERSION",
                          NULL, NULL, NULL)) {
      screensaver_window = current_window;
      break;
    }

    // For all others, like gnome-screensaver, check if the window's WM_CLASS
    // property contains "screensaver".
    Atom type = 0;
    int num_values = 0;
    uint8 *values = NULL;
    if (GetWindowProperty(current_window, "WM_CLASS",
                          &type, &num_values, &values) &&
        type == XA_STRING &&
        num_values > 0) {
      const char *s = reinterpret_cast<const char*>(values);
      int i = 0;
      while (i < num_values) {
        std::string str(s + i);
        if (str.find("screensaver") != std::string::npos) {
          screensaver_window = current_window;
          break;
        }
        i += str.length() + 1;
      }
    }
    if (values) {
      XFree(values);
    }
    if (screensaver_window != None) {
      break;
    }
  }

  XFree(child_windows);

  return screensaver_window != None;
}

bool LinuxUserActivityMonitor::IsScreensaverRunning() {
  // Igonres BadWindow error.
  gdk_error_trap_push();

  bool is_screensaver_running = InternalIsScreensaverRunning();

  // Restores to the original error handler.
  gdk_error_trap_pop();

  return is_screensaver_running;
}

bool LinuxUserActivityMonitor::IsWorkstationLocked() {
  // Usually screensaver is used to lock the screen. So we have nothing to do
  // here.
  return false;
}

Window LinuxUserActivityMonitor::FindTopMostWindow(Window window) {
  // Get all the child windows. The children are listed in current stacking
  // order, from bottommost (first) to topmost (last).
  Window root_window = None;
  Window parent_window = None;
  Window *child_windows = NULL;
  uint32 num_children = 0;
  if (!XQueryTree(GDK_DISPLAY(),
                  window,
                  &root_window,
                  &parent_window,
                  &child_windows,
                  &num_children)) {
    return None;
  }
  if (!num_children) {
    return None;
  }

  Window topmost_window = None;
  for (int i = static_cast<int>(num_children) - 1; i >= 0; --i) {
    Window current_window = child_windows[i];

    // Skip the invisible window.
    if (!IsWindowVisible(current_window)) {
      continue;
    }

    // Check if the window is managed by the window manager.
    if (GetWindowProperty(current_window, "WM_STATE", NULL, NULL, NULL)) {
      topmost_window = current_window;
      break;
    }

    // Otherwise, try to check its child windows.
    topmost_window = FindTopMostWindow(current_window);
    if (topmost_window) {
      break;
    }
  }

  XFree(child_windows);
  return topmost_window;
}

bool LinuxUserActivityMonitor::InternalIsFullScreenMode() {
  // Find the topmost window.
  Window topmost_window = FindTopMostWindow(DefaultRootWindow(GDK_DISPLAY()));
  if (topmost_window == None) {
    return false;
  }

  // Make sure it is not the desktop window.
  if (IsDesktopWindow(topmost_window)) {
    return false;
  }

  // If it is GDK window, check it using gdk function.
  GdkWindow *gwindow = gdk_window_lookup(topmost_window);
  if (gwindow && topmost_window != GDK_ROOT_WINDOW()) {
    return gdk_window_get_state(gwindow) == GDK_WINDOW_STATE_FULLSCREEN;
  }

  // Check if it is fullscreen using xlib function.
  return IsWindowFullScreen(topmost_window);
}

bool LinuxUserActivityMonitor::IsFullScreenMode() {
  // Igonres BadWindow error.
  gdk_error_trap_push();

  bool is_fullscreen = InternalIsFullScreenMode();

  // Restores to the original error handler.
  gdk_error_trap_pop();

  return is_fullscreen;
}

UserActivityMonitor *UserActivityMonitor::Create() {
  return new LinuxUserActivityMonitor();
}

#endif  // defined(LINUX) && !defined(OS_MACOSX)

#endif  // OFFICIAL_BUILD
