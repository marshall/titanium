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

#ifdef OFFICIAL_BUILD
  // The notification API has not been finalized for official builds.
#else

#include <stdio.h>
#ifdef OS_MACOSX
#include <CoreFoundation/CoreFoundation.h>
#include "gears/base/common/string_utils_osx.h"
#endif
#if !BROWSER_NONE
#include "gears/base/common/dispatcher.h"
#include "gears/base/common/js_runner.h"
#include "gears/base/common/module_wrapper.h"
#endif  // !BROWSER_NONE
#include "gears/base/common/security_model.h"
#include "gears/base/common/string_utils.h"
#include "gears/notifier/notification.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

#if !BROWSER_NONE
DECLARE_GEARS_WRAPPER(GearsNotification);

template<>
void Dispatcher<GearsNotification>::Init() {
  RegisterMethod("addAction", &GearsNotification::AddAction);
  RegisterProperty("description", &GearsNotification::GetDescription,
                   &GearsNotification::SetDescription);
  RegisterProperty("displayAtTime", &GearsNotification::GetDisplayAtTime,
                   &GearsNotification::SetDisplayAtTime);
  RegisterProperty("displayUntilTime", &GearsNotification::GetDisplayUntilTime,
                   &GearsNotification::SetDisplayUntilTime);
  RegisterProperty("icon", &GearsNotification::GetIcon,
                   &GearsNotification::SetIcon);
  RegisterProperty("id", &GearsNotification::GetId,
                   &GearsNotification::SetId);
  RegisterProperty("subtitle", &GearsNotification::GetSubtitle,
                   &GearsNotification::SetSubtitle);
  RegisterProperty("title", &GearsNotification::GetTitle,
                   &GearsNotification::SetTitle);
}

const std::string GearsNotification::kModuleName("Notification");
#endif  // !BROWSER_NONE

std::string16 GenerateGuid() {
  std::string16 wide_guid_str;
#ifdef WIN32
  GUID guid = {0};
  HRESULT hr = ::CoCreateGuid(&guid);
  assert(SUCCEEDED(hr));
  char16 wide_guid_buf[40] = {0};
  int num = ::StringFromGUID2(guid,
                              reinterpret_cast<LPOLESTR>(wide_guid_buf),
                              ARRAYSIZE(wide_guid_buf));
  assert(num);
  wide_guid_str = wide_guid_buf + 1;                // remove opening bracket
  wide_guid_str.erase(wide_guid_str.length() - 1);  // remove closing bracket
#elif defined(OS_ANDROID) || (defined(LINUX) && !defined(OS_MACOSX))
  FILE *fptr = fopen("/proc/sys/kernel/random/uuid", "r");
  assert(fptr);
  if (fptr) {
    char guid_str[37] = {0};
    fgets(guid_str, sizeof(guid_str) - 1, fptr);
    fclose(fptr);
    UTF8ToString16(guid_str, &wide_guid_str);
  }
#elif defined(OS_MACOSX)
  CFUUIDRef guid = CFUUIDCreate(NULL);
  CFStringRef guid_cfstr = CFUUIDCreateString(NULL, guid);
  CFStringRefToString16(guid_cfstr, &wide_guid_str);
  CFRelease(guid);
  CFRelease(guid_cfstr);
#else
#error "Unsupported platform."
#endif  // WIN32
  return wide_guid_str;
}

GearsNotification::GearsNotification()
    :
#if !BROWSER_NONE
      ModuleImplBaseClass(kModuleName),
#endif
      version_(kNotificationVersion),
      display_at_time_ms_(0),
      display_until_time_ms_(0),
      send_retries_(0) {
  id_ = GenerateGuid();
}

void GearsNotification::CopyFrom(const GearsNotification& from) {
  version_ = from.version_;
  title_ = from.title_;
  subtitle_ = from.subtitle_;
  icon_url_ = from.icon_url_;
  icon_raw_data_ = from.icon_raw_data_;
  security_origin_.CopyFrom(from.security_origin_);
  id_ = from.id_;
  description_ = from.description_;
  display_at_time_ms_ = from.display_at_time_ms_;
  display_until_time_ms_ = from.display_until_time_ms_;
  actions_ = from.actions_;

  send_retries_ = from.send_retries_;
}

bool GearsNotification::Serialize(Serializer *out) const {
  out->WriteInt(version_);
  out->WriteString(title_.c_str());
  out->WriteString(subtitle_.c_str());
  out->WriteString(icon_url_.c_str());
  out->WriteString(security_origin_.url().c_str());
  out->WriteString(id_.c_str());
  out->WriteString(description_.c_str());
  out->WriteInt64(display_at_time_ms_);
  out->WriteInt64(display_until_time_ms_);
  out->WriteInt(static_cast<int>(actions_.size()));
  for (size_t i = 0; i < actions_.size(); ++i) {
    out->WriteString(actions_[i].text.c_str());
    out->WriteString(actions_[i].url.c_str());
  }
  int icon_byte_count = std::max(static_cast<int>(icon_raw_data_.size()), 0);
  out->WriteInt(icon_byte_count);
  if (icon_byte_count) {
    out->WriteBytes(&icon_raw_data_.at(0), icon_raw_data_.size());
  }
  return true;
}

bool GearsNotification::Deserialize(Deserializer *in) {
  int action_size = 0;
  std::string16 security_origin_url;
  if (!in->ReadInt(&version_) ||
      version_ != kNotificationVersion ||
      !in->ReadString(&title_) ||
      !in->ReadString(&subtitle_) ||
      !in->ReadString(&icon_url_) ||
      !in->ReadString(&security_origin_url) ||
      !in->ReadString(&id_) ||
      !in->ReadString(&description_) ||
      !in->ReadInt64(&display_at_time_ms_) ||
      !in->ReadInt64(&display_until_time_ms_) ||
      !in->ReadInt(&action_size)) {
    return false;
  }
  if (!security_origin_.InitFromUrl(security_origin_url.c_str())) {
    return false;
  }

  for (int i = 0; i < action_size; ++i) {
    NotificationAction action;
    if (!in->ReadString(&action.text) || !in->ReadString(&action.url)) {
      return false;
    }
    actions_.push_back(action);
  }
  int icon_byte_count = 0;
  in->ReadInt(&icon_byte_count);
  if (icon_byte_count != 0 && icon_byte_count != kNotificationIconByteSize) {
    return false;
  }
  icon_raw_data_.resize(icon_byte_count);
  if (icon_byte_count) {
    in->ReadBytes(&icon_raw_data_.at(0), icon_raw_data_.size());
  }
  return true;
}

void GearsNotification::set_icon_raw_data(
    const std::vector<uint8>& icon_raw_data) {
  if (icon_raw_data.size() != static_cast<size_t>(kNotificationIconByteSize)) {
    icon_raw_data_.clear();
    return;
  }
  icon_raw_data_ = icon_raw_data;
}

#if !BROWSER_NONE
void GearsNotification::GetTitle(JsCallContext *context) {
  context->SetReturnValue(JSPARAM_STRING16, &title_);
}

void GearsNotification::SetTitle(JsCallContext *context) {
  std::string16 title;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &title },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set())
    return;

  title_ = title;
}

void GearsNotification::GetIcon(JsCallContext *context) {
  context->SetReturnValue(JSPARAM_STRING16, &icon_url_);
}

void GearsNotification::SetIcon(JsCallContext *context) {
  std::string16 icon;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &icon },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set())
    return;

  icon_url_ = icon;
}

void GearsNotification::GetId(JsCallContext *context) {
  context->SetReturnValue(JSPARAM_STRING16, &id_);
}

void GearsNotification::SetId(JsCallContext *context) {
  std::string16 id;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &id },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set())
    return;

  if (id.empty()) {
    context->SetException(STRING16(L"id can not be empty string."));
    return;
  }
  id_ = id;
}

void GearsNotification::GetSubtitle(JsCallContext *context) {
  context->SetReturnValue(JSPARAM_STRING16, &subtitle_);
}

void GearsNotification::SetSubtitle(JsCallContext *context) {
  std::string16 subtitle;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &subtitle },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set())
    return;

  subtitle_ = subtitle;
}

void GearsNotification::GetDescription(JsCallContext *context) {
  context->SetReturnValue(JSPARAM_STRING16, &description_);
}

void GearsNotification::SetDescription(JsCallContext *context) {
  std::string16 description;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &description },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set())
    return;

  description_ = description;
}

void GearsNotification::GetDisplayAtTime(JsCallContext *context) {
  if (display_at_time_ms_) {
    JsRunnerInterface *js_runner = GetJsRunner();
    scoped_ptr<JsObject> obj(js_runner->NewDate(display_at_time_ms_));
    context->SetReturnValue(JSPARAM_OBJECT, obj.get());
  } else {
    context->SetReturnValue(JSPARAM_NULL, 0);
  }
}

void GearsNotification::SetDisplayAtTime(JsCallContext *context) {
  JsObject obj;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_OBJECT, &obj },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set())
    return;

  JsRunnerInterface *js_runner = GetJsRunner();
  if (!js_runner->ConvertJsObjectToDate(&obj, &display_at_time_ms_)) {
    context->SetException(STRING16(L"failed to set value for displayAtTime."));
    return;
  }
}

void GearsNotification::GetDisplayUntilTime(JsCallContext *context) {
  if (display_until_time_ms_) {
    JsRunnerInterface *js_runner = GetJsRunner();
    scoped_ptr<JsObject> obj(js_runner->NewDate(display_until_time_ms_));
    context->SetReturnValue(JSPARAM_OBJECT, obj.get());
  } else {
    context->SetReturnValue(JSPARAM_NULL, 0);
  }
}

void GearsNotification::SetDisplayUntilTime(JsCallContext *context) {
  JsObject obj;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_OBJECT, &obj },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set())
    return;

  JsRunnerInterface *js_runner = GetJsRunner();
  if (!js_runner->ConvertJsObjectToDate(&obj, &display_until_time_ms_)) {
    context->SetException(
        STRING16(L"failed to set value for displayUntilTime."));
    return;
  }
}

void GearsNotification::AddAction(JsCallContext *context) {
  NotificationAction action;
  JsArgument argv[] = {
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &action.text },
    { JSPARAM_REQUIRED, JSPARAM_STRING16, &action.url },
  };
  context->GetArguments(ARRAYSIZE(argv), argv);
  if (context->is_exception_set())
    return;

  if (action.text.empty()) {
    context->SetException(STRING16(L"text can not be empty string."));
    return;
  }
  if (action.url.empty()) {
    context->SetException(STRING16(L"url can not be empty string."));
    return;
  }
  actions_.push_back(action);
}

#endif  // !BROWSER_NONE

#endif  // OFFICIAL_BUILD
