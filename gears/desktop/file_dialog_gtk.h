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

#ifndef GEARS_DESKTOP_FILE_DIALOG_GTK_H__
#define GEARS_DESKTOP_FILE_DIALOG_GTK_H__

#if defined(LINUX) && !defined(OS_MACOSX)

#include "gears/desktop/file_dialog.h"
#include "gears/base/common/scoped_token.h"

#include <gtk/gtk.h>
#include <gtk/gtkwindow.h>

typedef DECLARE_SCOPED_TRAITS(GtkWidget*, gtk_widget_destroy, NULL)
    GtkWidgetTraits;
typedef scoped_token<GtkWidget*, GtkWidgetTraits> scoped_GtkWidget;

class FileDialogGtk : public FileDialog {
 public:
  FileDialogGtk();
  virtual ~FileDialogGtk();

  // Called when the user dismisses the dialog normally.
  void HandleResponse(gint response_id);

  // Called when the dialog is closed abnormally.
  void HandleClose();

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
                  const FileDialog::Options& options,
                  std::string16* error);

  // Installs the default filename filter.
  bool SetFilter(const StringList& filter, std::string16* error);

  // Creates and displays the file dialog.
  bool Display(std::string16* error);

  // Extracts the selected files from the file dialog.
  bool ProcessSelection(StringList* selected_files, std::string16* error);

  scoped_GtkWidget dialog_;
  gulong response_handler_;

  DISALLOW_EVIL_CONSTRUCTORS(FileDialogGtk);
};

#endif  // defined(LINUX) && !defined(OS_MACOSX)

#endif  // GEARS_DESKTOP_FILE_DIALOG_GTK_H__
