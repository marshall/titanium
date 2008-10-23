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

#ifndef GEARS_CONSOLE_LOG_EVENT_H__
#define GEARS_CONSOLE_LOG_EVENT_H__

#include "gears/base/common/basictypes.h"
#include "gears/base/common/message_service.h"
#include "gears/base/common/stopwatch.h"
#include "gears/base/common/string16.h"


// LogEvent is a simple class that encapsulates information about a single
// log event.
class LogEvent : public NotificationData {
 public:
  LogEvent(const std::string16 &message,
           const std::string16 &type,
           const std::string16 &source_url)
   : message_(message), type_(type), source_url_(source_url),
     date_(GetCurrentTimeMillis()) { }
  
  const std::string16& message() const { return message_; }
  const std::string16& type() const { return type_; }
  const std::string16& sourceUrl() const { return source_url_; }
  const int64& date() const { return date_; }

  virtual SerializableClassId GetSerializableClassId() const {
    return SERIALIZABLE_CONSOLE_LOG_EVENT;
  }
  virtual bool Serialize(Serializer *out) const {
    out->WriteString(message_.c_str());
    out->WriteString(type_.c_str());
    out->WriteString(source_url_.c_str());
    out->WriteInt64(date_);
    return true;
  }
  virtual bool Deserialize(Deserializer *in) {
    return in->ReadString(&message_) &&
           in->ReadString(&type_) &&
           in->ReadString(&source_url_) &&
           in->ReadInt64(&date_);
  }
  static void RegisterLogEventClass() {
    Serializable::RegisterClass(SERIALIZABLE_CONSOLE_LOG_EVENT, New);
  }  
 private:
  std::string16 message_;
  std::string16 type_;        // 'debug', 'info', 'warn', 'error'
  std::string16 source_url_;  
  int64 date_;                // Time in milliseconds since 1/1/1970 UTC

  LogEvent() : date_(0) {}

  static Serializable *New() {
    return new LogEvent;
  }
};

#endif // GEARS_CONSOLE_LOG_EVENT_H__
