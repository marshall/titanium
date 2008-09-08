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
#ifdef WIN32

#include "gears/notifier/user_activity.h"

#include "gears/notifier/system.h"
#include "third_party/glint/include/rectangle.h"

#include <assert.h>
#include <windows.h>

class Win32UserActivityMonitor : public UserActivityMonitor {
 public:
  Win32UserActivityMonitor();
  virtual ~Win32UserActivityMonitor();

 protected:
  virtual UserMode PlatformDetectUserMode();
  virtual uint32 GetMonitorPowerOffTimeSec();
  virtual uint32 GetUserIdleTimeMs();
  virtual bool IsScreensaverRunning();
  virtual bool IsWorkstationLocked() ;
  virtual bool IsFullScreenMode();

 private:
  DISALLOW_EVIL_CONSTRUCTORS(Win32UserActivityMonitor);
};

Win32UserActivityMonitor::Win32UserActivityMonitor() {
}

Win32UserActivityMonitor::~Win32UserActivityMonitor() {
}

UserMode Win32UserActivityMonitor::PlatformDetectUserMode() {
#if WINVER >= 0x0600
  // Try to get the user mode by calling Vista helper function.
  QUERY_USER_NOTIFICATION_STATE state = QUNS_NOT_PRESENT;
  if (SUCCEEDED(QueryUserNotificationState(&state))) {
    switch (state) {
      case QUNS_NOT_PRESENT:
        return USER_AWAY_MODE;
      case QUNS_BUSY:
      case QUNS_RUNNING_D3D_FULL_SCREEN:
      case QUNS_PRESENTATION_MODE:
        return USER_PRESENTATION_MODE;
      case QUNS_ACCEPTS_NOTIFICATIONS:
        return USER_MODE_UNKNOWN;
    }
  }
#endif  // WINVER >= 0x0600
  return USER_MODE_UNKNOWN;
}

uint32 Win32UserActivityMonitor::GetMonitorPowerOffTimeSec() {
  BOOL power_off_enabled = FALSE;
  if (::SystemParametersInfo(SPI_GETPOWEROFFACTIVE, 0, &power_off_enabled, 0) &&
      power_off_enabled) {
    int power_off_sec = 0;
    if (::SystemParametersInfo(SPI_GETPOWEROFFTIMEOUT, 0, &power_off_sec, 0)) {
      return power_off_sec;
    }
  }
  return ULONG_MAX;
}

uint32 Win32UserActivityMonitor::GetUserIdleTimeMs() {
  LASTINPUTINFO last_input_info = {0};
  last_input_info.cbSize = sizeof(LASTINPUTINFO);
  if (::GetLastInputInfo(&last_input_info)) {
    return ::GetTickCount() - last_input_info.dwTime;
  }
  return 0;
}

bool Win32UserActivityMonitor::IsScreensaverRunning() {
  DWORD result = 0;
  if (::SystemParametersInfo(SPI_GETSCREENSAVERRUNNING, 0, &result, 0)) {
    return result != FALSE;
  }
  return false;
}

bool Win32UserActivityMonitor::IsWorkstationLocked() {
  bool is_locked = true;
  HDESK input_desk = ::OpenInputDesktop(0, 0, GENERIC_READ);
  if (input_desk)  {
    wchar_t name[256] = {0};
    DWORD needed = 0;
    if (::GetUserObjectInformation(input_desk,
                                   UOI_NAME,
                                   name,
                                   sizeof(name),
                                   &needed)) {
      is_locked = lstrcmpi(name, L"default") != 0;
    }
    ::CloseDesktop(input_desk);
  }
  return is_locked;
}

bool Win32UserActivityMonitor::IsFullScreenMode() {
  // Check if in full screen window mode.
  // 1) Get the window from any point lies at the main screen where we show
  //    show the notifications.
  // 2) Its window rect is at least as large as the monitor resolution.
  //    Typically for any maximized window the full rect is only as large or
  //    slightly larger than the monitor work area, not the full rect.
  //    Note that this is broken if a maximized window is displayed in either
  //    of the following situation:
  //    * The user hides the taskbar, sidebar and all other top-most bars.
  //    * The user is putting the window in the secondary monitor.
  // 3) Its style should not have WS_DLGFRAME and WS_THICKFRAME;
  //    and its extended style should not have WS_EX_WINDOWEDGE and
  //    WS_EX_TOOLWINDOW.
  glint::Rectangle bounds;
  System::GetMainScreenWorkArea(&bounds);
  POINT point = { bounds.left(), bounds.top() };
  HWND wnd = ::GetAncestor(::WindowFromPoint(point), GA_ROOT);
  if (wnd != ::GetDesktopWindow()) {
    RECT wnd_rc = {0};
    if (::GetWindowRect(wnd, &wnd_rc)) {
      HMONITOR mon = ::MonitorFromRect(&wnd_rc, MONITOR_DEFAULTTONULL);
      if (mon) {
        MONITORINFO mi = { sizeof(mi) };
        ::GetMonitorInfo(mon, &mi);
        ::IntersectRect(&wnd_rc, &wnd_rc, &mi.rcMonitor);
        if (::EqualRect(&wnd_rc, &mi.rcMonitor)) {
          LONG style = ::GetWindowLong(wnd, GWL_STYLE);
          LONG exstyle = ::GetWindowLong(wnd, GWL_EXSTYLE);
          if (!((style & (WS_DLGFRAME | WS_THICKFRAME)) ||
                (exstyle & (WS_EX_WINDOWEDGE | WS_EX_TOOLWINDOW)))) {
            return true;
          }
        }
      }
    }
  }

  // Check if in full screen console mode.
  // We do this by attaching the current process to the console of the
  // foreground window and then checking if it is in full screen mode.
  DWORD pid = 0;
  ::GetWindowThreadProcessId(::GetForegroundWindow(), &pid);
  if (pid) {
    if (::AttachConsole(pid)) {
      DWORD mode_flags = 0;
      ::GetConsoleDisplayMode(&mode_flags);
      ::FreeConsole();
      if (mode_flags & (CONSOLE_FULLSCREEN | CONSOLE_FULLSCREEN_HARDWARE)) {
        return true;
      }
    }
  }

  return false;
}

UserActivityMonitor *UserActivityMonitor::Create() {
  return new Win32UserActivityMonitor();
}

#endif  // WIN32

#endif  // OFFICIAL_BUILD
