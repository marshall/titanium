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

#if defined(WIN32) && !defined(WINCE)

#ifdef USING_CCTESTS

#include "gears/base/common/ipc_message_queue_test.h"

#include <stdio.h>
#include <stdlib.h>

#include "gears/base/common/common.h"
#include "gears/base/common/paths.h"
#include "gears/base/common/process_utils_win32.h"
#include "gears/base/common/stopwatch.h"
#include "gears/base/common/string16.h"
#include "gears/base/common/string_utils.h"

// Work around the header including errors.
#ifdef WIN32
#include <atlbase.h>
#include <windows.h>
#endif

const char16 *kIpcTestApp = STRING16(L"ipc_test.exe");

void TestingIpcMessageQueueWin32_GetAllProcesses(
        bool as_peer, std::vector<IpcProcessId> *processes);

//-----------------------------------------------------------------------------
// Master process test code
//-----------------------------------------------------------------------------

bool WaitForRegisteredProcesses(int n, int timeout) {
  int64 start_time = GetCurrentTimeMillis();
  std::vector<IpcProcessId> registered_processes;
  while (true) {
    TestingIpcMessageQueueWin32_GetAllProcesses(true, &registered_processes);
    if (registered_processes.size() == n)
      return true;
    if (!timeout || GetCurrentTimeMillis() - start_time > timeout)
      return false;
    Sleep(1);
  }
  return false;
}

bool ValidateRegisteredProcesses(int n, const IpcProcessId *process_ids) {
  std::vector<IpcProcessId> registered_processes;
  TestingIpcMessageQueueWin32_GetAllProcesses(true, &registered_processes);
  if (registered_processes.size() == n) {
    if (process_ids) {
      for (int i = 0; i < n; ++i) {
        if (registered_processes[i] != process_ids[i]) {
          return false;
        }
      }
    }
    return true;
  }
  return false;
}

bool MasterMessageHandler::WaitForMessages(int n) {
  assert(save_messages_);
  MutexLock locker(&lock_);
  num_messages_waiting_to_receive_ = n;
  wait_for_messages_start_time_ = GetCurrentTimeMillis();
  lock_.Await(Condition(this, &MasterMessageHandler::HasMessagesOrTimedout));
  return num_received_messages_ == num_messages_waiting_to_receive_;
}

bool GetSlavePath(bool as_peer, std::string16 *slave_path) {
  assert(slave_path);

#ifndef BROWSER_NONE
  // Additional arguments: peer/system process_id
  wchar_t additional_arguments[64] = {0};
  _snwprintf(additional_arguments,
             sizeof(additional_arguments), 
             L" %u %d",
             as_peer,
             ::GetCurrentProcessId());

#ifdef BROWSER_IE
  // Via rundll32.exe.
  char16 module_path[MAX_PATH] = {0};  // folder + filename
  if (0 == ::GetModuleFileName(GetGearsModuleHandle(),
                               module_path, MAX_PATH)) {
    return false;
  }

  // Get a version without spaces, to use it as a command line argument.
  char16 module_short_path[MAX_PATH] = {0};
  if (0 == GetShortPathNameW(module_path, module_short_path, MAX_PATH)) {
    return false;
  }

  *slave_path += L"rundll32.exe ";
  *slave_path += module_short_path;
  *slave_path += L",RunIpcSlave";
  *slave_path += additional_arguments;
#else
  // Via ipc_test.exe.
  slave_path->append(L"\"");

  std::string16 install_directory;
  if (!GetComponentDirectory(&install_directory)) {
    LOG(("Couldn't get install directory\n"));
    return false;
  }

  slave_path->append(install_directory);
  AppendName(kIpcTestApp, slave_path);

  slave_path->append(L"\"");
  slave_path->append(additional_arguments);
#endif  // BROWSER_IE

#endif  // !BROWSER_NONE

  return true;
}

bool SlaveProcess::Start(bool as_peer) {
  // Get the command.
  std::string16 slave_path;
  if (!GetSlavePath(as_peer, &slave_path)) {
    return false;
  }

  // Execute the command.
  PROCESS_INFORMATION pi = { 0 };
  STARTUPINFO si;
  GetStartupInfo(&si);
  if (!::CreateProcess(NULL, &slave_path.at(0),
                       NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
    return false;
  }
  ::CloseHandle(pi.hProcess);
  ::CloseHandle(pi.hThread);

  id_ = pi.dwProcessId;
  as_peer_ = as_peer;
  slave_processes_.push_back(id_);

  return true;
}

bool SlaveProcess::WaitTillRegistered(int timeout) {
  assert(id_);

  int64 start_time = GetCurrentTimeMillis();  
  std::vector<IpcProcessId> registered_processes;
  while (GetCurrentTimeMillis() - start_time < timeout) {
    TestingIpcMessageQueueWin32_GetAllProcesses(as_peer_,
                                                &registered_processes);
    if (std::find(registered_processes.begin(),
                  registered_processes.end(),
                  id_) != registered_processes.end()) {
      return true;
    }

    Sleep(1);
  }
  return false;
}

bool SlaveProcess::WaitForExit(int timeout) {
  assert(id_);

  HANDLE handle = ::OpenProcess(SYNCHRONIZE, false, id_);
  if (!handle) {
    return true;
  }

  bool rv = WaitForSingleObject(handle, timeout) == WAIT_OBJECT_0;
  ::CloseHandle(handle);
  return rv;
}


//-----------------------------------------------------------------------------
// Slave process test code
//-----------------------------------------------------------------------------

static SlaveMessageHandler g_slave_handler;

IpcMessageQueue *SlaveMessageHandler::GetIpcMessageQueue() const {
  return as_peer_ ? IpcMessageQueue::GetPeerQueue()
                  : IpcMessageQueue::GetSystemQueue();
}

void SlaveMessageHandler::TerminateSlave() {
  g_slave_handler.set_done(true);
  LOG(("Terminating slave process %u\n", ::GetCurrentProcessId()));
}

bool InitSlave() {
  LOG(("Initializing slave process %u\n", ::GetCurrentProcessId()));

  // Parse the additional arguments.
  // The last-1 argument indicates if we should start slave for peer or system
  // IPC testing. The last argument is the parent process id.
  int argc = 0;
  wchar_t **argv = ::CommandLineToArgvW(::GetCommandLineW(), &argc);
  if (!argv) {
    return false;
  }
  int as_peer = 0;
  int process_id = 0;
  if (argc > 2) {
    as_peer = _wtoi(argv[argc - 2]);
    process_id = _wtoi(argv[argc - 1]);
  }
  ::GlobalFree(argv);
  if (process_id <= 0) {
    return false;
  }
  g_slave_handler.set_as_peer(as_peer != 0);
  g_slave_handler.set_parent_process_id(process_id);

  // Create the new ipc message queue for the child process.
  IpcTestMessage::RegisterAsSerializable();
  IpcMessageQueue *ipc_message_queue = g_slave_handler.GetIpcMessageQueue();
  if (!ipc_message_queue) {
    return false;
  }
  ipc_message_queue->RegisterHandler(kIpcQueue_TestMessage, &g_slave_handler);

  // Send a hello message to the master process to tell that the child process
  // has been started.
  ipc_message_queue->Send(g_slave_handler.parent_process_id(),
                          kIpcQueue_TestMessage,
                          new IpcTestMessage(GetHelloMessage()));

  LOG(("Slave process sent hello message to master process %u\n",
       g_slave_handler.parent_process_id()));

  return true;
}

void RunSlave() {
  LOG(("Running slave process %u\n", ::GetCurrentProcessId()));

  // Loop until either done_ or our parent process terminates. While we're
  // looping the ipc worker thread will call HandleIpcMessage.
  ATL::CHandle parent_process(
                  ::OpenProcess(SYNCHRONIZE, FALSE,
                                g_slave_handler.parent_process_id()));
  while (::WaitForSingleObject(parent_process, 1000) == WAIT_TIMEOUT &&
         !g_slave_handler.done()) {
  }

  LOG(("Slave process %u exitting\n", ::GetCurrentProcessId()));
}

int SlaveMain() {
  if (!InitSlave()) {
    return 1;
  }
  RunSlave();
  return 0;
}

#ifdef BROWSER_NONE

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
  return SlaveMain();
}

#elif BROWSER_IE

// This is the function that rundll32 calls when we launch other processes.
extern "C"
__declspec(dllexport) void __cdecl RunIpcSlave(HWND window,
                                               HINSTANCE instance,
                                               LPWSTR command_line,
                                               int command_show) {
  SlaveMain();
}

#endif  // BROWSER_NONE

#endif  // USING_CCTESTS

#endif  //defined(WIN32) && !defined(WINCE)
