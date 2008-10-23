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

#ifndef GEARS_INSTALLER_IEMOBILE_PROCESS_RESTARTER_H__
#define GEARS_INSTALLER_IEMOBILE_PROCESS_RESTARTER_H__

#include <windows.h>  // must be first
#include <vector>

#include "gears/base/common/common.h"
#include "gears/base/common/string16.h"

class ProcessRestarter {
 public:

  // constants for specifying which methods to attempt when killing a process
  static const int KILL_METHOD_1_WINDOW_MESSAGE = 0x01;
  static const int KILL_METHOD_2_THREAD_MESSAGE = 0x02;
  static const int KILL_METHOD_3_TERMINATE_PROCESS = 0x04;

  // Creates the object given the process_name to kill.
  explicit ProcessRestarter(const char16* process_name);
  // Creates the object given the process_name to kill.
  // The window_name parameter denotes the name of the main window created
  // by the process. If not NULL or empty, this string will be used as a
  // fallback mechanism for finding the process handle when finding by
  // process name failed due to a win32 API error.
  ProcessRestarter(const char16* process_name,
                   const char16* window_name);
  virtual ~ProcessRestarter();

  // Go through process list try to find the required one to kill,
  // trying three methods to kill, from easiest and cleanest to a
  // harsh one.  S_OK if no process by the right name was found, or if it was
  // found and was killed.  E_FAIL otherwise.  was_found returns true if
  // process was found. Kills all instances of a process.
  HRESULT KillTheProcess(int timeout_msec, uint32 method_mask, 
                         bool* was_found_out);

  // Wait for all instances of the process to die.
  HRESULT WaitForAllToDie(int timeout_msec);

  // Starts the process if there aren't already any other instances
  // already running.
  HRESULT StartTheProcess(const std::string16& args);

  // Tests if the process is currently running.
  // The is_running parameter is only set if the return value denotes success.
  HRESULT IsProcessRunning(bool* is_running);

 private:
  // Finds all instances of the process.
  // The found parameter is only set if the return value denotes success.
  HRESULT FindProcessInstances(bool* found);

  // Finds all instances of the process using ::CreateToolhelp32Snapshot
  // The found parameter is only set if the return value denotes success.
  HRESULT FindProcessInstancesUsingSnapshot(bool* found);

  // Finds all instances of the process using ::FindWindow
  // The found parameter is only set if the return value denotes success.
  HRESULT FindProcessInstancesUsingFindWindow(bool* found);

  // Will try to open handle to each instance.
  // Leaves process handles open (in member process_handles_).
  // Will use access rights for opening appropriate for the purpose_of_opening.
  bool PrepareToKill(uint32 method_mask);

  // Wait for process instances to die for timeout_msec.
  // Return true if all are dead and false if timed out.
  bool WaitForProcessInstancesToDie(int timeout_msec) const;

  // Close all currently opened handles.
  void CloseAllHandles();

  //
  // Killing via messages to window
  //
  // Function which meet win32 requirements for callback
  // function passed into EnumWindows function.
  BOOL static CALLBACK EnumAllWindowsProc(HWND hwnd, LPARAM lparam);

  // Will return true if it succeeds in finding a window for the process
  // to be killed, otherwise false.  If there are such top-level windows
  // then returns an array of window handles.
  bool FindProcessWindows();

  // Will try to kill the process via posting windows messages
  // returns true on success otherwise false.
  // Timeout is maximum time to wait for WM_CLOSE to work before going to
  // next method.
  bool KillProcessViaWndMessages(int timeout_msec);

  //
  // Killing via messages to thread
  //
  // Try to find the threads than run in
  // the process in question.
  bool FindProcessThreads(std::vector<uint32>* thread_ids);

  // Will try to kill the process via posing thread messages
  // returns true on success otherwise false.
  // Timeout is maximum time to wait for message to work before going to
  // next method.
  bool KillProcessViaThreadMessages(int timeout_msec);

  // The last and crude method to kill the process.
  // Calls TerminateProcess function.
  bool KillProcessViaTerminate(int timeout_msec);

  // Private member variables:
  const char16* process_name_;
  const char16* window_name_;
  // One process can have several instances
  // running. This array will keep handles to all
  // instances of the process.
  std::vector<HANDLE> process_handles_;
  // Array of process ids which correspond to different
  // instances of the same process.
  std::vector<uint32>  process_ids_;
  // Function PrepareToKill can call itself
  // recursively under some conditions.
  // We need to stop the recursion at some point.
  // This is the purpose of this member.
  int recursion_level_;
  // Array of window handles.
  std::vector<HWND> window_handles_;

  // Disable copy constructor and assignment operator.
  DISALLOW_EVIL_CONSTRUCTORS(ProcessRestarter);
};

#endif  // GEARS_INSTALLER_IEMOBILE_PROCESS_RESTARTER_H__
