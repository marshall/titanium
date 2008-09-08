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

// Implementation of native dialogs for OSX.  These are temporary standins
// till we can fix the underlying bug in WebKit that's preventing us from
// using HTML Dialogs like the other Gears ports.

// Since the dialogs are so similar, we define a BaseNativeDialog that 
// implements nearly all of the functionality and draws the dialog components.
// Terminology used for dialog componets:
// gears_imageview_ - The image at the top left of the permission and location
// dialogs, this is blank in the case of the shortcut dialog.
// site_imageview_ - The site-specific icon portion of the dialogs.
// dialog_text_    - The central text area used to display the origin name
// and description text, we use the basic HTML parsing capabilities of the
// NSAttributedString class to display this.
//
// NativePermissionsDialog & NativeShortcutsDialog derive from the base class,
// they only add argument parsing and generation of the dialog_text html.

#import <Cocoa/Cocoa.h>
#import "gears/base/common/string16.h"

// This is the function that interfaces the native dialogs with the outside
// world.
bool ShowNativeModal(const char16 *html_filename, 
                     const char16 *arguments_string,
                     std::string16 *results);


// Preload .nib files into memory on Gears startup.
void PreloadNibFiles();
void UnloadNibFiles();

@protocol NativeDialog
- (bool)setArgumentsString:(NSString *)args;
- (NSString *)nibName;
- (NSString *)neverAllowLabel;

- (NSString *)results;
- (NSWindow *)window;

// Can we close the dialog?
-(bool)done;
@end

@interface BaseNativeDialog : NSObject {
// All the Gears dialogs basically have the same parts.
IBOutlet NSWindow *window_;
IBOutlet NSImageView *gears_imageview_;
IBOutlet NSImageView *site_imageview_;
IBOutlet NSTextField *dialog_origin_title_;
IBOutlet NSTextField *dialog_origin_subtitle_;
IBOutlet NSTextView *dialog_origin_description_;
IBOutlet NSTextField *never_allow_link_;
IBOutlet NSTextField *top_label_;
IBOutlet NSTextField *location_dialog_label_;

// Name of the icon to display in the gears_imageview_, keep NULL
// in order to keep icon blank.
const char16 *icon_name_;

// The Checkbox toggles the allow button in some dialogs so we need these
// here.
IBOutlet NSButton *trust_checkbox_;
IBOutlet NSButton *allow_button_;

// Arguments
NSString *origin_;
NSString *site_icon_url_;
NSString *custom_site_name_;
NSString *site_description_;

// Results
bool done_;
NSString *result_string_;
}

- (IBAction)trustCheckboxValueChanged:(id)sender;

// Show the site icon.
- (void)handleSiteIconDisplay;

- (NSString *)getDialogTextHtml:(int)dialog_width;

// Called when nib is loaded.
- (void)awakeFromNib;

// Methods called by buttons.
- (IBAction)allow:(id)sender;
- (IBAction)deny:(id)sender;
- (IBAction)neverAllow:(id)sender;

@end

// Local & Location permissions dialogs.
@interface NativePermissionsDialog : BaseNativeDialog {
  bool is_location_dialog;
}
- (bool)setArgumentsString:(NSString *)args;
- (NSString *)nibName;
@end

// Shortcuts dialog.
@interface NativeShortcutsDialog : BaseNativeDialog
- (bool)setArgumentsString:(NSString *)args;
- (NSString *)nibName;
@end
