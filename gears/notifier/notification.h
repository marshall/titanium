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
#ifndef GEARS_NOTIFIER_NOTIFICATION_H__
#define GEARS_NOTIFIER_NOTIFICATION_H__

#ifdef OFFICIAL_BUILD
  // The notification API has not been finalized for official builds.
#else

#include <vector>
#if !BROWSER_NONE
#include "gears/base/common/base_class.h"
#endif
#include "gears/base/common/security_model.h"
#include "gears/base/common/serialization.h"
#include "gears/base/common/string16.h"

struct NotificationAction {
  std::string16 text;
  std::string16 url;
};

const int kNotificationIconDimensions = 48;
const int kNotificationIconByteSize =
    kNotificationIconDimensions * kNotificationIconDimensions * 4;

// Describes the information about a notification.
// Note: do not forget to increase kNotificationVersion if you make any change
// to this class.
class GearsNotification :
#if !BROWSER_NONE
    public ModuleImplBaseClass,
#endif
    public Serializable {
 public:
  static const std::string kModuleName;

  GearsNotification();

  //
  // For JavaScript API
  //

#if !BROWSER_NONE
  // IN: nothing
  // OUT: string
  void GetTitle(JsCallContext *context);

  // IN: string
  // OUT: nothing
  void SetTitle(JsCallContext *context);

  // IN: nothing
  // OUT: string
  void GetIcon(JsCallContext *context);

  // IN: string
  // OUT: nothing
  void SetIcon(JsCallContext *context);

  // IN: nothing
  // OUT: string
  void GetId(JsCallContext *context);

  // IN: string
  // OUT: nothing
  void SetId(JsCallContext *context);

  // IN: nothing
  // OUT: string
  void GetSubtitle(JsCallContext *context);

  // IN: string
  // OUT: nothing
  void SetSubtitle(JsCallContext *context);

  // IN: nothing
  // OUT: string
  void GetDescription(JsCallContext *context);

  // IN: string
  // OUT: nothing
  void SetDescription(JsCallContext *context);

  // IN: nothing
  // OUT: date
  void GetDisplayAtTime(JsCallContext *context);

  // IN: date
  // OUT: nothing
  void SetDisplayAtTime(JsCallContext *context);

  // IN: nothing
  // OUT: date
  void GetDisplayUntilTime(JsCallContext *context);

  // IN: date
  // OUT: nothing
  void SetDisplayUntilTime(JsCallContext *context);

  // IN: string text and string url
  // OUT: nothing
  void AddAction(JsCallContext *context);
#endif  // !BROWSER_NONE

  //
  // Serializable interface
  //

  virtual SerializableClassId GetSerializableClassId() const {
    return SERIALIZABLE_DESKTOP_NOTIFICATION;
  }
  virtual bool Serialize(Serializer *out) const;

  virtual bool Deserialize(Deserializer *in);

  static Serializable *New() {
    return new GearsNotification;
  }

  static void RegisterAsSerializable() {
    Serializable::RegisterClass(SERIALIZABLE_DESKTOP_NOTIFICATION, New);
  }

  //
  // Helpers
  //

  void CopyFrom(const GearsNotification& from);

  bool Matches(const SecurityOrigin &security_origin,
               const std::string16 &id) const {
    return security_origin_.IsSameOrigin(security_origin) && id_ == id;
  }

  //
  // Accessors
  //

  const SecurityOrigin &security_origin() const {
    return security_origin_;
  }
  const std::string16& id() const { return id_; }
  const std::string16& title() const { return title_; }
  const std::string16& subtitle() const { return subtitle_; }
  const std::string16& icon_url() const { return icon_url_; }
  const std::vector<uint8>& icon_raw_data() const {
    return icon_raw_data_;
  }
  const std::string16& description() const { return description_; }
  int64 display_at_time_ms() const { return display_at_time_ms_; }
  int64 display_until_time_ms() const { return display_until_time_ms_; }
  const std::vector<NotificationAction> &actions() const { return actions_; }

  void set_security_origin(const SecurityOrigin& security_origin) {
    security_origin_.CopyFrom(security_origin);
  }
  void set_id(const std::string16& id) { id_ = id; }
  void set_title(const std::string16& title) { title_ = title; }
  void set_subtitle(const std::string16& subtitle) { subtitle_ = subtitle; }
  void set_icon_url(const std::string16& icon_url) { icon_url_ = icon_url; }
  void set_icon_raw_data(const std::vector<uint8>& icon_raw_data);
  void set_description(const std::string16& description) {
    description_ = description;
  }
  void set_display_at_time_ms(int64 display_at_time_ms) {
    display_at_time_ms_ = display_at_time_ms;
  }
  void set_display_until_time_ms(int64 display_until_time_ms) {
    display_until_time_ms_ = display_until_time_ms;
  }
  std::vector<NotificationAction> *mutable_actions() { return &actions_; }

  std::string16* mutable_icon_url() { return &icon_url_; }

  int send_retries() const { return send_retries_; }
  void set_send_retries(int send_retries) { send_retries_ = send_retries; }

 private:
  static const int kNotificationVersion = 5;

  // NOTE: Increase the kNotificationVersion every time the serialization is
  // going to produce different result. This most likely includes any change
  // to data members below.
  int version_;
  SecurityOrigin security_origin_;
  std::string16 id_;
  std::string16 title_;
  std::string16 subtitle_;
  std::string16 icon_url_;
  std::string16 description_;
  int64 display_at_time_ms_;
  int64 display_until_time_ms_;
  // The raw_data is stored in the notification to avoid the size
  // overhead of linking png utils into notifier.exe.
  std::vector<uint8> icon_raw_data_;
  std::vector<NotificationAction> actions_;
  // NOTE: Increase the kNotificationVersion every time the serialization is
  // going to produce different result. This most likely includes any change
  // to data members above.

  // Other data members that are not included in the serialization.
  int send_retries_;

  DISALLOW_EVIL_CONSTRUCTORS(GearsNotification);
};

#endif  // OFFICIAL_BUILD
#endif  // GEARS_NOTIFIER_NOTIFICATION_H__
