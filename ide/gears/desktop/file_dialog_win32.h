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

#ifndef GEARS_DESKTOP_FILE_DIALOG_WIN32_H__
#define GEARS_DESKTOP_FILE_DIALOG_WIN32_H__

#ifdef WIN32

#include <windows.h>
#include "gears/base/common/message_service.h"
#include "gears/base/common/thread.h"
#include "gears/desktop/file_dialog.h"

class JsRunnerInterface;

class FileDialogWin32 : public FileDialog, public Thread,
                        public MessageObserverInterface {
 public:
  FileDialogWin32();
  virtual ~FileDialogWin32();

 protected:
  // FileDialog Interface
  virtual bool BeginSelection(NativeWindowPtr parent,
                              const FileDialog::Options& options,
                              std::string16* error);
  virtual void CancelSelection();
  virtual void DoUIAction(UIAction action);

  // Thread Interface
  virtual void Run();

  // MessageObserverInterface
  virtual void OnNotify(MessageService *service,
                        const char16 *topic,
                        const NotificationData *data);

 private:
  // Initializes the dialog, based on options.
  void InitDialog(NativeWindowPtr parent,
                  const FileDialog::Options& options);

  // Installs the default filename filter.
  bool SetFilter(const StringList& filter, std::string16* error);

  // Creates and displays the file dialog.
  bool Display(StringList* selected_files, std::string16* error);

  // Handles OS events while the dialog is running.
  static UINT_PTR CALLBACK HookProc(HWND hdlg, UINT uiMsg, WPARAM wParam,
                                    LPARAM lParam);

  // Extracts the selected files from the file dialog.
  bool ProcessSelection(StringList* selected_files, std::string16* error);

  // The OS specification for the dialog.
  OPENFILENAME ofn_;
  // Provides the backing memory for ofn_.lpstrFilter.
  std::string16 filter_;
  // Provides the backing memory for ofn_.lpstrFile.
  std::vector<TCHAR> filename_buffer_;
  // MessageService topic for communication from worker to main thread.
  std::string16 topic_;

  // Protects access to the following members, which are accessed from multiple
  // threads.
  Mutex mutex_;
  // The main thread sets this to true, when the dialog should exit.
  bool should_exit_;
  // The window handle of the dialog.
  HWND wnd_;

  DISALLOW_EVIL_CONSTRUCTORS(FileDialogWin32);
};

#endif  // WIN32

#endif  // GEARS_DESKTOP_FILE_DIALOG_WIN32_H__
