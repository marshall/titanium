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

// Interface file between the GoogleBreakpad.framework and
// the Inspector process.

#ifndef GEARS_BASE_COMMON_EXCEPTION_HANDLER_OSX_INSPECTOR_H__
#define GEARS_BASE_COMMON_EXCEPTION_HANDLER_OSX_INSPECTOR_H__

#import "gears/base/common/exception_handler_osx/simple_string_dictionary.h"

// Types of mach messsages (message IDs)
enum {
  kMsgType_InspectorInitialInfo = 0,    // data is InspectorInfo
  kMsgType_InspectorKeyValuePair = 1,   // data is KeyValueMessageData
  kMsgType_InspectorAcknowledgement = 2 // no data sent
};

// Initial information sent from the crashed process by
// GoogleBreakpad.framework to the Inspector process
// The mach message with this struct as data will also include
// several descriptors for sending mach port rights to the crashed
// task, etc.
struct InspectorInfo {
  int           exception_type;
  int           exception_code;
  unsigned int  parameter_count;  // key-value pairs
};

// Key/value message data to be sent to the Inspector
struct KeyValueMessageData {
 public:
  KeyValueMessageData() {}
  KeyValueMessageData(const google_breakpad::KeyValueEntry &inEntry) {
    strlcpy(key, inEntry.GetKey(), sizeof(key) );
    strlcpy(value, inEntry.GetValue(), sizeof(value) );
  }

  char key[google_breakpad::KeyValueEntry::MAX_STRING_STORAGE_SIZE];
  char value[google_breakpad::KeyValueEntry::MAX_STRING_STORAGE_SIZE];
};
#endif  // GEARS_BASE_COMMON_EXCEPTION_HANDLER_OSX_INSPECTOR_H__
