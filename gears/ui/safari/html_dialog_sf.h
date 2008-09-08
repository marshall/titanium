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
#ifndef GEARS_UI_SAFARI_HTML_DIALOG_SF_H__
#define GEARS_UI_SAFARI_HTML_DIALOG_SF_H__

// Class to display a Modal dialog in WebKit.
@interface HTMLDialogImp : NSObject {
 @protected
  NSString *web_archive_filename_;
  NSString *arguments_; 
  NSString *user_agent_;
  int width_;
  int height_;
  bool window_dismissed_;
  NSString *result_string_;
  NSWindow *window_;  // weak
}

// Initialize an HTMLDialogImp instance.
- (id)initWithHtmlFile:(const std::string16 &)html_filename 
             arguments:(const std::string16 &)arguments
                width:(int)width
               height:(int)height;

// Show the modal dialog and block until said dialog is dismissed.
// returns: true on success.
- (bool)showModal: (std::string16 *)results;

// Creates a window and places a pointer to it in the |window_| ivar.
- (BOOL)createWindow:(unsigned int)window_style;

// WebView delegate Methods
// Delegate method called just before any js is run, so this is the perfect
// time to inject vairables into the js environment.
- (void)webView:(WebView *)webView 
            windowScriptObjectAvailable:(WebScriptObject *)windowScriptObject;

+ (BOOL)isSelectorExcludedFromWebScript:(SEL)selector;
+ (NSString *)webScriptNameForSelector:(SEL)sel;

// Methods callable from JS.
// JS callback for setting the result string.
- (void)setResults: (NSString *)window_results;
@end
#endif  // GEARS_UI_SAFARI_HTML_DIALOG_SF_H__
