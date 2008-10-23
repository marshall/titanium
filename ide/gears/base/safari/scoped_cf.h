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

// Scoped objects for the various CoreFoundation types.

#ifndef GEARS_BASE_SAFARI_SCOPED_CF_H__
#define GEARS_BASE_SAFARI_SCOPED_CF_H__

#include <ApplicationServices/ApplicationServices.h>
#include <CoreFoundation/CoreFoundation.h>
#include <Carbon/Carbon.h>
#include <CoreServices/CoreServices.h>

#include "gears/base/common/scoped_token.h"

template<typename CFTokenType>
class CFTokenTraits {
 public:
  static void Free(CFTokenType x) { CFRelease(x); }
  static CFTokenType Default() { return NULL; }
};

template<typename CFTokenType>
class scoped_cftype : public scoped_token<CFTokenType,
                                          CFTokenTraits<CFTokenType> > {
 public:
  scoped_cftype() {
  }
  explicit scoped_cftype(CFTokenType value)
      : scoped_token<CFTokenType, CFTokenTraits<CFTokenType> >(value) {
  }
};

// CFDictionaryRef
typedef scoped_cftype<CFDictionaryRef> scoped_CFDictionary;

// CFHTTPMessageRef
typedef scoped_cftype<CFHTTPMessageRef> scoped_CFHTTPMessage;

// CFMachPortRef
typedef scoped_cftype<CFMachPortRef> scoped_CFMachPort;

// CFMessagePortRef
typedef scoped_cftype<CFMessagePortRef> scoped_CFMessagePort;

// CFMutableDataRef
typedef scoped_cftype<CFMutableDataRef> scoped_CFMutableData;

// CFMutableStringRef
typedef scoped_cftype<CFMutableStringRef> scoped_CFMutableString;

// CFReadStreamRef
typedef scoped_cftype<CFReadStreamRef> scoped_CFReadStream;

// CFRunLoopSourceRef
typedef scoped_cftype<CFRunLoopSourceRef> scoped_CFRunLoopSource;

// CFStringRef
typedef scoped_cftype<CFStringRef> scoped_CFString;

// CFURLRef
typedef scoped_cftype<CFURLRef> scoped_CFURL;

// CFUUIDRef
typedef scoped_cftype<CFUUIDRef> scoped_CFUUID;

// Handle
typedef DECLARE_SCOPED_TRAITS(Handle, DisposeHandle, NULL) HandleTraits;
typedef scoped_token<Handle, HandleTraits> scoped_Handle;

// ComponentInstance
typedef DECLARE_SCOPED_TRAITS(ComponentInstance, CloseComponent, NULL)
    ComponentInstanceTraits;
typedef scoped_token<ComponentInstance, ComponentInstanceTraits>
    scoped_ComponentInstance;

// NavDialogRef
typedef DECLARE_SCOPED_TRAITS(NavDialogRef, NavDialogDispose, NULL)
    NavDialogTraits;
typedef scoped_token<NavDialogRef, NavDialogTraits> scoped_NavDialogRef;

// NavReplyRecord and AEDesc behave differently than most other Carbon objects,
// in that the caller must allocate the structure.  Thus, a simple check for
// a NULL pointer isn't sufficient to determine if the structure requires
// releasing.  Instead, we have a custom check for each type, and we initialize
// the object to a known state that does not require releasing.

// NavReplyRecord
class NavReplyRecordTraits {
 public:
  static void Free(NavReplyRecord* x) {
    if (x->validRecord) {
      NavDisposeReply(x);
    }
  }
  static NavReplyRecord* Default() { return NULL; }
};

class scoped_NavReplyRecord : public scoped_token<NavReplyRecord*,
                                                  NavReplyRecordTraits> {
 public:
  scoped_NavReplyRecord()
      : scoped_token<NavReplyRecord*, NavReplyRecordTraits>(&record_) {
    // Implicitly sets validRecord to false.
    memset(&record_, 0, sizeof(record_));
  }
  scoped_NavReplyRecord(NavReplyRecord* record)
      : scoped_token<NavReplyRecord*, NavReplyRecordTraits>(record) {
  }
 private:
  NavReplyRecord record_;
};

inline NavReplyRecord* as_out_parameter(scoped_NavReplyRecord& p) {
  assert(p.get() && !p.get()->validRecord);
  return p.get();
}  

// AEDesc
class AEDescTraits {
 public:
  static void Free(AEDesc* x) {
    if (typeNull != x->descriptorType) {
      AEDisposeDesc(x);
    }
  }
  static AEDesc* Default() { return NULL; }
};

class scoped_AEDesc : public scoped_token<AEDesc*, AEDescTraits> {
 public:
  scoped_AEDesc() : scoped_token<AEDesc*, AEDescTraits>(&desc_) {
    // Initialize desc_ to a null descriptor.
    AECreateDesc(typeNull, NULL, 0, &desc_);
  }
  scoped_AEDesc(AEDesc* desc) : scoped_token<AEDesc*, AEDescTraits>(desc) {
  }
 private:
  AEDesc desc_;
};

inline AEDesc* as_out_parameter(scoped_AEDesc& p) {
  assert(p.get() && (typeNull == p.get()->descriptorType));
  return p.get();
}

#endif  // GEARS_BASE_SAFARI_SCOPED_CF_H__
