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

#include "gears/desktop/file_dialog.h"

#include "gears/base/common/base_class.h"
#include "gears/base/common/module_wrapper.h"
#include "gears/base/common/js_types.h"
#include "gears/base/common/scoped_refptr.h"
#include "gears/base/common/string_utils.h"
#include "gears/blob/blob.h"
#include "gears/blob/file_blob.h"

#if defined(WIN32)
#include "gears/desktop/file_dialog_win32.h"
#elif defined(OS_MACOSX)
#include "gears/desktop/file_dialog_osx.h"
#elif defined(LINUX)
#include "gears/desktop/file_dialog_gtk.h"
#endif

namespace {

template<class CharT>
inline bool IsCharValidInHttpToken(CharT c) {
  // TODO(bpm): Consider usefulness in string_utils.h.
  // Criteria from RFC 2616, section 2.2.
  // No control characters (or space).
  if (c <= 32 || c >= 127) {
    return false;
  }

  // No separator characters.
  switch (c) {
    case '(':
    case ')':
    case '<':
    case '>':
    case '@':
    case ',':
    case ';':
    case ':':
    case '\\':
    case '"':
    case '/':
    case '[':
    case ']':
    case '?':
    case '=':
    case '{':
    case '}':
    // These last two are included in the above check.
    //case ' ':
    //case '\t':
      return false;

    default:
      return true;
  }
}

const char16 *kFilter = STRING16(L"filter");
const char16 *kSingleFile = STRING16(L"singleFile");

}  // anonymous namespace

Mutex FileDialog::active_mutex_;
FileDialog::ActiveMap FileDialog::active_;

FileDialog* FileDialog::Create(const ModuleImplBaseClass* module) {
  NativeWindowPtr parent = NULL;
  GetBrowserWindow(module, &parent);
#if defined(WIN32)
  FileDialog* dialog = new FileDialogWin32;
#elif defined(OS_MACOSX)
  FileDialog* dialog = new FileDialogCarbon;
#elif defined(LINUX)
  FileDialog* dialog = new FileDialogGtk;
#else
  FileDialog* dialog = NULL;
#endif
  if (dialog)
    dialog->Init(module, parent);
  return dialog;
}

FileDialog::FileDialog() {
}

void FileDialog::Init(const ModuleImplBaseClass* module,
                      NativeWindowPtr parent) {
  module_ = module;
  parent_ = parent;
}

FileDialog::~FileDialog() {
}

bool FileDialog::Open(const FileDialog::Options& options,
                      JsRootedCallback* callback,
                      std::string16* error) {
  assert(callback);
  assert(error);

  // Prevent more than one active dialog per JsContext using active_.
  scoped_refptr<ModuleEnvironment> environment;
  module_->GetModuleEnvironment(&environment);

  MutexLock lock(&active_mutex_);
  ActiveMap::const_iterator it = active_.find(environment.get());
  if (active_.end() != it) {
    // Bring the current dialog to the front, to ensure that the user deals with
    // the dialog.
    it->second->BringToFront();
    *error = STRING16(L"A dialog is currently active.");
    return false;
  }

  callback_.reset(callback);
  if (!BeginSelection(parent_, options, error)) {
    assert(!error->empty());
    return false;
  }

  active_.insert(std::make_pair(environment.get(), this));

  // Subclass will call CompleteSelection when done.
  return true;
}

void FileDialog::Show() {
  DoUIAction(UIA_SHOW);
}

void FileDialog::Hide() {
  DoUIAction(UIA_HIDE);
}

void FileDialog::BringToFront() {
  DoUIAction(UIA_BRING_TO_FRONT);
}

void FileDialog::Cancel() {
  CancelSelection();
}

void FileDialog::CompleteSelection(const StringList& selected_files) {
  scoped_refptr<ModuleEnvironment> environment;
  module_->GetModuleEnvironment(&environment);

  {
    // Allow another dialog to be created in this environment.
    MutexLock lock(&active_mutex_);
    ActiveMap::iterator it = active_.find(environment.get());
    assert(active_.end() != it);
    if (active_.end() != it) {
      assert(it->second == this);
      active_.erase(it);
    }
  }

  // Convert from StringList to a JavaScript array.
  std::string16 error;
  scoped_ptr<JsArray> files_array(module_->GetJsRunner()->NewArray());
  if (!files_array.get()) {
    HandleError(STRING16(L"Failed to create JS array"));
  } else if (!FileDialog::FilesToJsObjectArray(selected_files,
                                               environment.get(),
                                               files_array.get(), &error)) {
    HandleError(error);
  } else {
    // Make the file selection callback.
    JsParamToSend callback_argv[] = { JSPARAM_ARRAY, files_array.get() };
    module_->GetJsRunner()->InvokeCallback(callback_.get(),
                                           ARRAYSIZE(callback_argv),
                                           callback_argv, NULL);
  }

  // FileDialog is no longer needed, so destroy it.
  delete this;
}

void FileDialog::HandleError(const std::string16& error) {
  std::string error8;
  String16ToUTF8(error.c_str(), &error8);
  LOG(("FileDialog error: %s", error8.c_str()));
}

bool FileDialog::IsLegalFilter(const std::string16& filter) {
  // TODO(bpm): Write unit test for this.
  if (filter.empty())
    return false;
  if (L'.' == filter[0]) {
    // Allow legal extensions of the form: .foo
    if (filter.length() < 2)
      return false;
    for (size_t i = 1; i < filter.length(); ++i) {
      if (L'.' == filter[i])
        return false;
      if (!IsCharValidInPathComponent(filter[i], true))
        return false;
    }
  } else {
    // Allow Internet media types of the form: x/y
    bool found_slash = false;
    size_t segment_length[2];
    segment_length[0] = segment_length[1] = 0;
    for (size_t i = 0; i < filter.length(); ++i) {
      if (L'/' == filter[i]) {
        if (found_slash)
          return false;
        found_slash = true;
      } else if (L'*' == filter[i]) {
        // TODO(bpm): Allow media types of the form: x/*
        return false;
      } else if (!IsCharValidInHttpToken(filter[i])) {
        return false;
      } else {
        segment_length[found_slash] += 1;
      }
    }
    if (!segment_length[0] || !segment_length[1])
      return false;
  }
  return true;
}

bool FileDialog::ParseOptions(JsCallContext* context, const JsObject& map,
                              Options* options) {
  // options.filter = [ ".txt", "text/html", "text/*" ];
  if (map.HasProperty(kFilter)) {
    bool success = true;
    int count = 0;
    JsArray filter_array;
    if (!map.GetPropertyAsArray(kFilter, &filter_array)
        || !filter_array.GetLength(&count)) {
      success = false;
    } else {
      for (int i = 0; i < count; ++i) {
        std::string16 filter_item;
        if (!filter_array.GetElementAsString(i, &filter_item)) {
          success = false;
          break;
        }
        if (!IsLegalFilter(filter_item)) {
          std::string16 error = filter_item;
          error += STRING16(L" is not a valid filter.");
          context->SetException(error);
          return false;
        }
        options->filter.push_back(filter_item);
      }
    }
    if (!success) {
      std::string16 error = STRING16(L"options.");
      error += kFilter;
      error += STRING16(L" should be an array of strings.");
      context->SetException(error);
      return false;
    }
  }
  // options.singleFile = true;
  if (map.HasProperty(kSingleFile)) {
    bool singleFile = false;
    if (!map.GetPropertyAsBool(kSingleFile, &singleFile)) {
      std::string16 error = STRING16(L"options.");
      error += kSingleFile;
      error += STRING16(L" should be a boolean.");
      context->SetException(error);
      return false;
    }
    options->mode = singleFile ? SINGLE_FILE : MULTIPLE_FILES;
  }
  return true;
}

bool FileDialog::FilesToJsObjectArray(const StringList& selected_files,
                                      ModuleEnvironment* module_environment,
                                      JsArray* files,
                                      std::string16* error) {
  // selected_files, blobs and base_names are aligned
  const int size = selected_files.size();
  StringList base_names;
  base_names.reserve(size);
  std::vector< scoped_refptr<BlobInterface> > blobs;
  blobs.reserve(size);

  // create all blobs and base names
  for (StringList::const_iterator it = selected_files.begin();
       it != selected_files.end(); ++it) {
    std::string16 base_name;
    if (File::GetBaseName(*it, &base_name)) {
      base_names.push_back(base_name);
      blobs.push_back(new FileBlob(it->c_str()));
    } else {
      *error = STRING16(L"Failed to create blob.");
      return false;
    }
  }

  // create javascript objects and place in javascript array
  for (int i = 0; i < size; ++i) {
    JsRunnerInterface* js_runner = module_environment->js_runner_;
    if (!js_runner) {
      *error = STRING16(L"Failed to get JsRunnerInterface.");
      return false;
    }
    scoped_ptr<JsObject> obj(js_runner->NewObject());
    if (!obj.get()) {
      *error = STRING16(L"Failed to create javascript object.");
      return false;
    }

    if (!obj->SetPropertyString(STRING16(L"name"), base_names[i])) {
      *error = STRING16(L"Failed to set name property on File.");
      return false;
    }

    scoped_refptr<GearsBlob> gears_blob;
    if (!CreateModule<GearsBlob>(module_environment, NULL, &gears_blob)) {
      *error = STRING16(L"GearsBlob creation failed.");
      return false;
    }
    if (!obj->SetPropertyModule(STRING16(L"blob"), gears_blob.get())) {
      *error = STRING16(L"Failed to set blob property on File.");
      return false;
    }
    gears_blob->Reset(blobs[i].get());

    // JsArray takes the javascript token out of JsObject. Hence, the JsObject
    // wrapper around the token still needs to be deleted. This is why obj.get()
    // is used instead of release().
    if (!files->SetElementObject(i, obj.get())) {
      *error = STRING16(L"Failed to add File to array.");
      return false;
    }
  }

  return true;
}
