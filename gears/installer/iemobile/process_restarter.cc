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

#include "gears/installer/iemobile/process_restarter.h"

#include <psapi.h>
#include <shellapi.h>
#include <tlhelp32.h>
#include <vector>

#include "third_party/scoped_ptr/scoped_ptr.h"

//------------------------------------------------------------------------------
// class ProcessRestarter
//------------------------------------------------------------------------------

ProcessRestarter::ProcessRestarter(const char16* process_name) {
  ProcessRestarter(process_name, NULL);
}

ProcessRestarter::ProcessRestarter(const char16* process_name,
                                   const char16* window_name)
    : recursion_level_(0),
      process_name_(process_name),
      window_name_(window_name) {
}

// Clean up if we've left the process handle open
ProcessRestarter::~ProcessRestarter() {
  CloseAllHandles();
}

// Tries to kill the process that was specified in the constructor.
// timeout_msec specifies a grace period before trying the next killing method.
// method_mask specifies what killing methods to try.
// was_found is optional and can be NULL.
// Returns S_OK if all instances were killed, S_FALSE if process wasn't running,
// and E_FAIL if one or more instances weren't killed or finding the process
// failed at any step.
// The was_found parameter is only set if the return value is either S_OK or
// S_FALSE.
HRESULT ProcessRestarter::KillTheProcess(int timeout_msec,
                                         uint32 method_mask, bool* was_found) {
  LOG16(_T("KillTheProcess"));

  bool found;
  HRESULT result = FindProcessInstances(&found);
  if (FAILED(result)) return E_FAIL;
  if (!found) {
    if (was_found != NULL) {
      *was_found = false;
    }
    return S_FALSE;  // process is not running, so don't return a FAILED hr
  }

  // Try the nicest, cleanest method of closing a process: window messages.
  if (method_mask & KILL_METHOD_1_WINDOW_MESSAGE) {
    if (PrepareToKill(KILL_METHOD_1_WINDOW_MESSAGE)) {
      KillProcessViaWndMessages(timeout_msec);
    }

    // Are any instances of the process still running?
    result = FindProcessInstances(&found);
    if (FAILED(result)) return E_FAIL;
    if (!found) {
      // Success. Set was_found and return.
      if (was_found != NULL) *was_found = true;
      return S_OK;  // killed them all
    }
  }

  // Also nice method.
  if (method_mask & KILL_METHOD_2_THREAD_MESSAGE) {
    if (PrepareToKill(KILL_METHOD_2_THREAD_MESSAGE)) {
      KillProcessViaThreadMessages(timeout_msec);
    }
    // Are any instances of the process still running?
    result = FindProcessInstances(&found);
    if (FAILED(result)) return E_FAIL;
    if (!found) {
      // Success. Set was_found and return.
      if (was_found != NULL) *was_found = true;
      return S_OK;  // killed them all
    }
  }

  // Try the the crude one.
  if (method_mask & KILL_METHOD_3_TERMINATE_PROCESS) {
    if (PrepareToKill(KILL_METHOD_3_TERMINATE_PROCESS)) {
      KillProcessViaTerminate(timeout_msec);
    }
    // Are any instances of the process still running?
    result = FindProcessInstances(&found);
    if (FAILED(result)) return E_FAIL;
    if (!found) {
      // Success. Set was_found and return.
      if (was_found != NULL) *was_found = true;
      return S_OK;  // killed them all
    }

    LOG16((_T("KillTheProcess - totally unable to kill process '%s'"),
          process_name_));
  }

  return E_FAIL;
}

HRESULT ProcessRestarter::WaitForAllToDie(int timeout_msec) {
  LOG16(_T("WaitForAllToDie"));
  bool found = false;
  HRESULT result = FindProcessInstances(&found);
  if (FAILED(result)) return result;
  if (!found) {
    return S_OK;
  }
  // There are still some instances
  if (PrepareToKill(KILL_METHOD_1_WINDOW_MESSAGE)) {
    return WaitForProcessInstancesToDie(timeout_msec) ?
        S_OK : HRESULT_FROM_WIN32(WAIT_TIMEOUT);
  }

  return E_FAIL;
}

HRESULT ProcessRestarter::StartTheProcess(const std::string16& args) {
  bool found = false;
  HRESULT result = FindProcessInstances(&found);
  if (FAILED(result)) return result;
  if (found) {
    // Process is already running. Bail out.
    return S_FALSE;
  }
  SHELLEXECUTEINFO info;
  ZeroMemory(&info, sizeof(SHELLEXECUTEINFO));
  info.cbSize = sizeof(SHELLEXECUTEINFO);
  info.fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_NOCLOSEPROCESS;
  info.lpFile = process_name_;
  info.lpParameters = args.c_str();
  info.nShow = SW_SHOWNORMAL;
  if (::ShellExecuteEx(&info) == TRUE) return S_OK;
  return HRESULT_FROM_WIN32(::GetLastError());
}

HRESULT ProcessRestarter::IsProcessRunning(bool* is_running) {
  return FindProcessInstances(is_running);
}

//------------------------------------------------------------------------------
// Internal
//------------------------------------------------------------------------------

// Finds all instances of the process.
HRESULT ProcessRestarter::FindProcessInstances(bool* is_running) {
  if (SUCCEEDED(FindProcessInstancesUsingSnapshot(is_running))) {
    return S_OK;
  }
  return FindProcessInstancesUsingFindWindow(is_running);
}

// See http://msdn2.microsoft.com/en-us/library/aa446560.aspx for details
// on how to enumerate processes on Windows Mobile.
HRESULT ProcessRestarter::FindProcessInstancesUsingSnapshot(bool* found) {
  ASSERT(found);
  // Clear the process_ids_.
  process_ids_.clear();
  // Create a snapshot of the processes running in the system.
  // According to http://www.themssforum.com/PocketPCDev/CreateToolhelpSnapshot/
  // CreateToolhelp32Snapshot always allocates 1MB of virtual memory, which can
  // sometimes fail (e.g. high system memory usage, or fragmentation).
  HANDLE handle = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (handle == INVALID_HANDLE_VALUE) {
    return HRESULT_FROM_WIN32(::GetLastError());
  }

  // Create a new process entry structure
  PROCESSENTRY32 current_process_entry;
  ZeroMemory(&current_process_entry, sizeof(PROCESSENTRY32));
  current_process_entry.dwSize = sizeof(PROCESSENTRY32);

  // Get the first process
  BOOL success = ::Process32First(handle, &current_process_entry);
  while (success) {
    // Save the process id if the exe file name matches
    if (wcscmp(current_process_entry.szExeFile, process_name_) == 0) {
      process_ids_.push_back(current_process_entry.th32ProcessID);
    }
    // We need to reset this after each call to Process32Next
    current_process_entry.dwSize = sizeof(PROCESSENTRY32);
    success = ::Process32Next(handle, &current_process_entry);
  }

  // Done, so close the handle, free the process structure and return
  ::CloseToolhelp32Snapshot(handle);  // we ignore this function's return code
  *found = !process_ids_.empty();
  return S_OK;
}

HRESULT ProcessRestarter::FindProcessInstancesUsingFindWindow(bool* found) {
  ASSERT(found);
  
  // We fail fast if the window_name is NULL or the empty string.
  if (window_name_ == NULL || wcslen(window_name_) == 0) {
    return E_FAIL;
  }

  HRESULT result;
  HWND handle = FindWindow(NULL, window_name_);
  if (handle == NULL) {
    // We didn't find the window. Is this because there is no such window
    // or because FindWindow failed for a different reason?
    result = HRESULT_FROM_WIN32(::GetLastError());
    if (SUCCEEDED(result)) {
      // There is no such window.
      *found = false;
    }
    return result;
  }

  // Found a window, get the PID.  
  uint32 process_id = 0;
  uint32 thread_id =
      ::GetWindowThreadProcessId(handle, reinterpret_cast<DWORD*>(&process_id));  
  process_ids_.push_back(process_id);
  *found = true;
  return S_OK; 
}

// Given the process_ids_ array, this method will try to
// open a handle to each instance.
// Leaves process handles open (in member process_handles_)
// This function recursively calls itself if by the time
// it tries to open handles to process instances,
// some of the processes died or naturally exited.
bool ProcessRestarter::PrepareToKill(uint32 method_mask) {
  LOG16(_T("PrepareToKill"));

  // do clean up in case some handles are opened.
  CloseAllHandles();

  if (process_ids_.empty()) {
    // no instances are running.
    return false;
  }

  for (size_t i = 0; i < process_ids_.size(); ++i) {
    // On Windows Mobile, the first two parameters
    // to OpenProcess must be 0 and false.
    HANDLE handle = ::OpenProcess(0, false, process_ids_[i]);
    if (handle) {
      process_handles_.push_back(handle);
    } else {
      DWORD last_error = ::GetLastError();
      if (last_error == ERROR_ACCESS_DENIED) {
        // If we are here that means that we do not have enough priveleges
        // to open the process for a given kill method. No reason to attempt
        // other instances. Just clean up and return false.
        LOG16((_T("PrepareToKill failed for '%s'. Kill method %d."),
              process_name_,
              method_mask));
        CloseAllHandles();
        return false;
      }
    }
  }

  // We already handled the case when we don't have enough privileges to
  // open the process. So if we have less handles than process ids, then some
  // of the processes have died since we made a snapshot untill the time we
  // tried to open handles. We need to do another snapshot and try to
  // open handles one more time. We need number of handles and number of
  // ids to be equal. We can do it with recursion.
  // The idea is: make the next snapshot and open handles. Hopefully the number
  // will be equal. Stop recursion at the third level.

  if (process_handles_.size() != process_ids_.size()) {
    ++recursion_level_;

    // We have a disbalance here. This means that Some of the processes died
    // already so we need to take another snapshot.
    // Are any instances of the process still running?
    bool found = false;
    if(FAILED(FindProcessInstances(&found)) || !found) {
      // they are all dead or we can't tell.
      recursion_level_ = 0;
      return false;
    }

    // Try to obtain the balance three times, no more.
    if (recursion_level_ >= 3) {
      recursion_level_ = 0;
      LOG16((_T("Recursion level too deep in PrepareToKill for '%s'."),
            process_name_));
      return false;
    }

    // recursively call the function
    return PrepareToKill(method_mask);
  }
  recursion_level_ = 0;
  return true;
}

// Wait for a while till all process instances will die.
bool ProcessRestarter::WaitForProcessInstancesToDie(int timeout_msec) const {
  LOG16((_T("WaitForProcessInstancesToDie")));
  size_t size = process_handles_.size();
  scoped_array<HANDLE> handles(new HANDLE[size]);

  for (size_t i = 0; i < size; ++i) {
    handles[i] = process_handles_[i];
  }

  DWORD wait_result = ::WaitForMultipleObjects(size, handles.get(), true,
                                               timeout_msec);

#pragma warning(disable : 4296)
// C4296: '>=' : expression is always true
  if ((wait_result >= WAIT_OBJECT_0) &&
      (wait_result < WAIT_OBJECT_0 + size)) {
    return true;
  }
#pragma warning(default : 4296)

  LOG16((_T("WaitForProcessToDie timed out for '%s'. Waited for %d ms."),
        process_name_,
        timeout_msec));
  return false;
}

// Close all currently opened handles.
void ProcessRestarter::CloseAllHandles() {
  LOG16((_T("CloseAllHandles")));
  // Do clean up if we have opened handles.
  for (size_t i = 0; i < process_handles_.size(); ++i) {
    VERIFY(::CloseHandle(process_handles_[i]));
  }
  process_handles_.clear();
}

//------------------------------------------------------------------------------
// Implementation of KILL_METHOD_1_WINDOW_MESSAGE
//------------------------------------------------------------------------------

// Just calls built-in enumeration function
bool ProcessRestarter::FindProcessWindows() {
  window_handles_.clear();
  return ::EnumWindows(EnumAllWindowsProc, reinterpret_cast<LPARAM>(this)) &&
      !window_handles_.empty();
}

// During enumeration, this function will try to find a match between
// process ids we already found and process ids obtained from each window.
// If there is a match, we record the window in an array.
BOOL ProcessRestarter::EnumAllWindowsProc(HWND hwnd, LPARAM lparam) {
  ProcessRestarter* this_pointer =
      reinterpret_cast<ProcessRestarter*>(lparam);
  ASSERT(this_pointer);

  uint32 process_id = 0;
  uint32 thread_id =
    ::GetWindowThreadProcessId(hwnd, reinterpret_cast<DWORD*>(&process_id));

  for (std::vector<uint32>::const_iterator it =
      this_pointer->process_ids_.begin();
      it != this_pointer->process_ids_.end();
      ++it) {
    if (*it == process_id) {
      // Only close visible windows (please note that, on Windows Mobile,
      // EnumWindows only enumerates top level windows).
      if (::IsWindowVisible(hwnd)) {
        this_pointer->window_handles_.push_back(hwnd);
      }
    }
  }
  return TRUE;
}

// Attempts to kill the process by delivering a WM_CLOSE message to
// the process' visible windows.
bool  ProcessRestarter::KillProcessViaWndMessages(int timeout_msec) {
  LOG16((_T("KillProcessViaWndMessages")));
  if (!FindProcessWindows()) {
    LOG16((_T("failed to find any windows for '%s'"), process_name_));
    return false;
  }

  bool post_messages_succeeded = false;

  for (size_t i = 0; i < window_handles_.size(); ++i) {
    // On Windows Mobile, only WM_CLOSE is capable of closing
    // a process.
    if (::PostMessage(window_handles_[i], WM_CLOSE, NULL, NULL)) {
      post_messages_succeeded = true;
    }
  }

  if (!post_messages_succeeded) {
    LOG16((_T("failed to PostMessage to windows of '%s'"), process_name_));
  }
  // If we succeeded in posting message at least one time, we have to wait.
  // We don't know the relationship between windows in the process.
  return post_messages_succeeded && WaitForProcessInstancesToDie(timeout_msec);
}

//------------------------------------------------------------------------------
// Implementation of KILL_METHOD_2_THREAD_MESSAGE
//------------------------------------------------------------------------------

// find all the threads running in a given process.
bool ProcessRestarter::FindProcessThreads(std::vector<uint32>* thread_ids) {
  // Get a snapshot that includes thread ids
  HANDLE process_snapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
  if (process_snapshot == INVALID_HANDLE_VALUE) return false;

  THREADENTRY32 thread_info = {0};  // zero it out just in case.
  thread_info.dwSize = sizeof(THREADENTRY32);

  if (::Thread32First(process_snapshot, &thread_info)) {
    do {
      for (std::vector<uint32>::const_iterator it = process_ids_.begin();
           it != process_ids_.end(); ++it) {
        if (*it == thread_info.th32OwnerProcessID) {
          // we have found it.
          thread_ids->push_back(thread_info.th32ThreadID);
        }
      }
      // The system changes this value, do not forget to reset to
      // max possible.
      thread_info.dwSize = sizeof(THREADENTRY32);
    } while (::Thread32Next(process_snapshot, &thread_info));
  }

  return !thread_ids->empty();
}

// Try to post a thread message.
bool ProcessRestarter::KillProcessViaThreadMessages(int timeout_msec) {
  LOG16((_T("KillProcessViaThreadMessages")));
  std::vector<uint32> thread_ids;

  if (!FindProcessThreads(&thread_ids)) {
    LOG16((_T("failed to find any threads for '%s'"), process_name_));
    return false;
  }

  bool post_messages_succeeded = false;
  for (size_t i = 0; i < thread_ids.size(); ++i) {
    if (::PostThreadMessage(thread_ids[i], WM_CLOSE, 0, 0)) {
      post_messages_succeeded = true;
    }
  }

  if (!post_messages_succeeded) {
    LOG16((_T("[failed to PostMessage to threads of '%s'."), process_name_));
  }

  // If we succeded in posting message to at least one thread we have to wait.
  // We don't know the relationship between threads in the process.
  return post_messages_succeeded && WaitForProcessInstancesToDie(timeout_msec);
}

//------------------------------------------------------------------------------
//  Implementation of KILL_METHOD_3_TERMINATE_PROCESS
//------------------------------------------------------------------------------

// Last and crude method to kill the process. Should be used only
// if all other methods have failed.
bool ProcessRestarter::KillProcessViaTerminate(int timeout_msec) {
  LOG16((_T("KillProcessViaTerminate")));
  bool at_least_one_terminated = false;

  for (size_t i = 0; i < process_handles_.size(); ++i) {
    if (!::TerminateProcess(process_handles_[i], 0)) {
      LOG16((_T("[failed for instance of '%s'.System error %d."),
            process_name_,
            ::GetLastError()));
    } else {
       at_least_one_terminated = true;
    }
  }
  return at_least_one_terminated ?
      WaitForProcessInstancesToDie(timeout_msec) : false;
}
