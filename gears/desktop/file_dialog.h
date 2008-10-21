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

#ifndef GEARS_DESKTOP_FILE_DIALOG_H__
#define GEARS_DESKTOP_FILE_DIALOG_H__

#include <map>
#include <vector>

#include "gears/base/common/common.h"
#include "gears/base/common/js_types.h"
#include "gears/base/common/mutex.h"
#include "gears/ui/common/window_utils.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

struct ModuleEnvironment;
class ModuleImplBaseClass;

// An interface to file dialogs on multiple platforms.
class FileDialog {
 public:
  enum Mode {
    SINGLE_FILE,     // Let the user select exactly one file.
    MULTIPLE_FILES,  // Let the user select one or more files.
    // Future modes may allow selection of directories.
  };

  typedef std::vector<std::string16> StringList;

  struct Options {
    Options() : mode(MULTIPLE_FILES) { }

    Mode mode;
    // filter is a vector of Internet Media Types (eg, text/plain) and
    // filename extensions (eg, .txt).  If non-empty, the file dialog will
    // filter the selectable files by this criteria.
    StringList filter;
  };

  static FileDialog* Create(const ModuleImplBaseClass* module);

  virtual ~FileDialog();

  // Displays a file selection dialog according to the provided options, and
  // emits a callback with the selected files when complete.
  bool Open(const FileDialog::Options& options, JsRootedCallback* callback,
            std::string16* error);

  // Makes the dialog visible if it's hidden.
  void Show();

  // Makes the dialog invisible if it's shown.
  void Hide();

  // Moves the dialog to the front of the window z-order.
  void BringToFront();

  // Prematurely terminates the dialog selection.
  void Cancel();

  static bool ParseOptions(JsCallContext* context, const JsObject& map,
                           Options* options);

  // Creates an array of javascript objects from files.
  // Each javascript object has the following properties.
  //  name - the filename without path
  //  blob - the blob representing the contents of the file
  static bool FilesToJsObjectArray(const StringList& selected_files,
                                   ModuleEnvironment* module_environment,
                                   JsArray* files,
                                   std::string16* error);

 protected:
  enum UIAction { UIA_SHOW, UIA_HIDE, UIA_BRING_TO_FRONT };

  FileDialog();

  void Init(const ModuleImplBaseClass* module, NativeWindowPtr parent);

  // Implemented per platform to create and display a dialog with the provided
  // options, or returns false and sets error.  The dialog displays
  // asynchronously, and calls CompleteSelection() when dismissed.
  virtual bool BeginSelection(NativeWindowPtr parent,
                              const FileDialog::Options& options,
                              std::string16* error) = 0;

  // Implemented per platform to cancel the asynchronous dialog.
  virtual void CancelSelection() = 0;

  // Implemented per platform to perform UI actions on the dialog window.
  virtual void DoUIAction(UIAction action) = 0;

  // Called by platform implementations when the user has dismissed the dialog.
  // selected_files will be empty if the user cancelled the selection.
  void CompleteSelection(const StringList& selected_files);

  // Handles errors that occur during asynchronous processing (and therefore
  // can't be returned as exceptions to JavaScript).
  void HandleError(const std::string16& error);

  static bool IsLegalFilter(const std::string16& filter);

 private:
  typedef std::map<ModuleEnvironment*, FileDialog*> ActiveMap;

  const ModuleImplBaseClass* module_;
  NativeWindowPtr parent_;
  scoped_ptr<JsRootedCallback> callback_;

  // Provides thread safety for active_.
  static Mutex active_mutex_;
  // Maps all active dialogs, by ModuleEnvironment.
  static ActiveMap active_;

  DISALLOW_EVIL_CONSTRUCTORS(FileDialog);
};

#endif  // GEARS_DESKTOP_FILE_DIALOG_H__
