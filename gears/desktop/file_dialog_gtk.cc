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

#if defined(LINUX) && !defined(OS_MACOSX)

#include "gears/desktop/file_dialog_gtk.h"

#include "gears/base/common/string_utils.h"
#include "gears/ui/common/i18n_strings.h"

namespace {

// Degenerate filter callbacks which allow any file to be selected.
gboolean AnyFileFilter(const GtkFileFilterInfo* /*filter_info*/,
                       gpointer /*data*/) {
  return TRUE;
}

void AnyFileFilterDestroy(gpointer /*data*/) {
}

void ResponseHandler(GtkWidget* dialog, gint response_id, gpointer data) {
  FileDialogGtk* file_dialog = static_cast<FileDialogGtk*>(data);
  file_dialog->HandleResponse(response_id);
}

}  // anonymous namespace

FileDialogGtk::FileDialogGtk() : response_handler_(0) {
}

FileDialogGtk::~FileDialogGtk() {
}

void FileDialogGtk::HandleResponse(gint response_id) {
  gtk_window_set_modal(GTK_WINDOW(dialog_.get()), FALSE);
  g_signal_handler_disconnect(dialog_.get(), response_handler_);
  DoUIAction(UIA_HIDE);

  StringList selected_files;
  std::string16 error;
  if (GTK_RESPONSE_ACCEPT == response_id) {
    if (!ProcessSelection(&selected_files, &error)) {
      HandleError(error);
    }
  }
  CompleteSelection(selected_files);
}

bool FileDialogGtk::BeginSelection(NativeWindowPtr parent,
                                   const FileDialog::Options& options,
                                   std::string16* error) {
  if (!InitDialog(parent, options, error))
    return false;
  if (!SetFilter(options.filter, error))
    return false;
  if (!Display(error))
    return false;
  return true;
}

void FileDialogGtk::CancelSelection() {
  // TODO(bpm): Nothing calls CancelSelection yet, but it might someday.
}

void FileDialogGtk::DoUIAction(UIAction action) {
  switch (action) {
  case UIA_SHOW:
    gtk_widget_show(dialog_.get());
    break;
  case UIA_HIDE:
    gtk_widget_hide(dialog_.get());
    break;
  case UIA_BRING_TO_FRONT:
    gtk_window_present(GTK_WINDOW(dialog_.get()));
    break;
  }
}

bool FileDialogGtk::InitDialog(NativeWindowPtr parent,
                               const FileDialog::Options& options,
                               std::string16* error) {
  dialog_.reset(gtk_file_chooser_dialog_new(
      String16ToUTF8(GetLocalString(SK_OpenFile)).c_str(), parent,
      GTK_FILE_CHOOSER_ACTION_OPEN,
      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
      NULL));
  if (!dialog_.get()) {
    *error = STRING16(L"Failed to create dialog.");
    return false;
  }
  if (parent && parent->group) {
    gtk_window_group_add_window(parent->group, GTK_WINDOW(dialog_.get()));
  }
  bool multipleFiles = (MULTIPLE_FILES == options.mode);
  gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog_.get()),
                                       multipleFiles ? TRUE : FALSE);
  return true;
}

bool FileDialogGtk::SetFilter(const StringList& filter, std::string16* error) {
  GtkFileFilter* gtk_filter = NULL;
  for (size_t i = 0; i < filter.size(); ++i) {
    std::string filter_item;
    if (!String16ToUTF8(filter[i].c_str(), &filter_item))
      continue;
    if (!gtk_filter) {
      gtk_filter = gtk_file_filter_new();
      gtk_file_filter_set_name(gtk_filter, 
          String16ToUTF8(GetLocalString(SK_AllReadableDocuments)).c_str());
    }
    if ('.' == filter_item[0]) {
      std::string pattern("*");
      pattern.append(filter_item);
      gtk_file_filter_add_pattern(gtk_filter, pattern.c_str());
    } else {
      gtk_file_filter_add_mime_type(gtk_filter, filter_item.c_str());
    }
  }  
  if (!gtk_filter)
    return true;

  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog_.get()), gtk_filter);

  // Always include an unrestricted filter that the user may select.
  gtk_filter = gtk_file_filter_new();
  gtk_file_filter_set_name(gtk_filter,
      String16ToUTF8(GetLocalString(SK_AllDocuments)).c_str());
  gtk_file_filter_add_custom(gtk_filter, static_cast<GtkFileFilterFlags>(0),
                             &AnyFileFilter, NULL, &AnyFileFilterDestroy);
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog_.get()), gtk_filter);
  return true;
}

bool FileDialogGtk::Display(std::string16* error) {
  gtk_window_set_modal(GTK_WINDOW(dialog_.get()), TRUE);
  response_handler_ = g_signal_connect(dialog_.get(), "response",
                                       G_CALLBACK(ResponseHandler), this);
  gtk_widget_show_all(dialog_.get());
  return true;
}

bool FileDialogGtk::ProcessSelection(StringList* selected_files,
                                     std::string16* error) {
  GSList* files =
      gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(dialog_.get()));
  if (!files) {
    *error = STRING16(L"Failed to get selected files from dialog.");
    return false;
  }

  std::string16 file;
  for (GSList* curr = files; curr != NULL; curr = curr->next) {
    if (UTF8ToString16(static_cast<const char*>(curr->data), &file)) {
      selected_files->push_back(file);
    } else {
      // Failed to convert string to unicode... ignore it.
    }
    g_free(curr->data);
  }
  g_slist_free(files);
  return true;
}

#endif  // defined(LINUX) && !defined(OS_MACOSX)
