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

#ifdef WIN32

#ifdef OFFICIAL_BUILD

// The notification API has not been finalized for official builds.

#include <windows.h>

extern "C" {

__declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE, DWORD, LPVOID) {
  return TRUE;
}

__declspec(dllexport) int WINAPI DllEntry(const wchar_t *) {
  return 0;
}

}  // extern "C"

#else

#include "gears/notifier/notifier.h"

#ifdef OFFICIAL_BUILD
#include "gears/base/common/exception_handler.h"
#endif  // OFFICIAL_BUILD
#include "gears/base/common/file.h"
#include "gears/base/common/paths.h"
#ifdef OFFICIAL_BUILD
#include "gears/base/common/trace_buffers_win32/trace_buffers_win32.h"
#endif  // OFFICIAL_BUILD
#include "gears/notifier/const_notifier.h"
#include "gears/notifier/notifier_sync_win32.h"
#include "gears/notifier/notifier_utils_win32.h"

// Work around the header including errors.
#include <atlbase.h>
#include <atlwin.h>
#include <objbase.h>
#include <windows.h>

// Constants
static const char16 *kSingleInstanceMutexId =
    L"Local\\929B4371-2230-49b2-819F-6A34D5A11DED";
static const int kWaitProcessExitTimeoutMs = 5000;    // 5s

// Dummy window
class NotifierDummyWindow :
    public CWindowImpl<NotifierDummyWindow> {
 public:
  DECLARE_WND_CLASS(kNotifierDummyWndClassName);

  NotifierDummyWindow() : CWindowImpl() {}

  ~NotifierDummyWindow() {
    if (m_hWnd) {
      DestroyWindow();
    }
  }

  bool NotifierDummyWindow::Initialize() {
    return Create(NULL, 0, L"GearsNotifier", WS_POPUP) != NULL;
  }

  BEGIN_MSG_MAP(NotifierDummyWindow)
    MESSAGE_HANDLER(WM_CREATE, OnCreate)
    MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
  END_MSG_MAP()

  LRESULT OnCreate(UINT msg, WPARAM wparam, LPARAM lparam, BOOL& handled) {
    return 0;
  }

  LRESULT OnDestroy(UINT msg, WPARAM wparam, LPARAM lparam, BOOL& handled) {
    ::PostQuitMessage(0);
    return 0;
  }
};

class Win32Notifier : public Notifier {
 public:
  Win32Notifier(const wchar_t *running_version);
  virtual bool Initialize();
  virtual int Run();
  virtual void Terminate();
  virtual void RequestQuit();

  virtual bool IsRestartNeeded() const;

  void WaitParentProcessExitOnRestart();

 protected:
  virtual bool RegisterProcess();
  virtual bool UnregisterProcess();

 private:
  bool CheckAlreadyRunning(bool *already_running);

  std::string16 running_version_;
  HANDLE single_instance_mutex_handle_;
  scoped_ptr<NotifierDummyWindow> dummy_wnd_;
  scoped_ptr<NotifierSyncGate> sync_gate_;
  DISALLOW_EVIL_CONSTRUCTORS(Win32Notifier);
};

Win32Notifier::Win32Notifier(const wchar_t *running_version)
    : running_version_(running_version),
      single_instance_mutex_handle_(NULL) {
}

bool Win32Notifier::CheckAlreadyRunning(bool *already_running) {
  assert(already_running);

  single_instance_mutex_handle_ = ::CreateMutex(NULL,
                                                false,
                                                kSingleInstanceMutexId);

  DWORD err = ::GetLastError();
  if (err == ERROR_ALREADY_EXISTS) {
    *already_running = true;
    return true;
  }

  if (single_instance_mutex_handle_ == NULL) {
    LOG(("CreateMutex failed with errno=%u", err));
    return false;
  }

  *already_running = false;
  return true;
}

bool Win32Notifier::Initialize() {
  bool already_running = false;
  if (!CheckAlreadyRunning(&already_running)) {
    return false;
  }
  if (already_running) {
    LOG(("Notifier already started. Quit\n"));
    return false;
  }

  return Notifier::Initialize();
}

void Win32Notifier::Terminate() {
  // Release the single instance protection.
  if (single_instance_mutex_handle_) {
    ::ReleaseMutex(single_instance_mutex_handle_);
    single_instance_mutex_handle_ = NULL;
  }

  Notifier::Terminate();
}

int Win32Notifier::Run() {
  running_ = true;
  MSG msg;
  while (running_ && ::GetMessage(&msg, NULL, 0, 0)) {
    ::TranslateMessage(&msg);
    ::DispatchMessage(&msg);
  }
  return 0;
}

void Win32Notifier::RequestQuit() {
  ::PostQuitMessage(0);
}

bool Win32Notifier::RegisterProcess() {
  assert(!dummy_wnd_.get());
  dummy_wnd_.reset(new NotifierDummyWindow);
  if (!dummy_wnd_->Initialize()) {
    return false;
  }

  sync_gate_.reset(new NotifierSyncGate(kNotifierStartUpSyncGateName));
  return sync_gate_->Open();
}

bool Win32Notifier::UnregisterProcess() {
  sync_gate_.reset(NULL);

  assert(dummy_wnd_.get());
  dummy_wnd_.reset(NULL);

  return true;
}

bool Win32Notifier::IsRestartNeeded() const {
  std::string16 new_version;
  if (!GetNotifierVersion(&new_version)) {
    return false;
  }
  if (new_version == running_version_) {
    return false;
  }

// For debugging: when running notifier.exe directly from build location,
// the version is empty since notifier.dll is in the same directory as
// notifier.exe. Don't restart.
#ifdef DEBUG
  if (running_version_.empty()) {
    return false;
  }
#endif

  std::string16 path;
  GetMainModulePath(&path);
  path += running_version_;
  path += kPathSeparator;
  path += kCoreDllName;
  return File::Exists(path.c_str());
}

void Win32Notifier::WaitParentProcessExitOnRestart() {
  uint32 parent_process_id = 0;
  if (!GetParentProcessId(::GetCurrentProcessId(), &parent_process_id)) {
    return;
  }

  HANDLE process_handle = ::OpenProcess(SYNCHRONIZE, FALSE, parent_process_id);
  if (!process_handle) {
    return;
  }

  DWORD res = ::WaitForSingleObject(process_handle, kWaitProcessExitTimeoutMs);
  if (res == WAIT_TIMEOUT) {
    LOG(("Failed to wait parent process %d\n", parent_process_id));
  }

  ::CloseHandle(process_handle);
}

extern "C" {

__declspec(dllexport) BOOL WINAPI DllMain(HINSTANCE, DWORD, LPVOID) {
  return TRUE;
}

__declspec(dllexport) int WINAPI DllEntry(const wchar_t *running_version) {
#if defined(WIN32) && !defined(WINCE)
// Only send crash reports for official builds.  Crashes on an engineer's
// machine during internal development are confusing false alarms.
#ifdef OFFICIAL_BUILD
  static ExceptionManager exception_manager(true);
  exception_manager.StartMonitoring();
  // Trace buffers only exist in dbg official builds.
#ifdef DEBUG
  exception_manager.AddMemoryRange(g_trace_buffers,
                                   sizeof(g_trace_buffers));
  exception_manager.AddMemoryRange(g_trace_positions,
                                   sizeof(g_trace_positions));
#endif  // DEBUG
#endif  // OFFICIAL_BUILD
#endif  // WIN32 && !WINCE

  int argc = 0;
  char16 **argv = CommandLineToArgvW(GetCommandLineW(), &argc);
  if (!argv) { return __LINE__; }  // return line as a lame error code

  LOG(("Gears Notifier started.\n"));

  Win32Notifier notifier(running_version);

  if (argc > 1 && wcscmp(argv[1], kRestartCmdLineSwitch) == 0) {
    notifier.WaitParentProcessExitOnRestart();
  }

  LocalFree(argv);  // MSDN says to free 'argv', using LocalFree().

  int retval = -1;
  if (notifier.Initialize()) {
    retval = notifier.Run();
    notifier.Terminate();
  }

  LOG(("Gears Notifier terminated.\n"));

  return retval;
}

}  // extern "C"

#endif  // OFFICIAL_BUILD
#endif  // WIN32
