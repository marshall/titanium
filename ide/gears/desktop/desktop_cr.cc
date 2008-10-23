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

#include "gears/desktop/desktop_cr.h"

#include "gears/base/chrome/browsing_context_cr.h"
#include "gears/base/chrome/module_cr.h"
#include "gears/base/common/string_utils.h"
#include "gears/desktop/desktop.h"
#include "gears/ui/common/html_dialog.h"

// We keep track of the set of URLs that have open shortcuts dialogs, so we
// don't open 2 at once.
static std::set<std::string16> *g_open_shortcuts_dialogs;

static void RegisterOpenDialog(const std::string16 &url) {
  if (!g_open_shortcuts_dialogs)
    g_open_shortcuts_dialogs = new std::set<std::string16>;
  g_open_shortcuts_dialogs->insert(url);
}

static void UnregisterOpenDialog(const std::string16 &url) {
  assert(g_open_shortcuts_dialogs);
  g_open_shortcuts_dialogs->erase(url);
  if (g_open_shortcuts_dialogs->empty()) {
    delete g_open_shortcuts_dialogs;
    g_open_shortcuts_dialogs = NULL;
  }
}

// Like UTF8ToString16, but treats NULL as "".
static bool MaybeUTF8ToString16(const char *in, std::string16 *out) {
  return in == NULL || UTF8ToString16(in, out);
}

// Handles the results of the async icon fetch.  It relies on
// ModelessShortcutsDialog to manage its lifetime.
class ShortcutIconHandler : public Desktop::IconHandlerInterface {
 public:
  ShortcutIconHandler(ModelessShortcutsDialog *shortcut_dialog,
                      int index)
      : shortcut_dialog_(shortcut_dialog),
        index_(index),
        abort_interface_(NULL) {
    assert(0 <= index_ && index_ < 4);
  }

  ~ShortcutIconHandler() {
    Abort();
  }

  virtual void ProcessIcon(bool success,
                           const std::string16 &icon_error) {
    // TODO(mpcomplete): handle error?
    // Specifically when !success.
    shortcut_dialog_->IconRequestFinished(index_);
  }

  virtual Desktop::IconData *mutable_icon() {
    switch (index_) {
      case 0:
        return &shortcut_dialog_->shortcut_info_.icon16x16;
      case 1:
        return &shortcut_dialog_->shortcut_info_.icon32x32;
      case 2:
        return &shortcut_dialog_->shortcut_info_.icon48x48;
      case 3:
        return &shortcut_dialog_->shortcut_info_.icon128x128;
    }
    return NULL;
  }

  virtual void set_abort_interface(Desktop::AbortInterface *abort_interface) {
    abort_interface_ = abort_interface;
  }

  bool Abort() {
    if (!abort_interface_) {
      return true;
    }
    return abort_interface_->Abort();
  }

 private:
  ModelessShortcutsDialog *shortcut_dialog_;
  int index_;
  Desktop::AbortInterface *abort_interface_;
  DISALLOW_EVIL_CONSTRUCTORS(ShortcutIconHandler);
};

// Simply here so that ShortcutIconHandler can be forward
// declared in the header file.
ModelessShortcutsDialog::ModelessShortcutsDialog(CPBrowsingContext context)
    : browsing_context_(context), results_ready_(false),
      shortcut_data_(NULL) {
}

ModelessShortcutsDialog::~ModelessShortcutsDialog() {
}

static void ResultsReadyCallback(Json::Value *result, void *closure) {
  ModelessShortcutsDialog* dialog =
      static_cast<ModelessShortcutsDialog *>(closure);
  dialog->ResultsReady();
}

bool ModelessShortcutsDialog::ShowDialog(GearsShortcutData2 *shortcut_data) {
  const char* name = CP_GET_MINOR_VERSION(CP::version()) >= 6 ?
      shortcut_data->orig_name : shortcut_data->name;
  if (!UTF8ToString16(name, &shortcut_info_.app_name) ||
      !UTF8ToString16(shortcut_data->url, &shortcut_info_.app_url) ||
      !MaybeUTF8ToString16(shortcut_data->icons[0].url,
                           &shortcut_info_.icon16x16.url) ||
      !MaybeUTF8ToString16(shortcut_data->icons[1].url,
                           &shortcut_info_.icon32x32.url) ||
      !MaybeUTF8ToString16(shortcut_data->icons[2].url,
                           &shortcut_info_.icon48x48.url) ||
      !MaybeUTF8ToString16(shortcut_data->icons[3].url,
                           &shortcut_info_.icon128x128.url) ||
      !MaybeUTF8ToString16(shortcut_data->description,
                           &shortcut_info_.app_description)) {
    return false;
  }

  EnsureStringValidPathComponent(shortcut_info_.app_name, false);

  // Only allow one dialog for this URL.
  if (g_open_shortcuts_dialogs &&
      g_open_shortcuts_dialogs->find(shortcut_info_.app_url) !=
          g_open_shortcuts_dialogs->end())
    return false;

  SecurityOrigin security_origin;
  if (!security_origin.InitFromUrl(shortcut_info_.app_url.c_str()))
    return false;

  // TODO(mpcomplete): We'll probably want to override behavior if the
  // shortcut exists - ie, show an edit dialog.
  // TODO(mpcomplete): The browsing_context Chrome gives us here is an HWND.
  // We need to associate it with an URLRequestContext.
  scoped_refptr<CRBrowsingContext> cr_context(
      new CRBrowsingContext(browsing_context_));
  desktop_.reset(new Desktop(security_origin, cr_context.get()));
  dialog_.reset(new HtmlDialog(cr_context.get()));

  const int kShortcutsDialogWidth = 360;
  const int kShortcutsDialogHeight = 240;

  if (!desktop_->ValidateShortcutInfo(&shortcut_info_, false) ||
      !PrefetchIcons() ||
      !desktop_->InitializeDialog(&shortcut_info_,
                                  dialog_.get(),
                                  Desktop::DIALOG_STYLE_SIMPLE) ||
      dialog_->DoModeless(STRING16(L"shortcuts_dialog.html"),
                          kShortcutsDialogWidth, kShortcutsDialogHeight,
                          ResultsReadyCallback, this) != HTML_DIALOG_SUCCESS)
    return false;

  shortcut_data_ = shortcut_data;

  // Keep track of this dialog being open.
  RegisterOpenDialog(shortcut_info_.app_url);
  return true;
}

void ModelessShortcutsDialog::ResultsReady() {
  results_ready_ = true;

  if (dialog_->result == Json::Value::null ||
      !dialog_->result["allow"].isBool() ||
      !dialog_->result["allow"].asBool()) {
    // The user doesn't want to create the shortcut, so cancel the icon fetch
    // and handle the results immediately.
    AbortPrefetchIcons();
    HandleDialogResults();
    return;
  }

  if (AreIconsReady() && results_ready_)
    HandleDialogResults();
}

bool ModelessShortcutsDialog::PrefetchIcons() {
  std::string16 error;
  for (size_t i = 0; i < 4; ++i) {
    icon_handler_[i].reset(new ShortcutIconHandler(this, i));
    if (!desktop_->FetchIcon(icon_handler_[i]->mutable_icon(), &error,
                             icon_handler_[i].get())) {
      return false;
    }
  }

  return true;
}

void ModelessShortcutsDialog::AbortPrefetchIcons() {
  for (size_t i = 0; i < 4; ++i) {
    if (icon_handler_[i].get()) {
      // Deleting icon handlers will abort any pending
      // fetches.
      icon_handler_[i].reset();
    }
  }
}

void ModelessShortcutsDialog::IconRequestFinished(int index) {
  // Figure out which icon this fetch was for, and mark the fetch as done by
  // clearing it.
  icon_handler_[index].reset();
  if (AreIconsReady() && results_ready_)
    HandleDialogResults();
}

bool ModelessShortcutsDialog::AreIconsReady() {
  for (size_t i = 0; i < 4; ++i) {
    if (icon_handler_[i].get())
      return false;
  }

  return true;
}

void ModelessShortcutsDialog::HandleDialogResults() {
  bool success = desktop_->HandleDialogResults(&shortcut_info_, dialog_.get());
  UnregisterOpenDialog(shortcut_info_.app_url);

  bool allow = dialog_->result["allow"].asBool();

  // Notify Chrome that the user's choice is ready.
  GearsCreateShortcutResult result = {
    shortcut_data_,
    (allow && success) ? CPERR_SUCCESS : CPERR_FAILURE
  };
  CP::browser_funcs().handle_command(
      CP::cpid(), browsing_context_,
      GEARSBROWSERCOMMAND_CREATE_SHORTCUT_DONE, &result);

  delete this;
}

// Helper to convert a Desktop::ShortcutInfo to the right format for Chrome.
static void ConvertToGearsShortcutData(
    const Desktop::ShortcutInfo &info,
    GearsShortcutData *shortcut_data) {
  memset(shortcut_data, 0, sizeof(*shortcut_data));
  shortcut_data->url = CP::String16ToUTF8Dup(info.app_url);
  shortcut_data->name = CP::String16ToUTF8Dup(info.app_name);
  shortcut_data->description = CP::String16ToUTF8Dup(info.app_description);
  if (!info.icon16x16.url.empty()) {
    shortcut_data->icons[0].width = shortcut_data->icons[0].height = 16;
    shortcut_data->icons[0].url = CP::String16ToUTF8Dup(info.icon16x16.url);
  }
  if (!info.icon32x32.url.empty()) {
    shortcut_data->icons[1].width = shortcut_data->icons[1].height = 32;
    shortcut_data->icons[1].url = CP::String16ToUTF8Dup(info.icon32x32.url);
  }
  if (!info.icon48x48.url.empty()) {
    shortcut_data->icons[2].width = shortcut_data->icons[2].height = 48;
    shortcut_data->icons[2].url = CP::String16ToUTF8Dup(info.icon48x48.url);
  }
  if (!info.icon128x128.url.empty()) {
    shortcut_data->icons[3].width = shortcut_data->icons[3].height = 128;
    shortcut_data->icons[3].url = CP::String16ToUTF8Dup(info.icon128x128.url);
  }
}

bool GetAllShortcuts(GearsShortcutList *shortcut_list) {
  // Build the list in a temporary vector for ease of use.
  std::vector<GearsShortcutData> shortcuts;
  size_t num_shortcuts = 0;

  PermissionsDB *permissions = PermissionsDB::GetDB();
  if (!permissions) {
    return false;
  }

  std::vector<SecurityOrigin> origins;
  if (!permissions->GetOriginsWithShortcuts(&origins)) {
    LOG16((L"GetOriginsWithShortcuts() failed"));
    return false;
  }

  for (size_t i = 0; i < origins.size(); ++i) {
    std::vector<std::string16> names;
    if (!permissions->GetOriginShortcuts(origins[i], &names)) {
      LOG16((L"GetOriginShortcuts(%s) failed", origins[i].full_url().c_str()));
      continue;
    }

    // This is a conservative resize (since we skip disallowed shortcuts).
    shortcuts.resize(num_shortcuts + names.size());
    for (size_t j = 0; j < names.size(); ++j) {
      Desktop::ShortcutInfo info;
      bool allow = false;
      if (!permissions->GetShortcut(origins[i], names[j].c_str(),
                                    &info.app_url,
                                    &info.icon16x16.url,
                                    &info.icon32x32.url,
                                    &info.icon48x48.url,
                                    &info.icon128x128.url,
                                    &info.app_description,
                                    &allow)) {
        LOG16((L"GetShortcut(%s) failed",
               origins[i].full_url().c_str(), names[j].c_str()));
        continue;
      }
      if (!allow)
        continue;  // Skip disallowed shortcuts.

      info.app_name = names[j];
      ConvertToGearsShortcutData(info, &shortcuts[num_shortcuts++]);
    }
  }

  // Copy back out of our temporary vector.
  shortcut_list->num_shortcuts = num_shortcuts;
  if (num_shortcuts > 0) {
    shortcut_list->shortcuts = CP::Alloc<GearsShortcutData>(num_shortcuts);
    memcpy(shortcut_list->shortcuts, &shortcuts[0],
           sizeof(GearsShortcutData) * num_shortcuts);
  } else {
    shortcut_list->shortcuts = NULL;
  }
  return true;
}
