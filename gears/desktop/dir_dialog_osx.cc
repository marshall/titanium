#include "dir_dialog_osx.h"

DirDialogCarbon::DirDialogCarbon () : FileDialogCarbon() { }
DirDialogCarbon::~DirDialogCarbon () { }

static void EventCallBack(NavEventCallbackMessage message,
                          NavCBRecPtr parameters, void* closure) {
  assert(closure);
  FileDialogCarbon* file_dialog = static_cast<FileDialogCarbon*>(closure);
  file_dialog->Event(message, parameters);
}

static Boolean FilterCallback(AEDesc* theItem, void* info,
                              void* closure, NavFilterModes filterMode) {
  assert(closure);
  FileDialogCarbon* file_dialog = static_cast<FileDialogCarbon*>(closure);
  return file_dialog->Filter(theItem, static_cast<NavFileOrFolderInfo*>(info),
                             filterMode);
}

bool DirDialogCarbon::InitDialog(NativeWindowPtr parent,
	const Options& options, std::string16* error)
{
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
  if (noErr != NavCreateChooseFolderDialog(&dialog_options, &EventCallBack,
                                      filter_callback, this,
                                      as_out_parameter(dialog_))) {
    *error = STRING16(L"Failed to create dialog.");
    return false;
  }
  return true;

}

