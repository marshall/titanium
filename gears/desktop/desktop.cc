// Copyright 2007, Google Inc.
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

#include "gears/desktop/desktop.h"

#include "gears/base/common/common.h"
#include "gears/base/common/dispatcher.h"
#include "gears/base/common/file.h"
#include "gears/base/common/http_utils.h"
#include "gears/base/common/ipc_message_queue.h"
#include "gears/base/common/js_dom_element.h"
#include "gears/base/common/js_runner.h"
#include "gears/base/common/js_types.h"
#include "gears/base/common/module_wrapper.h"
#include "gears/base/common/paths.h"
#include "gears/base/common/permissions_db.h"
#include "gears/base/common/png_utils.h"
#include "gears/base/common/security_model.h"
#include "gears/base/common/serialization.h"
#include "gears/base/common/url_utils.h"
#ifdef BROWSER_WEBKIT
#include "gears/base/safari/curl_downloader.h"
#endif
#include "gears/blob/blob_interface.h"
#include "gears/desktop/drag_and_drop_registry.h"
#include "gears/desktop/file_dialog.h"
#include "gears/desktop/notification_message_orderer.h"
#include "gears/notifier/notification.h"
#include "gears/localserver/common/http_constants.h"
#include "gears/localserver/common/http_request.h"
#include "gears/ui/common/html_dialog.h"

#include "third_party/scoped_ptr/scoped_ptr.h"

DECLARE_GEARS_WRAPPER(GearsDesktop);

template<>
void Dispatcher<GearsDesktop>::Init() {
  RegisterMethod("createShortcut", &GearsDesktop::CreateShortcut);
  RegisterMethod("openFiles", &GearsDesktop::OpenFiles);

#ifdef OFFICIAL_BUILD
  // The notification API has not been finalized for official builds.
#else
#ifdef OS_ANDROID
  // The notification API has not been implemented for Android
#else
  RegisterMethod("createNotification", &GearsDesktop::CreateNotification);
  RegisterMethod("removeNotification", &GearsDesktop::RemoveNotification);
  RegisterMethod("showNotification", &GearsDesktop::ShowNotification);
#endif  // OS_ANDROID
#endif  // OFFICIAL_BUILD

#ifdef OFFICIAL_BUILD
  // The Drag-and-Drop API has not been finalized for official builds.
#else
#if defined(BROWSER_WEBKIT) || defined(OS_ANDROID)
  // The Drag-and-Drop API has not been implemented for Safari and Android.
#else
  RegisterMethod("registerDropTarget", &GearsDesktop::RegisterDropTarget);
#endif  // defined(BROWSER_WEBKIT) || defined(OS_ANDROID)
#endif  // OFFICIAL_BUILD
}


static const int kControlPanelIconDimensions = 16;
#ifdef WIN32
static const PngUtils::ColorFormat kDesktopIconFormat = PngUtils::FORMAT_BGRA;
#else
static const PngUtils::ColorFormat kDesktopIconFormat = PngUtils::FORMAT_RGBA;
#endif

#ifdef OFFICIAL_BUILD
  // The notification API has not been finalized for official builds.
#else
#ifdef OS_ANDROID
  // The notification API has not been implemented for Android.
#else
static NotificationMessageOrderer g_notification_message_orderer;
#endif  // OS_ANDROID
#endif  // OFFICIAL_BUILD

bool DecodeIcon(Desktop::IconData *icon, int expected_size,
                std::string16 *error) {
  // Icons are optional.  Only try to decode if one was provided.
  if (icon->url.empty()) {
    return true;
  }

  // Decode the png
  if (!PngUtils::Decode(&icon->png_data.at(0), icon->png_data.size(),
                        kDesktopIconFormat,
                        &icon->raw_data, &icon->width, &icon->height)) {
    *error = STRING16(L"Could not decode PNG data for icon ");
    *error += icon->url;
    *error += STRING16(L".");
    return false;
  }

  if (icon->width != expected_size || icon->height != expected_size) {
    *error = STRING16(L"Icon ");
    *error += icon->url;
    *error += STRING16(L" has incorrect size. Expected ");
    *error += IntegerToString16(expected_size);
    *error += STRING16(L"x");
    *error += IntegerToString16(expected_size);
    *error += STRING16(L".");
    return false;
  }

  return true;
}

GearsDesktop::GearsDesktop()
    : ModuleImplBaseClass("GearsDesktop") {
#ifdef OFFICIAL_BUILD
  // The notification API has not been finalized for official builds.
#else
  GearsNotification::RegisterAsSerializable();
#endif  // OFFICIAL_BUILD
}

Desktop::Desktop(const SecurityOrigin &security_origin,
                 BrowsingContext *context)
    : security_origin_(security_origin),
      browsing_context_(context) {
}

bool Desktop::ValidateShortcutInfo(ShortcutInfo *shortcut_info,
                                   bool strict_filename) {
  // Gears doesn't allow spaces in path names, but desktop shortcuts are the
  // exception, so instead of rewriting our path validation code, patch a
  // temporary string.
  std::string16 name_without_spaces = shortcut_info->app_name;
  ReplaceAll(name_without_spaces, std::string16(STRING16(L" ")),
             std::string16(STRING16(L"_")));
  if (!strict_filename) {
    // We also disallow any chars above 127, but we optionally make an exception
    // here too.
    for (std::string16::iterator i = name_without_spaces.begin();
         i < name_without_spaces.end(); ++i) {
      if (*i >= 127)
        *i = '_';
    }
  }
  if (!IsUserInputValidAsPathComponent(name_without_spaces, &error_)) {
    return false;
  }

  // Normalize and resolve, in case this is a relative URL.
  if (!ResolveUrl(&shortcut_info->app_url, &error_)) {
    return false;
  }

  // We only allow shortcuts to be created within origin for now.
  if (!security_origin_.IsSameOriginAsUrl(shortcut_info->app_url.c_str())) {
    error_ = STRING16(L"Cannot create cross-origin shortcuts.");
    return false;
  }

  // Validate that we got at least one icon.
  if (shortcut_info->icon16x16.url.empty() &&
      shortcut_info->icon32x32.url.empty() &&
      shortcut_info->icon48x48.url.empty() &&
      shortcut_info->icon128x128.url.empty()) {
    error_ = STRING16(L"Invalid value for icon parameter. At "
                      L"least one icon must be specified.");
    return false;
  }

  // Resolve the icon urls
  if (!shortcut_info->icon16x16.url.empty() &&
      !ResolveUrl(&shortcut_info->icon16x16.url, &error_) ||
      !shortcut_info->icon32x32.url.empty() &&
      !ResolveUrl(&shortcut_info->icon32x32.url, &error_) ||
      !shortcut_info->icon48x48.url.empty() &&
      !ResolveUrl(&shortcut_info->icon48x48.url, &error_) ||
      !shortcut_info->icon128x128.url.empty() &&
      !ResolveUrl(&shortcut_info->icon128x128.url, &error_)) {
    return false;
  }

  // Check if we should display UI.
  return AllowCreateShortcut(*shortcut_info);
}

bool Desktop::InitializeDialog(ShortcutInfo *shortcut_info,
                               HtmlDialog *shortcuts_dialog,
                               DialogStyle style) {
  shortcuts_dialog->arguments = Json::Value(Json::objectValue);

  // Json needs utf8.
  std::string app_url_utf8;
  std::string app_name_utf8;
  std::string app_description_utf8;
  std::string icon16_url_utf8;
  std::string icon32_url_utf8;
  std::string icon48_url_utf8;
  std::string icon128_url_utf8;
  if (!String16ToUTF8(shortcut_info->app_url.c_str(), &app_url_utf8) ||
      !String16ToUTF8(shortcut_info->app_name.c_str(), &app_name_utf8) ||
      !String16ToUTF8(shortcut_info->app_description.c_str(),
                      &app_description_utf8) ||
      !String16ToUTF8(shortcut_info->icon16x16.url.c_str(), &icon16_url_utf8) ||
      !String16ToUTF8(shortcut_info->icon32x32.url.c_str(), &icon32_url_utf8) ||
      !String16ToUTF8(shortcut_info->icon48x48.url.c_str(), &icon48_url_utf8) ||
      !String16ToUTF8(shortcut_info->icon128x128.url.c_str(),
                      &icon128_url_utf8)) {
    error_ = GET_INTERNAL_ERROR_MESSAGE();
    return false;
  }

  // Populate the JSON object we're passing to the dialog.
  shortcuts_dialog->arguments["name"] = Json::Value(app_name_utf8);
  shortcuts_dialog->arguments["link"] = Json::Value(app_url_utf8);
  shortcuts_dialog->arguments["description"] = Json::Value(app_description_utf8);
  shortcuts_dialog->arguments["icon16x16"] = Json::Value(icon16_url_utf8);
  shortcuts_dialog->arguments["icon32x32"] = Json::Value(icon32_url_utf8);
  shortcuts_dialog->arguments["icon48x48"] = Json::Value(icon48_url_utf8);
  shortcuts_dialog->arguments["icon128x128"] = Json::Value(icon128_url_utf8);

  switch (style) {
  case DIALOG_STYLE_STANDARD:
    // We default to the standard style.
    break;
  case DIALOG_STYLE_SIMPLE:
    shortcuts_dialog->arguments["style"] = Json::Value("simple");
    break;
  default:
    assert(false);
    return false;
  }

  return true;
}

bool Desktop::HandleDialogResults(ShortcutInfo *shortcut_info,
                                  HtmlDialog *shortcuts_dialog) {
  bool allow = false;
  bool permanently = false;

  if (shortcuts_dialog->result == Json::Value::null) {
    // A null value is ok.  We interpret it as denying the shortcut temporarily.
    allow = false;
  } else if(shortcuts_dialog->result["allow"].isBool()) {
    allow = shortcuts_dialog->result["allow"].asBool();

    // We interpret an explicit false as permanently denying shortcut creation.
    if (!allow) {
      permanently = true;
    }
  } else {
    assert(false);
    LOG(("CreateShortcut: Unexpected result"));
    return true;
  }

  // Default the locations.
  if (allow) {
    // Ensure the directory we'll be storing the icons in exists.
    std::string16 icon_dir;
    if (!GetDataDirectory(security_origin_, &icon_dir)) {
      error_ = GET_INTERNAL_ERROR_MESSAGE();
      return false;
    }
    AppendDataName(STRING16(L"icons"), kDataSuffixForDesktop, &icon_dir);
    if (!File::RecursivelyCreateDir(icon_dir.c_str())) {
      error_ = GET_INTERNAL_ERROR_MESSAGE();
      return false;
    }
  }

  // TODO(zork): Get the shortcut location from the dialog.
  uint32 locations =
#ifdef WINCE
      // WinCE only supports the start menu.
      SHORTCUT_LOCATION_STARTMENU;
#else
      SHORTCUT_LOCATION_DESKTOP;
#endif

  if (shortcuts_dialog->result["locations"].asBool()) {
    locations = shortcuts_dialog->result["locations"].asInt();
  }
  if (!SetShortcut(shortcut_info, allow, permanently, locations, &error_)) {
    return false;
  }

  return true;
}

void GearsDesktop::CreateShortcut(JsCallContext *context) {
  if (EnvIsWorker()) {
    context->SetException(
                 STRING16(L"createShortcut is not supported in workers."));
    return;
  }

  Desktop::ShortcutInfo shortcut_info;
  JsObject icons;

  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &shortcut_info.app_name },
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &shortcut_info.app_url },
    { JSPARAM_REQUIRED, JSPARAM_OBJECT, &icons },
    { JSPARAM_OPTIONAL, JSPARAM_STRING16, &shortcut_info.app_description },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set()) return;

  if (shortcut_info.app_name.empty()) {
    context->SetException(STRING16(L"Required argument 1 is missing."));
    return;
  }

  if (shortcut_info.app_url.empty()) {
    context->SetException(STRING16(L"Required argument 2 is missing."));
    return;
  }

  // Get the icons the user specified
  icons.GetPropertyAsString(STRING16(L"16x16"), &shortcut_info.icon16x16.url);
  icons.GetPropertyAsString(STRING16(L"32x32"), &shortcut_info.icon32x32.url);
  icons.GetPropertyAsString(STRING16(L"48x48"), &shortcut_info.icon48x48.url);
  icons.GetPropertyAsString(STRING16(L"128x128"),
                            &shortcut_info.icon128x128.url);

  // Prepare the shortcut.
  Desktop desktop(EnvPageSecurityOrigin(), EnvPageBrowsingContext());
  if (!desktop.ValidateShortcutInfo(&shortcut_info, true)) {
    if (desktop.has_error())
      context->SetException(desktop.error());
    return;
  }

  // Show the dialog.
  // TODO(cprince): Consider moving this code to /ui/common/shortcut_dialog.cc
  // to keep it alongside the permission and settings dialog code.  And consider
  // sharing these constants to keep similar dialogs the same size.

  const int kShortcutsDialogWidth = 360;
  const int kShortcutsDialogHeight = 320;

  HtmlDialog shortcuts_dialog;
  if (desktop.InitializeDialog(&shortcut_info, &shortcuts_dialog,
                               Desktop::DIALOG_STYLE_STANDARD)) {
    HtmlDialogReturnValue dialog_result = shortcuts_dialog.DoModal(
        STRING16(L"shortcuts_dialog.html"), kShortcutsDialogWidth,
        kShortcutsDialogHeight);
    if (dialog_result == HTML_DIALOG_SUCCESS) {
      desktop.HandleDialogResults(&shortcut_info, &shortcuts_dialog);
    }
  }

  if (desktop.has_error())
    context->SetException(desktop.error());
}

// Check whether the user has forbidden this shortcut from being created
// permanently.
bool Desktop::AllowCreateShortcut(const Desktop::ShortcutInfo &shortcut_info) {
  PermissionsDB *capabilities = PermissionsDB::GetDB();
  if (!capabilities) {
    return false;
  }

  std::string16 app_url;
  std::string16 icon16x16_url;
  std::string16 icon32x32_url;
  std::string16 icon48x48_url;
  std::string16 icon128x128_url;
  std::string16 msg;
  bool allow_shortcut_creation = false;
  if (!capabilities->GetShortcut(security_origin_,
                                 shortcut_info.app_name.c_str(),
                                 &app_url,
                                 &icon16x16_url,
                                 &icon32x32_url,
                                 &icon48x48_url,
                                 &icon128x128_url,
                                 &msg,
                                 &allow_shortcut_creation)) {
    // If shortcut doesn't exist in DB then it's OK to create it.
    return true;
  }

  return allow_shortcut_creation;
}

void GearsDesktop::OpenFiles(JsCallContext *context) {
  if (EnvIsWorker()) {
    context->SetException(
                 STRING16(L"openFiles is not supported in workers."));
    return;
  }

  scoped_ptr<JsRootedCallback> callback;
  JsObject options_map;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_FUNCTION, as_out_parameter(callback) },
    { JSPARAM_OPTIONAL, JSPARAM_OBJECT, &options_map },
  };
  int argc = context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set()) return;

  // TODO(cdevries): set focus to tab where this function was called

  FileDialog::Options options;
  if (argc > 1) {
    if (!FileDialog::ParseOptions(context, options_map, &options)) {
      assert(context->is_exception_set());
      return;
    }
  }

  std::string16 error;
  scoped_ptr<FileDialog> dialog(FileDialog::Create(this));
  if (!dialog.get()) {
    context->SetException(STRING16(L"Failed to create dialog."));
    return;
  }
  if (!dialog->Open(options, callback.release(), &error)) {
    context->SetException(error);
    return;
  }
  // The dialog will destroy itself when it completes.
  dialog.release();
}

// Handle all the icon creation and creation call required to actually install
// a shortcut.
bool Desktop::SetShortcut(Desktop::ShortcutInfo *shortcut,
                          const bool allow,
                          const bool permanently,
                          uint32 locations,
                          std::string16 *error) {
  PermissionsDB *capabilities = PermissionsDB::GetDB();
  if (!capabilities) {
    *error = GET_INTERNAL_ERROR_MESSAGE();
    return false;
  }

  // Create the database entry.

  // If the user wants to deny shortcut creation permanently then write to the
  // db & return.
  if (!allow && permanently) {
    capabilities->SetShortcut(security_origin_,
                              shortcut->app_name.c_str(),
                              shortcut->app_url.c_str(),
                              shortcut->icon16x16.url.c_str(),
                              shortcut->icon32x32.url.c_str(),
                              shortcut->icon48x48.url.c_str(),
                              shortcut->icon128x128.url.c_str(),
                              shortcut->app_description.c_str(),
                              allow);

    return true;
  }

  // If the user denies shortcut creation temporarily, this is where we bail
  // from the creation logic.
  if (!allow) {
    return true;
  }

  if (!FetchIcon(&shortcut->icon16x16, error, NULL) ||
      !FetchIcon(&shortcut->icon32x32, error, NULL) ||
      !FetchIcon(&shortcut->icon48x48, error, NULL) ||
      !FetchIcon(&shortcut->icon128x128, error, NULL)) {
    return false;
  }

  if (!DecodeIcon(&shortcut->icon16x16, 16, error) ||
      !DecodeIcon(&shortcut->icon32x32, 32, error) ||
      !DecodeIcon(&shortcut->icon48x48, 48, error) ||
      !DecodeIcon(&shortcut->icon128x128, 128, error)) {
    return false;
  }

  // Save the icon file that will be used in the control panel
  if (!WriteControlPanelIcon(*shortcut)) {
    *error = GET_INTERNAL_ERROR_MESSAGE();
    return false;
  }

#if defined(WIN32) || defined(OS_MACOSX)
  const Desktop::IconData *next_largest_provided = NULL;

  // For each icon size, we use the provided one if available.  If not, and we
  // have a larger version, we scale the closest image to fit because our
  // box-filter minify looks better than the automatic minify done by the OS.

  if (!shortcut->icon128x128.raw_data.empty()) {
    next_largest_provided = &shortcut->icon128x128;
  }

  if (shortcut->icon48x48.raw_data.empty()) {
    if (next_largest_provided) {
      shortcut->icon48x48.width = 48;
      shortcut->icon48x48.height = 48;
      PngUtils::ShrinkImage(&next_largest_provided->raw_data.at(0),
                            next_largest_provided->width,
                            next_largest_provided->height, 48, 48,
                            &shortcut->icon48x48.raw_data);
    }
  } else {
    next_largest_provided = &shortcut->icon48x48;
  }

  if (shortcut->icon32x32.raw_data.empty()) {
    if (next_largest_provided) {
      shortcut->icon32x32.width = 32;
      shortcut->icon32x32.height = 32;
      PngUtils::ShrinkImage(&next_largest_provided->raw_data.at(0),
                            next_largest_provided->width,
                            next_largest_provided->height, 32, 32,
                            &shortcut->icon32x32.raw_data);
    }
  } else {
    next_largest_provided = &shortcut->icon32x32;
  }

  if (shortcut->icon16x16.raw_data.empty()) {
    if (next_largest_provided) {
      shortcut->icon16x16.width = 16;
      shortcut->icon16x16.height = 16;
      PngUtils::ShrinkImage(&next_largest_provided->raw_data.at(0),
                            next_largest_provided->width,
                            next_largest_provided->height, 16, 16,
                            &shortcut->icon16x16.raw_data);
    }
  }
#endif  // defined(WIN32) || defined(OS_MACOSX)

  // Create the desktop shortcut using platform-specific code
  assert(allow);
  if (!CreateShortcutPlatformImpl(security_origin_, *shortcut,
                                  locations, error)) {
    return false;
  }

  capabilities->SetShortcut(security_origin_,
                            shortcut->app_name.c_str(),
                            shortcut->app_url.c_str(),
                            shortcut->icon16x16.url.c_str(),
                            shortcut->icon32x32.url.c_str(),
                            shortcut->icon48x48.url.c_str(),
                            shortcut->icon128x128.url.c_str(),
                            shortcut->app_description.c_str(),
                            allow);
  return true;
}

bool Desktop::WriteControlPanelIcon(
                       const Desktop::ShortcutInfo &shortcut) {
  const Desktop::IconData *chosen_icon = NULL;

  // Pick the best icon we can for the control panel
  if (!shortcut.icon16x16.png_data.empty()) {
    chosen_icon = &shortcut.icon16x16;
  } else if (!shortcut.icon32x32.png_data.empty()) {
    chosen_icon = &shortcut.icon32x32;
  } else if (!shortcut.icon48x48.png_data.empty()) {
    chosen_icon = &shortcut.icon48x48;
  } else if (!shortcut.icon128x128.png_data.empty()) {
    chosen_icon = &shortcut.icon128x128;
  } else {
    // Caller should have ensured that there was at least one icon
    assert(false);
  }

  std::string16 icon_loc;
  if (!GetControlPanelIconLocation(security_origin_, shortcut.app_name,
                                   &icon_loc)) {
    return false;
  }

  // Write the control panel icon.
  File::CreateNewFile(icon_loc.c_str());
  return File::WriteVectorToFile(icon_loc.c_str(), &chosen_icon->png_data);
}

bool GetIconFromRequest(HttpRequest *request,
                        Desktop::IconData *icon,
                        std::string16 *error) {
  assert(request && icon && error);
  int status = 0;
  if (!request->GetStatus(&status) ||
      status != HTTPResponse::RC_REQUEST_OK) {
    *error = STRING16(L"Could not load icon ");
    *error += icon->url.c_str();
    *error += STRING16(L".");
    return false;
  }

  // Extract the data.
  scoped_refptr<BlobInterface> blob;
  if (!request->GetResponseBody(&blob)) {
    *error = STRING16(L"Invalid data for icon ");
    *error += icon->url;
    *error += STRING16(L".");
    return false;
  }
  // TODO(bgarcia): Change type of IconData.png_data to Blob.
  //                That will eliminate the copying done here.
  int64 blob_length(blob->Length());
  assert(blob_length >= 0);
  assert(blob_length <= static_cast<int64>(kuint32max));
  icon->png_data.resize(static_cast<size_t>(blob_length));
#ifdef DEBUG
  int64 length = blob->Read(&(icon->png_data[0]), 0, blob_length);
  assert(length == blob_length);
#else
  blob->Read(&(icon->png_data[0]), 0, blob_length);
#endif  // DEBUG
  return true;
}

class IconRequestListener : public HttpRequest::HttpListener,
                            public Desktop::AbortInterface {
 public:
  IconRequestListener(HttpRequest *request,
                      Desktop::IconHandlerInterface *icon_handler)
      : request_(request),
        icon_handler_(icon_handler),
        process_when_done_(false),
        done_(false),
        success_(false) {
    assert(icon_handler_);
    icon_handler->set_abort_interface(this);
    request_->SetListener(this, false);
  }

  void ProcessWhenDone() {
    process_when_done_ = true;
    if (done_) {
      FinishProcess();
    }
  }

  virtual void ReadyStateChanged(HttpRequest *source) {
    HttpRequest::ReadyState state = HttpRequest::UNINITIALIZED;
    source->GetReadyState(&state);
    if (state != HttpRequest::COMPLETE) {
      return;
    }

    std::string16 error;
    if (GetIconFromRequest(source, icon_handler_->mutable_icon(), &error_)) {
      success_ = true;
    }
    done_ = true;
    if (process_when_done_) {
      FinishProcess();
    }
  }

  virtual bool Abort() {
    process_when_done_ = false;
    request_->SetListener(NULL, false);
    bool success = request_->Abort();
    delete this;
    return success;
  }


 private:
  void FinishProcess() {
    // Calling Abort after this point would be harmful.
    icon_handler_->set_abort_interface(NULL);
    icon_handler_->ProcessIcon(success_, error_);
    delete this;
  }

  std::string16 error_;
  scoped_refptr<HttpRequest> request_;
  Desktop::IconHandlerInterface *icon_handler_;
  bool process_when_done_;
  bool done_;
  bool success_;
  DISALLOW_EVIL_CONSTRUCTORS(IconRequestListener);
};

bool Desktop::FetchIcon(Desktop::IconData *icon, std::string16 *error,
                        Desktop::IconHandlerInterface *icon_handler) {
  assert(icon && error);
  assert(!icon_handler || icon == icon_handler->mutable_icon());

  // Icons are optional. Only try to fetch if one was provided.
  if (icon->url.empty()) {
    if (icon_handler) {
      icon_handler->ProcessIcon(false, *error);
    }
    return true;
  }

  if (!icon->png_data.empty()) {
    // Icons can come pre-fetched.  In that case, no need to fetch again.
    if (icon_handler) {
      icon_handler->ProcessIcon(false, *error);
    }
  } else if (IsDataUrl(icon->url.c_str())) {
    // data URL
    std::string16 mime_type, charset;
    if (!ParseDataUrl(icon->url, &mime_type, &charset, &icon->png_data)) {
      *error = STRING16(L"Could not load icon ");
      *error += icon->url.c_str();
      *error += STRING16(L".");
      return false;
    }
    if (icon_handler) {
      icon_handler->ProcessIcon(false, *error);
    }
  } else {
#ifdef BROWSER_WEBKIT
    // Workaround for bug in OS X 10.4, see definition of GetURLData() for
    // details.
    // Get user agent.
    std::string16 user_agent;
    if (!BrowserUtils::GetUserAgentString(&user_agent)) {
      return false;
    }

    if (!GetURLDataAsVector(icon->url, user_agent, &(icon->png_data))) {
      *error = STRING16(L"Could not load icon ");
      *error += icon->url.c_str();
      *error += STRING16(L".");
      return false;
    }
    if (icon_handler) {
      icon_handler->ProcessIcon(true, *error);
    }
#else
    // Fetch the png data
    scoped_refptr<HttpRequest> request;
    HttpRequest::Create(&request);
    bool async = icon_handler != NULL;
    scoped_ptr<IconRequestListener> listener;
    if (async) {
      listener.reset(new IconRequestListener(request.get(), icon_handler));
    }
    request->SetCachingBehavior(HttpRequest::USE_ALL_CACHES);
    request->SetRedirectBehavior(HttpRequest::FOLLOW_ALL);

    if (!request->Open(HttpConstants::kHttpGET, icon->url.c_str(),
                       async, browsing_context_.get()) ||
        !request->Send(NULL)) {
      *error = STRING16(L"Could not load icon ");
      *error += icon->url.c_str();
      *error += STRING16(L".");
      return false;
    }

    if (async) {
      listener->ProcessWhenDone();
      listener.release();
      return true;
    }

    if (!GetIconFromRequest(request.get(), icon, error)) {
      return false;
    }
#endif  // BROWSER_WEBKIT
  }

  return true;
}

// Get the location of one of the icon files stored in the data directory.
bool Desktop::GetControlPanelIconLocation(const SecurityOrigin &origin,
                                          const std::string16 &app_name,
                                          std::string16 *icon_loc) {
  if (!GetDataDirectory(origin, icon_loc)) {
    return false;
  }

  AppendDataName(STRING16(L"icons"), kDataSuffixForDesktop, icon_loc);
  *icon_loc += kPathSeparator;
  *icon_loc += app_name;
  *icon_loc += STRING16(L"_cp");
  *icon_loc += STRING16(L".png");

  return true;
}

bool Desktop::ResolveUrl(std::string16 *url, std::string16 *error) {
  std::string16 full_url;
  if (IsDataUrl(url->c_str()))
    return true;  // don't muck with data URLs

  if (!ResolveAndNormalize(security_origin_.full_url().c_str(),
                           url->c_str(), &full_url)) {
    *error = STRING16(L"Could not resolve url ");
    *error += *url;
    *error += STRING16(L".");
    return false;
  }

  *url = full_url;
  return true;
}

#ifdef OFFICIAL_BUILD
  // The notification API has not been finalized for official builds.
#else
#ifdef OS_ANDROID
  // The notification API has not been implemented for Android.
#else

bool Desktop::ValidateNotification(GearsNotification *notification) {
  assert(!has_error() && notification &&
         !notification->security_origin().initialized());

  // Associate the notification with its security orgin.
  notification->set_security_origin(security_origin_);

  // TODO(levin): validate actions with respect to the secruity origin.

  // Handle the icon url.
  // Normalize and resolve, in case this is a relative URL.
  if (!notification->icon_url().empty()) {
    if (!ResolveUrl(notification->mutable_icon_url(), &error_)) {
      return false;
    }

    // We only allow icons to be retrieved within origin for now.
    if (!IsDataUrl(notification->icon_url().c_str()) &&
        !security_origin_.IsSameOriginAsUrl(notification->icon_url().c_str())) {
      error_ = STRING16(L"Cannot use cross-origin notfications icons.");
      return false;
    }
  }

  return true;
}

class NotificationIconHandler : public Desktop::IconHandlerInterface {
 public:
  explicit NotificationIconHandler(GearsNotification *released_notification)
      : notification_(released_notification),
        reservation_(g_notification_message_orderer.AddReservation(
                         *released_notification)) {
    icon_.url = notification_->icon_url();
  }

  ~NotificationIconHandler() {
    if (reservation_ != NotificationMessageOrderer::kInvalidReservation) {
      g_notification_message_orderer.RemoveReservation(reservation_);
    }
  }

  virtual void set_abort_interface(Desktop::AbortInterface *) {
    // Not used for now.
  }

  virtual Desktop::IconData *mutable_icon() {
    return &icon_;
  }

  virtual void ProcessIcon(bool success,
                           const std::string16 &icon_error) {
    scoped_ptr<NotificationIconHandler> delete_me(this);
    if (!icon_error.empty()) {
      assert(!success);
      LogError(STRING16(L"fetch icon"), icon_error.c_str());
    }

    std::string16 error;
    if (success) {
      if (DecodeIcon(&icon_, kNotificationIconDimensions, &error)) {
        notification_->set_icon_raw_data(icon_.raw_data);
      } else {
        LogError(STRING16(L"decode icon"), error.c_str());
        error.clear();
      }
    }

    g_notification_message_orderer.PostNotification(kDesktop_AddNotification,
                                                    notification_.release(),
                                                    reservation_);
    reservation_ = NotificationMessageOrderer::kInvalidReservation;
  }

 private:
  void LogError(const char16 *type, const char16 *error) {
    std::string origin;
    String16ToUTF8(notification_->security_origin().url().c_str(), &origin);
    std::string id;
    String16ToUTF8(notification_->id().c_str(), &id);
    std::string type8;
    String16ToUTF8(type, &type8);
    std::string error8;
    String16ToUTF8(error, &error8);
    LOG(("Notification (origin=%s, id=%s) %s error: %s",
         origin.c_str(), id.c_str(), type8.c_str(), error8.c_str()));
  }
  scoped_ptr<GearsNotification> notification_;
  int64 reservation_;
  Desktop::IconData icon_;
  DISALLOW_EVIL_CONSTRUCTORS(NotificationIconHandler);
};

bool Desktop::ValidateAndShowNotification(
    GearsNotification *released_notification) {
  assert(released_notification);
  if (!ValidateNotification(released_notification)) {
    delete released_notification;
    return false;
  }

  // Don't create icon_handler until released_notification
  // has its url resolved (inside of ValidateNotification).
  NotificationIconHandler *icon_handler =
      new NotificationIconHandler(released_notification);
  if (!FetchIcon(icon_handler->mutable_icon(), &error_, icon_handler)) {
    return false;
  }
  return true;
}

bool Desktop::ValidateAndRemoveNotification(
    GearsNotification *released_notification) {
  assert(released_notification);
  scoped_ptr<GearsNotification> notification(released_notification);
  if (!ValidateNotification(notification.get())) {
    return false;
  }
  g_notification_message_orderer.PostNotification(
      kDesktop_RemoveNotification,
      notification.release(),
      NotificationMessageOrderer::kInvalidReservation);
  return true;
}

void GearsDesktop::CreateNotification(JsCallContext *context) {
  scoped_refptr<GearsNotification> notification;
  if (!CreateModule<GearsNotification>(module_environment_.get(),
                                       context, &notification)) {
    return;
  }
  context->SetReturnValue(JSPARAM_MODULE, notification.get());
}

void GearsDesktop::ShowNotification(JsCallContext *context) {
  ModuleImplBaseClass *module = NULL;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_MODULE, &module },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set()) return;
  if (GearsNotification::kModuleName != module->get_module_name()) {
    context->SetException(STRING16(L"First argument must be a notification."));
    return;
  }
  scoped_ptr<GearsNotification> notification(new GearsNotification());
  notification->CopyFrom(*(static_cast<GearsNotification*>(module)));

  Desktop desktop(EnvPageSecurityOrigin(), EnvPageBrowsingContext());
  if (!desktop.ValidateAndShowNotification(notification.release())) {
    if (desktop.has_error())
      context->SetException(desktop.error());
    return;
  }
}

void GearsDesktop::RemoveNotification(JsCallContext *context) {
  std::string16 id;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &id },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set()) return;

  if (id.empty()) {
    context->SetException(
        STRING16(L"Cannot remove the notification with empty id."));
    return;
  }
  scoped_ptr<GearsNotification> notification(new GearsNotification());
  notification->set_id(id);

  Desktop desktop(EnvPageSecurityOrigin(), EnvPageBrowsingContext());
  if (!desktop.ValidateAndRemoveNotification(notification.release())) {
    if (desktop.has_error())
      context->SetException(desktop.error());
    return;
  }
}
#endif  // OS_ANDROID
#endif  // OFFICIAL_BUILD

#ifdef OFFICIAL_BUILD
// The Drag-and-Drop API has not been finalized for official builds.
#else
#if defined(BROWSER_WEBKIT) || defined(OS_ANDROID)
  // The Drag-and-Drop API has not been implemented for Safari and Android.
#else
void GearsDesktop::RegisterDropTarget(JsCallContext *context) {
  if (EnvIsWorker()) {
    context->SetException(
                 STRING16(L"registerDropTarget is not supported in workers."));
    return;
  }

  JsDomElement dom_element;
  JsObject drag_drop_options;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_DOM_ELEMENT, &dom_element },
    { JSPARAM_REQUIRED, JSPARAM_OBJECT, &drag_drop_options },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set()) return;

  std::string16 error;
  if (!DragAndDropRegistry::RegisterDropTarget(this,
                                               dom_element,
                                               drag_drop_options,
                                               &error)) {
    context->SetException(error);
    return;
  }
}
#endif  // defined(BROWSER_WEBKIT) || defined(OS_ANDROID)
#endif  // OFFICIAL_BUILD
