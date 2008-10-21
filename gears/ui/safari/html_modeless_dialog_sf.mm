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

#import <Cocoa/Cocoa.h>
#import <WebKit/WebKit.h>

#import "gears/base/common/string16.h"
#import "gears/base/common/js_runner.h"
#import "gears/base/common/paths_sf_more.h"
#import "gears/base/common/string_utils.h"
#import "gears/base/npapi/browser_utils.h"
#import "gears/base/safari/nsstring_utils.h"
#import "gears/base/safari/scoped_cf.h"
#import "gears/ui/common/html_dialog.h"
#import  "gears/ui/safari/html_dialog_sf.h"

// Class to display a Modal dialog in WebKit.
@interface HTMLModelessDialogImp  : HTMLDialogImp {
 @private
  HtmlDialog::ModelessCompletionCallback callback_;
  void *callback_closure_;
  HtmlDialog *owner_;
}

// Initialize an HTMLDialogImp instance.
- (id)initWithHtmlFile:(const std::string16 &)html_filename 
             arguments:(const std::string16 &)arguments
                width:(int)width
               height:(int)height
                owner:(HtmlDialog *)owner
             callback:(HtmlDialog::ModelessCompletionCallback)callback
     callback_closure:(void *)callback_closure;

// Show the modal dialog and block until said dialog is dismissed.
// returns: true on success.
- (bool)showModeless:(NSString *)title;
@end

@implementation HTMLModelessDialogImp

#pragma mark Public Instance methods
- (id)initWithHtmlFile:(const std::string16 &)html_filename 
             arguments:(const std::string16 &)arguments
                 width:(int)width
                height:(int)height
                owner:(HtmlDialog *)owner
             callback:(HtmlDialog::ModelessCompletionCallback)callback
     callback_closure:(void *)callback_closure {
  assert(callback);
  self = [super initWithHtmlFile:html_filename
                       arguments:arguments
                           width:width
                          height:height];
  if (self) {
    owner_ = owner;
    callback_ = callback;
    callback_closure_ = callback_closure;
  }
  return self;
}

- (bool)showModeless:(NSString *)title {
  // Create the dialog's window.
  unsigned int window_style = NSTitledWindowMask | NSResizableWindowMask | 
                              NSClosableWindowMask;
  if (![self createWindow:window_style]) {
    return false;
  }

  // Modeless-specific init.
  [window_ setDelegate:self];
  [window_ setTitle:title];
  [window_ center];
  [window_ setFrameUsingName:web_archive_filename_];
  
  // Display sheet.
  [window_ makeKeyAndOrderFront:nil];
  return true;
}
@end

@implementation HTMLModelessDialogImp(GearsWebViewDelegateMethods)
// JS callback for setting the result string.
- (void)setResults: (NSString *)window_results {
  [super setResults:window_results];
  // Save current location and size of window.
  [window_ saveFrameUsingName:web_archive_filename_];
  [window_ close];

  std::string16 results;
  [result_string_ string16:&results];
  owner_->SetResult(results.c_str());
  callback_(&(owner_->result), callback_closure_);
  [self release];
}
@end

@implementation HTMLModelessDialogImp(GearsNSWindowDelegateMethods)
- (BOOL)windowShouldClose:(id)sender {
  callback_(NULL, callback_closure_);
  // Save current location and size of window.
  [window_ saveFrameUsingName:web_archive_filename_];
  [self release];
  return YES;
}
@end

bool HtmlDialog::DoModelessImpl(const char16 *html_filename, int width,
                                int height, const char16 *arguments_string, 
                                ModelessCompletionCallback callback, 
                                void *closure) {
   HTMLModelessDialogImp * dialog = [[HTMLModelessDialogImp alloc]
                                              initWithHtmlFile:html_filename 
                                                     arguments:arguments_string
                                                         width:width
                                                        height:height
                                                         owner:this
                                                      callback:callback
                                              callback_closure:closure];
  
  if (!dialog) {
    return false;
  }
  std::string16 results;
  if (![dialog showModeless:@PRODUCT_FRIENDLY_NAME_ASCII" Settings"]) {
    return false;
  }
  
  return true;
}

