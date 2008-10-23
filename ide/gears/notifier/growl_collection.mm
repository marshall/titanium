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

#import "gears/notifier/growl_collection.h"

#import <Cocoa/Cocoa.h>

#import "gears/base/common/string_utils_osx.h"  // for CFStringRefToString16
#import "gears/notifier/balloons.h"
#import "gears/notifier/notification.h"
#import "gears/notifier/notifier_pref_common.h"
#import "gears/notifier/nsstring_string16.h"  // for GMStringWithString16
#import "third_party/growl/GrowlApplicationBridge.h"

NSData *TIFFDataFromRawBitmap(const std::vector<uint8> icon_data_vector);

// for click context dictionary
#define ID_KEY  @"NOTIFICATION_ID"
#define URL_KEY @"NOTIFICATION_URL"

// Called by GrowlApplicationBridge to determine what notifications we send
// when we register with Growl. It also provides callbacks when notifications
// are clicked.
// These methods are defined by the Growl framework:
// http://growl.info/documentation/developer/
@interface GBCGrowlDelegate : NSObject <GrowlApplicationBridgeDelegate> {
  GrowlBalloonCollection *growl_collection_;
}
// Designated initializer, don't call init.
- (id)initWithGrowlBalloonCollection:(GrowlBalloonCollection *)growl_collection;

// Growl calls this to figure out what notifications we support.
- (NSDictionary *)registrationDictionaryForGrowl;

// Click (or non-click) callbacks called by Growl.
- (void)growlNotificationWasClicked:(id)clickContext;
- (void)growlNotificationTimedOut:(id)clickContext;

@end

@implementation GBCGrowlDelegate

- (id)initWithGrowlBalloonCollection:(GrowlBalloonCollection *)growl_collection {
  if ((self = [super init])) {
    growl_collection_ = growl_collection;
  }
  return self;
}

- (NSDictionary *)registrationDictionaryForGrowl {
  // TODO(chimene): Internationalize this!
  NSArray *names = [NSArray arrayWithObject:(NSString *)kGrowlNotificationName];

  // Init with value1, key1, value2, key2, nil
  // TODO(chimene): Internationalize this!
  NSDictionary *dict = [NSDictionary dictionaryWithObjectsAndKeys:
      (NSString *)kGrowlNotificationName, GROWL_APP_NAME,
      (NSString *)kGearsNotifierAppID,    GROWL_APP_ID,
      names,                              GROWL_NOTIFICATIONS_ALL,
      names,                              GROWL_NOTIFICATIONS_DEFAULT,
      nil];

  return dict;
}

- (void)growlNotificationWasClicked:(id)clickContext {
  NSDictionary *context = (NSDictionary *)clickContext;

  NSString *notification_id  = (NSString *)[context objectForKey:ID_KEY];
  NSString *notification_url = (NSString *)[context objectForKey:URL_KEY];

  assert(growl_collection_);
  growl_collection_->NotificationClicked((CFStringRef)notification_id,
                                         (CFStringRef)notification_url);
}

- (void)growlNotificationTimedOut:(id)clickContext {
  NSDictionary *context = (NSDictionary *)clickContext;

  NSString *notification_id  = (NSString *)[context objectForKey:ID_KEY];
  NSString *notification_url = (NSString *)[context objectForKey:URL_KEY];

  assert(growl_collection_);
  growl_collection_->NotificationTimedOut((CFStringRef)notification_id,
                                          (CFStringRef)notification_url);
}
@end  // GBCGrowlDelegate


// GrowlBalloonCollection

// These are called by the Growl delegate.
void GrowlBalloonCollection::NotificationTimedOut(CFStringRef id,
                                                  CFStringRef url) {

  std::string16 id_str;
  std::string16 url_str;

  CFStringRefToString16(id, &id_str);
  CFStringRefToString16(url, &url_str);

  DeleteWithURL(id_str, url_str);
}

void GrowlBalloonCollection::NotificationClicked(CFStringRef id,
                                                 CFStringRef url) {
  // TODO(chimene): Signal to load primary action URL here
  // when actions are ready.

  std::string16 id_str;
  std::string16 url_str;

  CFStringRefToString16(id, &id_str);
  CFStringRefToString16(url, &url_str);

  DeleteWithURL(id_str, url_str);
}

GrowlBalloonCollection::GrowlBalloonCollection() {
  GBCGrowlDelegate *growl_delegate =
      [[[GBCGrowlDelegate alloc] initWithGrowlBalloonCollection:this] autorelease];
  // GrowlApplicationBridge retains its delegate,
  // so we don't have to hang on to it.
  [GrowlApplicationBridge setGrowlDelegate:growl_delegate];
}

GrowlBalloonCollection::~GrowlBalloonCollection() {
}

// Called by NotificationManager when a new notification comes in
// Constructs a Growl notification from the Gears notification and sends it.
void GrowlBalloonCollection::Add(const GearsNotification &notification) {
  NotificationId id(notification.security_origin().url(), notification.id());

  GearsNotification *added_notification = new GearsNotification();
  added_notification->CopyFrom(notification);
  active_notifications_[id] = added_notification;

  NSString *url =
      [NSString GMStringWithString16:notification.security_origin().url()];
  NSString *title = [NSString GMStringWithString16:notification.title()];
  NSString *subtitle = [NSString GMStringWithString16:notification.subtitle()];
  NSString *description =
      [NSString GMStringWithString16:notification.description()];
  NSString *notification_id = [NSString GMStringWithString16:notification.id()];
  
  NSString *secure_notification_id = [NSString stringWithFormat:@"%@::%@",
                                      url, notification_id];

  NSData *image = TIFFDataFromRawBitmap(notification.icon_raw_data());

  // There's no 'subtitle' in Growl, so we stick subtitle and description
  // together unless one is empty.
  // If both are empty, we send an empty description.
  NSString *subtitle_and_description;
  if ([subtitle length] == 0) {
    subtitle_and_description = description;
  } else if ([description length] == 0) {
    subtitle_and_description = subtitle;
  } else {
    subtitle_and_description = [NSString stringWithFormat:@"%@\n%@",
                                subtitle, description];
  }

  // We build an array of strings instead of a custom object because Growl
  // creates a property list out of the arguments to notifyWithTitle,
  // and only certain types are allowed in.
  NSDictionary *context = [NSDictionary dictionaryWithObjectsAndKeys:
                           url,               URL_KEY,
                           notification_id,   ID_KEY,
                           nil];
  
  // A notification should be displayed for longer than the default time if
  // display_until_time_ms is set.  We can't tell a sticky to go away with 
  // Growl, so we just sticky it and let the user click it to make it go away.
  BOOL is_sticky = (notification.display_until_time_ms() != 0);

  [GrowlApplicationBridge notifyWithTitle:title
                              description:subtitle_and_description
                         notificationName:(NSString *)kGrowlNotificationName
                                 iconData:image
                                 priority:0
                                 isSticky:is_sticky
                             clickContext:context
                               identifier:secure_notification_id];
}

// Returns autoreleased NSData* with the contents of the bitmap
// converted to TIFF
NSData *TIFFDataFromRawBitmap(const std::vector<uint8> icon_data_vector) {

  uint8 *icon_raw_data = new uint8[icon_data_vector.size()];

  if (icon_raw_data == NULL)
    return nil;

  copy(icon_data_vector.begin(), icon_data_vector.end(), icon_raw_data);

  // We're in 'meshed' (not planar) configuration, so we only need one plane.
  // But the init method is very general, so it supports bitmaps which have
  // separate arrays for R,G,B,A.
  uint8 **data_planes = &icon_raw_data;

  // Officially the longest Cocoa method name.
  NSBitmapImageRep* bitmap = [[[NSBitmapImageRep alloc]
      initWithBitmapDataPlanes:data_planes
                    pixelsWide:kNotificationIconDimensions
                    pixelsHigh:kNotificationIconDimensions
                 bitsPerSample:8  // uint8
               samplesPerPixel:4  // RGBA
                      hasAlpha:YES  // RGB*A*
                      isPlanar:NO  // meshed, only one entry in data_plane_array
                colorSpaceName:NSCalibratedRGBColorSpace
                  bitmapFormat:NSAlphaNonpremultipliedBitmapFormat
                   bytesPerRow:0  // hint only, let it figure these out
                  bitsPerPixel:0] autorelease];

  // Ensures TIFF's dpi is set properly.
  NSSize bitmapSize = { [bitmap pixelsWide], [bitmap pixelsHigh] };
  [bitmap setSize: bitmapSize];

  NSData *tiff_data = [bitmap TIFFRepresentation];

  // NSBitmapImageRep references the original array
  // but doesn't take ownership of it.
  delete[] icon_raw_data;

  return tiff_data;
}

bool GrowlBalloonCollection::Update(const GearsNotification &notification) {
  NotificationId id(notification.security_origin().url(), notification.id());
  if (active_notifications_.find(id) == active_notifications_.end()) {
    return false;
  }
  active_notifications_.erase(id);
  // Erase and re-add, because Growl can't modify notifications that are
  // already visible.
  // TODO(chimene): Does Growl consolidate notifications if they are different
  // but have the same id?
  Add(notification);
  return true;
}

bool GrowlBalloonCollection::DeleteWithURL(const std::string16 &url,
                                           const std::string16 &bare_id) {
  NotificationId id(url, bare_id);
  if (active_notifications_.find(id) == active_notifications_.end()) {
    return false;
  }
  active_notifications_.erase(id);
  return true;
}

bool GrowlBalloonCollection::Delete(const SecurityOrigin &security_origin,
                                    const std::string16 &bare_id) {
  // We want the click handler to still work even if the notification is deleted
  // because Growl notifications are still visible even after deletion,
  // and they should still work.
  return false;
}

// These methods are here to implement BalloonCollectionInterface.
// has_space tells NotificationManager how many more balloons can fit in the
// vertical space of the screen, but since Growl has no way to figure it out,
// we just accept all notifications.
// (Growl might be sending the notfications over email for all we know!)
// TODO(chimene): Add a preference to limit notifications to X amount at a time
// Make sure that it doesn't break if Growl forgets to send the callback.
bool GrowlBalloonCollection::HasSpace() const {
  return true;
}

// The concept is that we 'hand off' notifications to Growl, so we shouldn't
// worry about saving displayed notifications in case we are restarted.
int GrowlBalloonCollection::count() const {
  return 0;
}

// This would break saving notifications if count returned anything > 0.
const GearsNotification *GrowlBalloonCollection::notification_at(int i) const {
  return NULL;
}

#endif  // OFFICIAL_BUILD
