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

#if defined(WIN32)

#include <set>
#include "gears/desktop/file_dialog_win32.h"
#include "gears/base/common/string_utils.h"
#include "gears/ui/common/i18n_strings.h"

namespace {

const char16 kSelectionCompleteTopic[] = L"selection complete";

// Maximum number of characters for the selected path and all filenames,
// combined.
const size_t kFilenameBufferSize = 32768;

// Utility class for passing StringList between threads.
class FileDialogData : public NotificationData {
 public:
  FileDialogData() {}
  virtual ~FileDialogData() {}

  FileDialog::StringList selected_files;

 private:
  // (Copied from geolocation.cc)
  // NotificationData implementation.
  //
  // We do not wish our messages to be delivered across process boundaries. To
  // achieve this, we use an unregistered class ID. On receipt of an IPC
  // message, InboundQueue::ReadOneMessage will attempt to deserialize the
  // message data using CreateAndReadObject. This will fail silently because no
  // factory function is registered for our class ID and Deserialize will not be
  // called.
  virtual SerializableClassId GetSerializableClassId() const {
    return SERIALIZABLE_DESKTOP;
  }
  virtual bool Serialize(Serializer * /* out */) const {
    // The serialized message is not used.
    return true;
  }
  virtual bool Deserialize(Deserializer * /* in */) {
    // This method should never be called.
    assert(false);
    return false;
  }

  DISALLOW_EVIL_CONSTRUCTORS(FileDialogData);
};

void ExitDialog(HWND dlg) {
  ::PostMessage(dlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), NULL);
}

}  // anonymous namespace

FileDialogWin32::FileDialogWin32() : should_exit_(false), wnd_(NULL) {
  static unsigned long id = 0;
  topic_.assign(kSelectionCompleteTopic);
  topic_.append(IntegerToString16(++id));

  // Set up the thread message queue.
  if (!ThreadMessageQueue::GetInstance()->InitThreadMessageQueue()) {
    LOG(("Failed to set up thread message queue.\n"));
    assert(false);
    return;
  }

  MessageService::GetInstance()->AddObserver(this, topic_.c_str());
}

FileDialogWin32::~FileDialogWin32() {
  MessageService::GetInstance()->RemoveObserver(this, topic_.c_str());
}

bool FileDialogWin32::BeginSelection(NativeWindowPtr parent,
                                     const FileDialog::Options& options,
                                     std::string16* error) {
  InitDialog(parent, options);

  std::string16 filter_buffer;
  if (!SetFilter(options.filter, error))
    return false;

  if (0 == Start()) {
    *error = STRING16(L"Thread creation failed.");
    return false;
  }
  return true;
}

void FileDialogWin32::CancelSelection() {
  MutexLock lock(&mutex_);
  if (wnd_) {
    ExitDialog(wnd_);
  } else {
    should_exit_ = true;
  }
}

void FileDialogWin32::DoUIAction(UIAction action) {
  MutexLock lock(&mutex_);
  if (wnd_ && !should_exit_) {
    UINT flags = SWP_NOMOVE | SWP_NOSIZE;
    switch (action) {
    case UIA_SHOW:
      flags |= SWP_NOACTIVATE | SWP_NOZORDER | SWP_SHOWWINDOW;
      break;
    case UIA_HIDE:
      flags |= SWP_NOACTIVATE | SWP_NOZORDER | SWP_HIDEWINDOW;
      break;
    case UIA_BRING_TO_FRONT:
      flags |= SWP_SHOWWINDOW;
      break;
    default:
      assert(false);
      return;
    }
    SetWindowPos(wnd_, HWND_TOP, 0, 0, 0, 0, flags);
  }
}

void FileDialogWin32::Run() {
  scoped_ptr<FileDialogData> data(new FileDialogData);
  std::string16 error;
  if (!Display(&data->selected_files, &error)) {
    HandleError(error);
  }
  MessageService::GetInstance()->NotifyObservers(topic_.c_str(),
                                                 data.release());
}

void FileDialogWin32::OnNotify(MessageService *service,
                               const char16 *topic,
                               const NotificationData *data) {
  assert(topic_ == topic);
  Join();  // The worker thread has completed.
  const FileDialogData* dialog_data = static_cast<const FileDialogData*>(data);
  CompleteSelection(dialog_data->selected_files);
}

// Initialize an open file dialog to open multiple files.
void FileDialogWin32::InitDialog(NativeWindowPtr parent,
                                 const FileDialog::Options& options) {
  filename_buffer_.resize(kFilenameBufferSize);

  // Initialize OPENFILENAME
  memset(&ofn_, 0, sizeof(ofn_));
  ofn_.lStructSize = sizeof(ofn_);
  ofn_.hwndOwner = parent;
  ofn_.lpstrFile = &filename_buffer_[0];
  ofn_.lpstrFile[0] = '\0';
  ofn_.nMaxFile = kFilenameBufferSize;
  ofn_.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER
               | OFN_HIDEREADONLY;
  if (MULTIPLE_FILES == options.mode) {
#ifdef WINCE
    // The native WinCE file picker does not support multi-select.
#else
    ofn_.Flags |= OFN_ALLOWMULTISELECT;
#endif
  }
}

bool FileDialogWin32::SetFilter(const StringList& filter,
                                std::string16* error) {
  std::string16 default_filter;
  std::set<std::string16> filter_types;
  for (StringList::const_iterator it = filter.begin(); it != filter.end();
       ++it) {
    // Handle extensions of the form ".foo".
    if (L'.' == (*it)[0]) {
      default_filter.append(L";*");
      default_filter.append(*it);
      continue;
    }

    // Handle media types of the form "application/foo".
    // Store them in a set for efficient lookup while enumerating the registry.
    // TODO(bpm): Handle wildcard media types.
    filter_types.insert(*it);
  }

  if (!filter_types.empty()) {
    for (DWORD index = 0; true; ++index) {
      TCHAR name[32];  // Ignore any extensions larger than 30 chars.
      DWORD len = ARRAYSIZE(name);
      LONG result = RegEnumKeyEx(HKEY_CLASSES_ROOT, index, name, &len, NULL,
                                 NULL, NULL, NULL);
      if (ERROR_NO_MORE_ITEMS == result)
        break;
      if (ERROR_SUCCESS != result)
        continue;
      if (name[0] != L'.')
        continue;
      HKEY key;
      result = RegOpenKeyEx(HKEY_CLASSES_ROOT, name, 0, KEY_QUERY_VALUE, &key);
      if (ERROR_SUCCESS != result)
        continue;
      DWORD regtype;
      TCHAR content_type[128];  // Ignore content types larger than 127 chars.
      // Note: content_type length is expressed in bytes, not char16s, and may
      // not be null-terminated.
      len = sizeof(content_type) - 2;
      result = RegQueryValueEx(key, L"Content Type", NULL, &regtype,
                               reinterpret_cast<BYTE*>(content_type), &len);
      if ((ERROR_SUCCESS == result) && (REG_SZ == regtype)) {
        // Ensure null-termination. (len in bytes)
        content_type[len / 2] = L'\0';
        if (filter_types.find(content_type) != filter_types.end()) {
          default_filter.append(L";*");
          default_filter.append(name);
        }
      }
      RegCloseKey(key);
    }
  }

  if (default_filter.empty())
    return true;

  filter_.append(GetLocalString(SK_AllReadableDocuments));
  filter_.push_back('\0');
  // Append everything but the first character, which is always ';'.
  filter_.insert(filter_.end(), default_filter.begin() + 1, 
                 default_filter.end());
  filter_.push_back('\0');
  
  // Always include an unrestricted filter that the user may select.
  // On Win32, *.* matches everything, even files with no extension.
  filter_.append(GetLocalString(SK_AllDocuments));
  filter_.push_back('\0');
  filter_.append(STRING16(L"*.*"));
  filter_.push_back('\0');
  
  // Terminate the filter with an extra null.
  filter_.push_back('\0');

  ofn_.lpstrFilter = filter_.c_str();
  ofn_.nFilterIndex = 1;
  return true;
}

bool FileDialogWin32::Display(StringList* selected_files,
                              std::string16* error) {
  if (FAILED(CoInitializeEx(NULL, GEARS_COINIT_THREAD_MODEL))) {
    *error = STRING16("Failed to initialize new thread.");
    return false;
  }
  ofn_.lpfnHook = &FileDialogWin32::HookProc;
  ofn_.lCustData = reinterpret_cast<LPARAM>(this);
  if (ofn_.hwndOwner) {
    // Fake modal behavior by disabling our parent.
    ::EnableWindow(ofn_.hwndOwner, FALSE);
  }
  bool success = (FALSE != ::GetOpenFileName(&ofn_));
  if (ofn_.hwndOwner) {
    ::EnableWindow(ofn_.hwndOwner, TRUE);
  }
  if (success) {
    success = ProcessSelection(selected_files, error);
  }
  ::CoUninitialize();
  return success;
}

UINT_PTR FileDialogWin32::HookProc(HWND hdlg, UINT uiMsg, WPARAM wParam,
                                   LPARAM lParam) {
  if (WM_INITDIALOG == uiMsg) {
    OPENFILENAME* ofn = reinterpret_cast<OPENFILENAME*>(lParam);
    FileDialogWin32* dialog =
        reinterpret_cast<FileDialogWin32*>(ofn->lCustData);
    MutexLock lock(&dialog->mutex_);
    if (dialog->should_exit_) {
      ExitDialog(::GetParent(hdlg));
    } else {
      dialog->wnd_ = ::GetParent(hdlg);
    }
  } else if (WM_NOTIFY == uiMsg) {
    NMHDR* nmhdr = reinterpret_cast<NMHDR*>(lParam);
    if (CDN_INITDONE == nmhdr->code) {
#if 0
      // TODO(bpm): Determine if we want this.
      // This code sets a customized icon for the dialog, and centers it on the
      // screen.
      HWND dialog_handle = ::GetParent(hdlg);
      if (HICON icon = ::LoadIcon(::GetModuleHandle(NULL), L"IDI_ICON")) {
        ::SendMessage(dialog_handle, WM_SETICON, ICON_BIG,
                      reinterpret_cast<LPARAM>(icon));
      }
      RECT desktop_area, dialog_area;
      ::GetWindowRect(::GetDesktopWindow(), &desktop_area);
      desktop_area.right -= desktop_area.left;
      desktop_area.bottom -= desktop_area.top;
      ::GetWindowRect(dialog_handle, &dialog_area);
      dialog_area.right -= dialog_area.left;
      dialog_area.bottom -= dialog_area.top;
      desktop_area.left += (desktop_area.right - dialog_area.right) / 2;
      desktop_area.top += (desktop_area.bottom - dialog_area.bottom) / 2;
      ::SetWindowPos(dialog_handle, NULL,
                     desktop_area.left, desktop_area.top,
                     0, 0, SWP_NOSIZE | SWP_NOZORDER);
#endif
    }
  }
  return 0;
}

bool FileDialogWin32::ProcessSelection(StringList* selected_files,
                                       std::string16* error) {
  StringList files;
  const TCHAR* selection = ofn_.lpstrFile;
  while (*selection) {  // Empty string indicates end of list.
    files.push_back(selection);
    // Skip over filename and null-terminator.
    selection += files.back().length() + 1;
  }
  if (files.empty()) {
    *error = STRING16(L"Selection contained no files. A minimum of one was "
                      L"expected.");
    return false;
  }
  if (files.size() == 1) {
    // When there is one file, it contains the path and filename.
    selected_files->swap(files);
  } else {
    // Otherwise, the first string is the path, and the remainder are filenames.
    StringList::iterator path = files.begin();
    for (StringList::iterator file = path + 1; file != files.end(); ++file) {
      selected_files->push_back(*path + L'\\' + *file);
    }
  }
  return true;
}

#endif  // WIN32
