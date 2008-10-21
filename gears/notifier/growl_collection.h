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

#ifndef GEARS_NOTIFIER_GROWL_COLLECTION_H__
#define GEARS_NOTIFIER_GROWL_COLLECTION_H__
#ifdef OFFICIAL_BUILD
  // The notification API has not been finalized for official builds.
#else
#if defined(OS_MACOSX)
// for CFStringRef (non-Objective-C for notification_manager.cc)
#include <CoreFoundation/CoreFoundation.h>

#include <map>

#include "gears/base/common/string16.h"
#include "gears/notifier/balloons.h"

class GearsNotification;
class SecurityOrigin;

// A display class for notification_manager which displays the balloons
// using Growl instead of Glint.
// It switches with BalloonCollection in balloons.cc.
class GrowlBalloonCollection : public BalloonCollectionInterface {
 public:
  GrowlBalloonCollection();
  virtual ~GrowlBalloonCollection();

  virtual void Add(const GearsNotification &notification);
  virtual bool Update(const GearsNotification &notification);
  virtual bool Delete(const SecurityOrigin &security_origin,
                      const std::string16 &bare_id);
  virtual void ShowAll() {}
  virtual void HideAll() {}

  virtual bool HasSpace() const;
  virtual int count() const;
  virtual const GearsNotification *notification_at(int i) const;

  // CF instead of NS so that we don't have to include Foundation
  // and compile anything that includes this as Objective-C++...
  void NotificationClicked(CFStringRef id, CFStringRef url);
  void NotificationTimedOut(CFStringRef id, CFStringRef url);
  bool DeleteWithURL(const std::string16 &url,
                     const std::string16 &bare_id);
 private:

  typedef std::pair<std::string16, std::string16> NotificationId;

  std::map<NotificationId, GearsNotification*> active_notifications_;
  DISALLOW_EVIL_CONSTRUCTORS(GrowlBalloonCollection);
};
#endif  // defined(OS_MACOSX)
#endif  // OFFICIAL_BUILD
#endif  // GEARS_NOTIFIER_GROWL_COLLECTION_H__
