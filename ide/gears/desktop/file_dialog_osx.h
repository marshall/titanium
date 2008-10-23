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

#ifndef GEARS_DESKTOP_FILE_DIALOG_OSX_H__
#define GEARS_DESKTOP_FILE_DIALOG_OSX_H__

#ifdef OS_MACOSX

#include <Carbon/Carbon.h>

#include "gears/base/safari/scoped_cf.h"
#include "gears/desktop/file_dialog.h"

class Filters;

class FileDialogCarbon : public FileDialog {
 public:
  FileDialogCarbon();
  virtual ~FileDialogCarbon();

  // Determines whether the selected file matches our active filter.
  bool Filter(AEDesc* theItem, NavFileOrFolderInfo* info,
              NavFilterModes filterMode);

  // Processes events from the dialog.
  void Event(NavEventCallbackMessage message, NavCBRecPtr parameters);

 protected:
  // FileDialog Interface
  virtual bool BeginSelection(NativeWindowPtr parent,
                              const FileDialog::Options& options,
                              std::string16* error);
  virtual void CancelSelection();
  virtual void DoUIAction(UIAction action);

 private:
  // Initializes the dialog, based on options.
  bool InitDialog(NativeWindowPtr parent,
                  const Options& options, std::string16* error);

  // Converts the input filter list into the Mac-native filter type, UTIs.
  bool SetFilter(const StringList& filter,
                 NavDialogCreationOptions* dialog_options,
                 std::string16* error);

  // Creates and displays the file dialog.
  bool Display(std::string16* error);

  // Extracts the selected files from the file dialog.
  bool ProcessSelection(StringList* selected_files, std::string16* error);

  scoped_NavDialogRef dialog_;
  scoped_cftype<CFMutableArrayRef> utis_;
  scoped_cftype<CFArrayRef> labels_;
  int selected_filter_;
  StringList selected_files_;

  DISALLOW_EVIL_CONSTRUCTORS(FileDialogCarbon);
};

#endif  // OS_MACOSX

#endif  // GEARS_DESKTOP_FILE_DIALOG_OSX_H__
