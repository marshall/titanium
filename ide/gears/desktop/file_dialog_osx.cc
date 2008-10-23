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

#ifdef OS_MACOSX
#include "gears/desktop/file_dialog_osx.h"

#include <algorithm>
#include <sys/syslimits.h>

#include "gears/base/common/string_utils.h"
#include "gears/base/common/common.h"
#include "gears/base/common/string_utils_osx.h"
#include "gears/ui/common/i18n_strings.h"

// In MacOS 10.4 and better, the preferred way to classify filesystem objects
// is via Uniform Type Identifiers (aka, UTIs).  Each UTI has one or more
// distinguishing criteria, such as file extensions, mime type, os types, etc.
// In addition, UTIs participate in a hierarchy.  For example, a file with
// .jpeg extension has UTI type "public.jpeg", which is a subtype of
// "public.image", which is a subtype of "public.data", etc.

// This callback wraps the FileDialogCarbon::Event member, and is called during
// NavDialogRun at various points during the lifetime of the dialog.
static void EventCallBack(NavEventCallbackMessage message,
                          NavCBRecPtr parameters, void* closure) {
  assert(closure);
  FileDialogCarbon* file_dialog = static_cast<FileDialogCarbon*>(closure);
  file_dialog->Event(message, parameters);
}

// This callback wraps the FileDialogCarbon::Filter member, and is called
// during NavDialogRun to determine whether theItem should be selectable based
// on user-selected filter.
static Boolean FilterCallback(AEDesc* theItem, void* info,
                              void* closure, NavFilterModes filterMode) {
  assert(closure);
  FileDialogCarbon* file_dialog = static_cast<FileDialogCarbon*>(closure);
  return file_dialog->Filter(theItem, static_cast<NavFileOrFolderInfo*>(info),
                             filterMode);
}

FileDialogCarbon::FileDialogCarbon() : selected_filter_(0) {
}

FileDialogCarbon::~FileDialogCarbon() {
}

bool FileDialogCarbon::Filter(AEDesc* theItem, NavFileOrFolderInfo* info,
                              NavFilterModes filterMode) {
  // The first filter is numbered 0, which is the only one that filters.
  // The second filter matches everything, so we just return true.
  if (selected_filter_ > 0)  
    return true;

  // On the following lines, we conservatively return true (allow selection)
  // for any error related to this file.
  if (!info || info->isFolder)
    return true;

  scoped_AEDesc fsDesc;
  if (noErr != AECoerceDesc(theItem, typeFSRef, as_out_parameter(fsDesc)))
    return true;

  FSRef fsRef;
  if (noErr != AEGetDescData(fsDesc.get(), &fsRef, sizeof(FSRef)))
    return true;

  // Get the content type (UTI) of this file.
  scoped_cftype<CFTypeRef> uti;
  if ((noErr != LSCopyItemAttribute(&fsRef, kLSRolesViewer, kLSItemContentType,
                                    as_out_parameter(uti)))
      || (NULL == uti.get()))
    return true;

  // Check if the file conforms to any of the UTIs set up in SetFilter.
  CFIndex count = CFArrayGetCount(utis_.get());
  for (CFIndex i = 0; i < count; ++i) {
    const CFStringRef supportedUTI =
        static_cast<const CFStringRef>(CFArrayGetValueAtIndex(utis_.get(), i));
    if (UTTypeConformsTo(static_cast<const CFStringRef>(uti.get()),
                         supportedUTI)) {
      return true;
    }
  }

  // No match, so don't allow the user to select this file.
  return false;
}

void FileDialogCarbon::Event(NavEventCallbackMessage message,
                             NavCBRecPtr parameters) {
  switch (message) {
  case kNavCBPopupMenuSelect:
    // Dynamically change the file filter.
    NavMenuItemSpec* menu = reinterpret_cast<NavMenuItemSpec*>(
        parameters->eventData.eventDataParms.param);
    selected_filter_ = menu->menuType;
    break;
  case kNavCBUserAction:
    if (selected_files_.empty()) {  // Guard against re-entrancy.
      std::string16 error;
      if (!ProcessSelection(&selected_files_, &error)) {
        HandleError(error);
      }
    }
    break;
  case kNavCBTerminate:
    if (dialog_.get()) {  // Guard against re-entrancy.
      // CompleteSelection invokes a JavaScript callback, which may result in a
      // modal window loop (ie, via 'alert()').  If we don't hide the dialog
      // first, FileDialogCarbon::Event can reenter with kNavCBUserAction and
      // kNavCBTerminate events.
      DoUIAction(UIA_HIDE);
      dialog_.reset(NULL);
      CompleteSelection(selected_files_);
    }
    break;
  }
}

bool FileDialogCarbon::BeginSelection(NativeWindowPtr parent,
                                      const FileDialog::Options& options,
                                      std::string16* error) {
  if (!InitDialog(parent, options, error))
    return false;
  if (!Display(error))
    return false;
  return true;
}

void FileDialogCarbon::CancelSelection() {
  // TODO(bpm): Nothing calls CancelSelection yet, but it might someday.
}

void FileDialogCarbon::DoUIAction(UIAction action) { 
  if (WindowRef window = NavDialogGetWindow(dialog_.get())) {
    switch (action) {
    case UIA_SHOW:
      ShowWindow(window);
      break;
    case UIA_HIDE:
      HideWindow(window);
      break;
    case UIA_BRING_TO_FRONT:
      ShowWindow(window);
      SelectWindow(window);
      break;
    }
  }
}

bool FileDialogCarbon::InitDialog(NativeWindowPtr parent,
                                  const Options& options,
                                  std::string16* error) {
  NavDialogCreationOptions dialog_options;
  OSStatus status = NavGetDefaultDialogCreationOptions(&dialog_options);
  if (noErr != status) {
    *error = STRING16(L"Failed to create dialog options.");
    return false;
  }
  if (parent) {
    dialog_options.parentWindow = parent;
    dialog_options.modality = kWindowModalityWindowModal;
  } else {
    dialog_options.modality = kWindowModalityNone;
  }
  if (MULTIPLE_FILES == options.mode) {
    dialog_options.optionFlags |= kNavAllowMultipleFiles;
  } else {
    dialog_options.optionFlags &= ~kNavAllowMultipleFiles;
  }
  if (!SetFilter(options.filter, &dialog_options, error))
    return false;
  NavObjectFilterUPP filter_callback =
      dialog_options.popupExtension ? &FilterCallback : NULL;
  if (noErr != NavCreateGetFileDialog(&dialog_options, NULL, &EventCallBack,
                                      NULL, filter_callback, this,
                                      as_out_parameter(dialog_))) {
    *error = STRING16(L"Failed to create dialog.");
    return false;
  }
  return true;
}

bool FileDialogCarbon::SetFilter(const StringList& filter,
                                 NavDialogCreationOptions* dialog_options,
                                 std::string16* error) {
  // Populate a list of UTIs which will be used for the default filter.
  utis_.reset(CFArrayCreateMutable(NULL, filter.size(),
                                   &kCFTypeArrayCallBacks));
  if (!utis_.get()) {
    *error = STRING16(L"Failed to create CFArrayMutable.");
    return false;
  }
  for (size_t i = 0; i < filter.size(); ++i) {
    CFStringRef tagClass;
    scoped_cftype<CFStringRef> tag;
    if (L'.' == filter[i][0]) {
      // The filter is an extension of the form ".html".
      tagClass = kUTTagClassFilenameExtension;
      tag.reset(CFStringCreateWithCharacters(NULL, filter[i].data() + 1,
          filter[i].length() - 1));
    } else {
      // The filter is a media type of the form "text/html".
      tagClass = kUTTagClassMIMEType;
      tag.reset(CFStringCreateWithCharacters(NULL, filter[i].data(),
          filter[i].length()));
      // TODO(bpm): Handle wildcard media types.
    }
    // The following function will create a dynamic UTI for criteria that does
    // not match any registered UTI.  In the media type case, those UTIs will
    // never match anything, but the dynamic extension UTIs are still useful.
    scoped_cftype<CFStringRef> uti(UTTypeCreatePreferredIdentifierForTag(
        tagClass, tag.get(), NULL));
    CFArrayAppendValue(utis_.get(), uti.get());
  }

  if (0 == CFArrayGetCount(utis_.get()))
    return true;

  // Add entries to the filter drop-down.  The first entry displays any file
  // that conforms to a UTI in utis_.  The second entry displays all files.
  scoped_CFString default_label(CFStringCreateWithString16(
      GetLocalString(SK_AllReadableDocuments).c_str()));
  scoped_CFString all_label(CFStringCreateWithString16(
      GetLocalString(SK_AllDocuments).c_str()));
  const void* kFilterNames[] = {
    default_label.get(),
    all_label.get()
  };
  labels_.reset(CFArrayCreate(NULL, kFilterNames, ARRAYSIZE(kFilterNames),
                              &kCFTypeArrayCallBacks));
  if (!labels_.get()) {
    *error = STRING16(L"Failed to create CFArray.");
    return false;
  }
  dialog_options->popupExtension = labels_.get();
  return true;
}

bool FileDialogCarbon::Display(std::string16* error) {
  OSStatus status = NavDialogRun(dialog_.get());
  if (noErr != status) {
    *error = STRING16(L"Failed to display dialog.");
    return false;
  }
  return true;
}

bool FileDialogCarbon::ProcessSelection(StringList* selected_files,
                                        std::string16* error) {
  // Determine how the user dismissed the dialog.
  NavUserAction action = NavDialogGetUserAction(dialog_.get());
  if (kNavUserActionNone == action  || kNavUserActionCancel == action) {
    // The user did not select files.
    return true;
  }

  // Process the selected files.
  scoped_NavReplyRecord reply_record;
  OSStatus status = NavDialogGetReply(dialog_.get(),
                                      as_out_parameter(reply_record));
  if ((noErr != status) || !reply_record.get()->validRecord) {
    *error = STRING16(L"Failed to get the dialog reply.");
    return false;
  }

  long file_count = 0;
  status = AECountItems(&reply_record.get()->selection, &file_count);
  if (noErr != status) {
    *error = STRING16(L"Failed to get the number of selected files.");
    return false;
  }

  UInt8 file_path_8[PATH_MAX];
  for (long i = 1; i <= file_count; ++i) {
    FSRef file;
    status = AEGetNthPtr(&reply_record.get()->selection, i, typeFSRef, NULL,
                         NULL, &file, sizeof(FSRef), NULL);
    if (noErr != status) {
      *error = STRING16(L"Failed to get selected files.");
      return false;
    }

    // Convert filename to a string16.
    status = FSRefMakePath(&file, file_path_8, ARRAYSIZE(file_path_8));
    if (noErr != status) {
      *error = STRING16(L"Failed to get filename of selected file.");
      return false;
    }
    std::string16 file_path_16;
    if (!UTF8ToString16(reinterpret_cast<const char*>(&file_path_8),
                        &file_path_16)) {
      *error = STRING16(L"Failed to convert string to unicode.");
      return false;
    }
    selected_files->push_back(file_path_16);
  }

  return true;
}

#endif  // OS_MACOSX
