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

#import "gears/base/common/common.h"
#import "gears/base/npapi/browser_utils.h"
#import "gears/base/safari/curl_downloader.h"
#import "gears/base/safari/nsstring_utils.h"
#import "gears/base/safari/resource_archive.h"
#import "gears/ui/safari/native_dialogs_osx.h"
#include "third_party/jsoncpp/json.h"

const int kSiteIconWidth = 32;
const NSString *kLocationLabelStr = @"The website below wants to access "
                                     "information about your location using"
                                     " Gears.";
const NSString *kShortcutLabelStr = @"This website wants to create a shortcut"
                                     " on your computer. Do you want to allow"
                                     " this?";                                     

@interface NSAttributedString (Underlining)
  +(id)setUnderLineString:(NSString*)inString;
@end

@implementation NSAttributedString (Underlining)
+(id)setUnderLineString:(NSString*)inString
{
  NSMutableAttributedString* attrString = [[NSMutableAttributedString alloc]
                                              initWithString: inString];
  NSRange range = NSMakeRange(0, [attrString length]);

  [attrString beginEditing];

  // make the text appear in blue
  [attrString addAttribute:NSForegroundColorAttributeName
                     value:[NSColor blueColor] 
                     range:range];

  // next make the text appear with an underline
  [attrString addAttribute:NSUnderlineStyleAttributeName
                     value:[NSNumber numberWithInt:NSSingleUnderlineStyle] 
                     range:range];
  [attrString endEditing];

  return [attrString autorelease];
}
@end

// Resize an NSWindow by a delta.
@interface NSWindow (Resizing)
  -(void)resizeBy:(NSSize)delta;
@end

@implementation NSWindow (Resizing)
-(void)resizeBy:(NSSize)delta {
  NSRect new_size = [self frame];
  new_size.size.height += delta.height;
  new_size.size.width += delta.width;
  [self setFrame:new_size display:YES];
}
@end

// Move an NSView.
@interface NSView (Moving)
  -(void)expandBy:(int)delta;
@end

@implementation NSView (Moving)
-(void)expandBy:(int)delta {
  NSRect new_size = [self frame];
  new_size.origin.x -= delta/2;
  new_size.size.width += delta/2;
  [self setFrame:new_size];
}
@end


// Convert a value from a JSON hash into an NSString.
static NSString *getJSONString(const Json::Value &val, const char *key) {
  if (val[key].isString()) {
    std::string description = val[key].asString();
    if (description.empty()) return nil;
    
    return [NSString stringWithUTF8String:description.c_str()];
  }
  return nil;
}

static const NSMutableDictionary *kNibDictionary = nil;

void PreloadNibFiles() {
  assert(!kNibDictionary);
  kNibDictionary = [[NSMutableDictionary alloc] init];
  NSBundle *gears_bundle = [NSBundle bundleForClass:[BaseNativeDialog class]];

  NSArray *nib_names = [NSArray arrayWithObjects:
                                    @"shortcuts_dialog.nib",
                                    @"location_permissions_dialog.nib",
                                    @"permissions_dialog.nib",
                                    nil];
  NSEnumerator *nib_enum = [nib_names objectEnumerator];
  while (NSString *nib_name = [nib_enum nextObject]) {
    NSNib *nib_obj = [[NSNib alloc] initWithNibNamed:nib_name 
                                               bundle:gears_bundle];
    [kNibDictionary setObject:nib_obj forKey:nib_name];
  }
}

void UnloadNibFiles() {
  [kNibDictionary release];
}

bool ShowNativeModal(const char16 *html_filename, 
                     const char16 *arguments_string,
                     std::string16 *results) {

  id <NativeDialog> dialog;
  if (StringCompareIgnoreCase(html_filename, 
                              STRING16(L"shortcuts_dialog.html")) == 0) {
   dialog = [[[NativeShortcutsDialog alloc] init] autorelease];
  } else {
   dialog = [[[NativePermissionsDialog alloc] init] autorelease];
  }
  NSString *nib_name;
  
  if (![dialog setArgumentsString:
                   [NSString stringWithString16:arguments_string]] ) {
   return false;
  }
  nib_name = [dialog nibName];
  if (kNibDictionary == nil) {
    NSLog(@"Gears InputManager failed to load Gears, exiting...");
    return false;
  }
  NSNib *dialog_nib = [kNibDictionary objectForKey:nib_name];
  [dialog_nib instantiateNibWithOwner:dialog topLevelObjects:nil];

  NSWindow *window = [dialog window];
  NSWindow *front_window = [NSApp keyWindow];
  
  [NSApp beginSheet:window
     modalForWindow:front_window
      modalDelegate:nil
     didEndSelector:nil
        contextInfo:nil];
  [window makeKeyAndOrderFront:nil];
  [NSApp runModalForWindow:window];
  [NSApp endSheet:window];
  
  [[dialog results] string16:results];
  
  return true;
}

static NSMutableAttributedString *left_justify(const NSString *str) {
  // Attributed String is left-aligned by default.
  NSMutableAttributedString *ret = [[[NSMutableAttributedString alloc] 
                                      initWithString:str]
                                      autorelease];
  return ret;
}

@implementation BaseNativeDialog

- (NSString *)results {
  return result_string_;
}

- (NSWindow *)window {
 return window_;
}

- (bool)done {
  return done_;
}

- (IBAction)trustCheckboxValueChanged:(id)sender {
  // Allow button is only enabled if checkbox is checked.
  bool checked = [sender state] == NSOnState;
  [allow_button_ setEnabled:checked];
}

// Handle display of a custom site icon.
- (void)handleSiteIconDisplay {
  // Load site icon Asynchronously
  if(site_icon_url_) {
    NSArray *args = [NSArray arrayWithObjects:site_icon_url_, nil];

    // detachNewThreadSelector will retain self until after the thread exits.
    [NSThread detachNewThreadSelector:@selector(loadURL:)
                           toTarget:self
                         withObject:args];
  } else {
    // To hide it, just kick it out of the window.
    [site_imageview_ removeFromSuperview];
  }
}

#pragma mark Protected Instance methods
// Sets the image at the top left of the dialog.
- (void)setDialogIcon:(NSData *)icon_data {
  NSImage *the_image = [[[NSImage alloc] initWithData:icon_data] autorelease];
  [gears_imageview_ setImage:the_image];
}

- (void)setSiteIcon:(NSData *)icon_data {
  NSImage *the_image = [[[NSImage alloc] initWithData:icon_data] autorelease];
  [site_imageview_ setImage:the_image];
}

// Set the underline for the "Never Allow" link.
- (void)initNeverAllow {
  [never_allow_link_ setAllowsEditingTextAttributes:YES];
  NSMutableAttributedString* string = [[NSMutableAttributedString alloc] init];
  [string setAttributedString:[NSAttributedString 
      setUnderLineString:[(id <NativeDialog>)self neverAllowLabel]]];
  [never_allow_link_ setAttributedStringValue:string];
}

#pragma mark Methods for dynamic image loading.
// Copied from html_dialog_sf.mm

// This selector is launched as a thread, it loads the URL with CURL
// and then calls back to displayImage: when done.
- (void)loadURL:(NSArray *)arguments {
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  
  assert([arguments count] == 1);
  NSString *url = [arguments objectAtIndex:0];
  
  // Make sure URL is escaped.
  url = [url stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding];
  std::string16 url_utf16;
  [url string16:&url_utf16];
  
  // Download Icon Data.
  std::string16 user_agent_utf16;
  BrowserUtils::GetUserAgentString(&user_agent_utf16);
  
  NSMutableData *icon_data = [[[NSMutableData alloc] init] autorelease];
  if (!GetURLDataAsNSData(url_utf16, user_agent_utf16, icon_data)) return;
  
  NSArray *args = [NSArray arrayWithObjects:icon_data, nil];
  // args is retained until the selector returns.
  [self performSelectorOnMainThread:@selector(displayImage:)
                                withObject:args
                            waitUntilDone:false];
  [pool release];
}

// Called asynchronously when image data is loaded, this displays the image
// data in the WebView.
- (void)displayImage:(NSArray *)arguments {
  // If the window was closed, don't do anything.
  if (done_) return;
  
  assert([arguments count] == 1);
  NSData *image_data = [arguments objectAtIndex:0];
  [self setSiteIcon:image_data];
}

- (void)removeView:(NSView *)view {
  NSRect view_frame = [view frame];
  [window_ resizeBy:NSMakeSize(0, -view_frame.size.height)];
  [view removeFromSuperview];
}

- (void)awakeFromNib {
  [self initNeverAllow];
  
  // These are actually .png images, but because of the way the scripts
  // work, they are alwyas named .webarchive.
  if (icon_name_ != NULL) {
    unsigned char *resource_data;
    size_t resource_len;
    if (!GetNamedResource(icon_name_,
                          &resource_data,
                          &resource_len)) return;
    NSData *icon_data = [NSData dataWithBytes:resource_data
                                          length:resource_len];
    [self setDialogIcon:icon_data];
  }
  
    // Move text left if icon isn't present.
  if (!site_icon_url_) {
    [site_imageview_ removeFromSuperview];
    [dialog_origin_title_ expandBy:kSiteIconWidth/2];
    [dialog_origin_subtitle_ expandBy:kSiteIconWidth/2];
    [dialog_origin_description_ expandBy:kSiteIconWidth/2];
  }
  
  if (custom_site_name_) {
    NSAttributedString *site_name = left_justify(custom_site_name_);
    [dialog_origin_title_ setAttributedStringValue:site_name];
    if (origin_) {
      [dialog_origin_subtitle_ setStringValue:origin_];
    }
  }
  else {
    [dialog_origin_title_ setStringValue:origin_];
    [self removeView:dialog_origin_subtitle_];
    // Magic value that fixes up vertical position.
    [window_ resizeBy:NSMakeSize(0, -105)];
  }

  if (site_description_) {
    [dialog_origin_description_ setEditable:YES];
    [dialog_origin_description_ insertText:site_description_];
    [dialog_origin_description_ setEditable:NO];
    
    // Resize description to fit content vertically.
    NSRect original_size = [dialog_origin_description_ frame];
    NSLayoutManager *layout = [dialog_origin_description_ layoutManager];
    NSTextContainer *text_container = [dialog_origin_description_ textContainer];
    
    NSRect new_size = [layout usedRectForTextContainer:text_container];
    int delta =  new_size.size.height - original_size.size.height;
    
    // Revert horizontal resizing.
    new_size.origin.x = original_size.origin.x;
    new_size.size.width = original_size.size.width;
    
    // Now move description up.
    new_size.origin.y -= delta;
    [dialog_origin_description_ setFrame:new_size];
    [window_ resizeBy:NSMakeSize(0, delta)];
  } else {
    [self removeView:dialog_origin_description_];
  }
  //------------
  
  
  [self handleSiteIconDisplay];
}

- (NSString *)getDialogTextHtml:(int)dialog_width {
  // Should only be called for base classes.
  assert(false);
  return nil;
}

- (void)dealloc {
  [origin_ release];
  [site_icon_url_ release];
  [custom_site_name_ release];
  [site_description_ release];
  [super dealloc];
}

// pure virtual methods.
- (NSString *)neverAllowLabel {
  assert(false);
  return nil;
}

- (IBAction)allow:(id)sender {
  assert(false);
}

- (IBAction)deny:(id)sender {
  assert(false);
}

- (IBAction)neverAllow:(id)sender {
  assert(false);
}

@end

@implementation NativePermissionsDialog

- (bool)setArgumentsString:(NSString *)args {

// From JS implementation:
//    var origin = args['origin'];  // required parameter
//    var dialogType = args['dialogType'];  // required parameter
//    var customIcon = args['customIcon'];
//    var customName = args['customName'];
//    var customMessage = args['customMessage'];

  Json::Value result;
  Json::Reader reader;
  
  // Parse arguments.
  const char *arg_string_utf8 = [args UTF8String];
  if (!reader.parse(arg_string_utf8, result)) {
    LOG(("Error parsing return value from dialog. Error was: %s", 
         reader.getFormatedErrorMessages().c_str()));
    return false;
  }

  // required.
  origin_ = [getJSONString(result, "origin") retain];
  if(!origin_) return false;

  // required.
  NSString *dialog_type = getJSONString(result, "dialogType");
  if(!dialog_type) return false;
  is_location_dialog = ![dialog_type isEqualToString:@"localData"];
  
  site_icon_url_    =  [getJSONString(result, "customIcon") retain];
  custom_site_name_ =  [getJSONString(result, "customName") retain];
  site_description_ =  [getJSONString(result, "customMessage") retain];
  
  // Set the icon name based on whether this is a location or local data dialog.
  // These are actually png images, the .webarchive extension is an artifact
  // of the serialization mechanism we use.  It's not worth chaning it for
  // the temporary HTML dialogs.
  icon_name_ = STRING16(L"local_data.webarchive");
  if (is_location_dialog) {
    icon_name_ = STRING16(L"location_data.webarchive");
  }
  
  return true;
}

- (void)awakeFromNib {
  // Adjust dialog if it's a location dialog.
  if (is_location_dialog) {
    // Set the text for the top dialog label.
    [top_label_ setStringValue:kLocationLabelStr];
  }
  [super awakeFromNib];
}

- (NSString *)nibName {
  if (is_location_dialog) {
    return @"location_permissions_dialog.nib";
	} else {
    return @"permissions_dialog.nib";
	}
}

- (IBAction)allow:(id)sender {
  result_string_ = @"{\"allow\": true, \"permanently\": true}";
  done_ = true;
  [window_ close];
  [NSApp stopModal];
}

- (IBAction)deny:(id)sender  {
  result_string_ = @"{\"allow\": false, \"permanently\": false}";
  done_ = true;
  [window_ close];
  [NSApp stopModal];
}

- (IBAction)neverAllow:(id)sender  {
  result_string_ = @"{\"allow\": false, \"permanently\": true}";
  done_ = true;
  [window_ close];
  [NSApp stopModal];
}

- (NSString *)neverAllowLabel {
  return @"Never allow for this site";
}
@end

@implementation NativeShortcutsDialog

- (NSString *)nibName {
    return @"shortcuts_dialog.nib";
}

- (void)awakeFromNib {
  // Adjust dialog if it's a shortcut dialog.
  // Set the text for the top dialog label.
  [top_label_ setStringValue:kShortcutLabelStr];
  [super awakeFromNib];
}

- (bool)setArgumentsString:(NSString *)args {
  Json::Value result;
  Json::Reader reader;
  
  // Parse arguments.
  const char *arg_string_utf8 = [args UTF8String];
  if (!reader.parse(arg_string_utf8, result)) {
    LOG(("Error parsing return value from dialog. Error was: %s", 
         reader.getFormatedErrorMessages().c_str()));
    return false;
  }
  
  // required.
  custom_site_name_    =  [getJSONString(result, "name") retain];
  if (!custom_site_name_) return false;
  site_description_    =  [getJSONString(result, "description") retain];
  
  // Chose which icon to display, preference is in the same order as the
  // HTML shortcuts dialog.
  NSArray *icon_names = [NSArray arrayWithObjects:@"icon32x32",
                                                  @"icon48x48",
                                                  @"icon128x128",
                                                  @"icon16x16",
                                                  nil];
  NSEnumerator *icon_enum = [icon_names objectEnumerator];
  while (NSString *icon_url = [icon_enum nextObject]) {
    const char *icon_url_utf8 = [icon_url UTF8String];
    if (result[icon_url_utf8].isString()) {
      std::string val = result[icon_url_utf8].asString();
      if (val.empty()) continue;
      site_icon_url_ = [NSString stringWithUTF8String:val.c_str()];
      [site_icon_url_ retain];
      break;
    }
  }
  
  return true;
}

- (IBAction)allow:(id)sender {
  result_string_ = @"{\"allow\": true, \"permanently\": true}";
  done_ = true;
  [window_ close];
  [NSApp stopModal];
}

- (IBAction)deny:(id)sender  {
  result_string_ = @"";
  done_ = true;
  [window_ close];
  [NSApp stopModal];
}

- (IBAction)neverAllow:(id)sender  {
  result_string_ = @"{\"allow\": false, \"permanently\": true}";
  done_ = true;
  [window_ close];
  [NSApp stopModal];
}

- (NSString *)neverAllowLabel {
  return @"Never allow for this shortcut";
}

@end
