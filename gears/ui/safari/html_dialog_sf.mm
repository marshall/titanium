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
#import "gears/base/safari/curl_downloader.h"
#import "gears/base/safari/resource_archive.h"
#import "gears/base/safari/safari_workarounds.h"
#import "gears/base/safari/scoped_cf.h"
#import "gears/ui/common/html_dialog.h"
#import "gears/ui/safari/html_dialog_sf.h"
#import "gears/ui/safari/native_dialogs_osx.h"

@implementation HTMLDialogImp

static NSDictionary *ScriptableMethods() {
  // In order to create a static objective-c object we need to create it after
  // the runtime is initialized.
  static const NSDictionary *scriptable_methods = 
      [[NSDictionary dictionaryWithObjectsAndKeys:
                         @"setResults",
                         @"setResults:",
                         @"loadImageIntoElement",
                         @"loadImageIntoElement:top:left:width:height:",
                         nil] retain];
  return scriptable_methods;
}

#pragma mark Private Instance methods
// Creates a window and places a pointer to it in the |window_| ivar.
- (BOOL)createWindow:(unsigned int)window_style {
  // Create the dialog's window.
  NSRect content_rect = NSMakeRect(0, 0, width_, height_);
  window_ = [[NSWindow alloc]  initWithContentRect:content_rect 
                                         styleMask:window_style
                                           backing:NSBackingStoreBuffered
                                             defer:YES];

  if (!window_) {
    return false;
  }
  
  [window_ setMinSize:NSMakeSize(width_, height_)];
  [window_ setReleasedWhenClosed:YES];
  
  // Create a WebView and attach it.
  NSView *content_view = [window_ contentView];
  WebView *webview = [[WebView alloc] initWithFrame:[content_view frame]];
  
  // Set the user agent for the WebView that we're opening to the same one as
  // Safari.
  std::string16 ua_str16;
  BrowserUtils::GetUserAgentString(&ua_str16);
  
  // The user agent string is initialized by the plugin's NP_Initialize
  // function, but the settings dialog may be displayed before that's
  // been called, in which case we just use a default string.
  NSURL *tmp_url = [NSURL URLWithString:@""];
  user_agent_ = [NSString stringWithString16:ua_str16.c_str()];
  if (ua_str16.length() == 0) { 
    user_agent_ = [NSString stringWithFormat:@"%@ Safari",
                    [webview userAgentForURL:tmp_url]];
  }
  [user_agent_ retain];
  [webview setCustomUserAgent:user_agent_];

  [content_view addSubview:webview];
  [webview release]; // addSubView retains webView.
  [webview setAutoresizingMask:NSViewHeightSizable | NSViewWidthSizable];
  
  // Make ourselves the WebView's delegate.
  [webview setFrameLoadDelegate:self];
  [webview setUIDelegate:self];
  
  // Load in the dialog's contents.
  WebFrame *frame = [webview mainFrame];
  
  // Read in WebArchive.
  unsigned char *resource_data;
  size_t resource_len;
  std::string16 resource_name;
  [web_archive_filename_ string16:&resource_name];
   if (!GetNamedResource(resource_name,
                         &resource_data,
                         &resource_len)) return false;
  NSData *archive_data = [NSData dataWithBytes:resource_data
                                        length:resource_len];
  
  WebArchive *webarchive = [[WebArchive alloc] initWithData:archive_data];
  
  [frame loadArchive:webarchive];
  
  // Turn off scrollbars.
  [[frame frameView] setAllowsScrolling:NO];

  return true;
}

#pragma mark Public Instance methods
- (id)initWithHtmlFile:(const std::string16 &)html_filename 
             arguments:(const std::string16 &)arguments
                 width:(int)width
                height:(int)height {
  self = [super init];
  if (self) {
    NSString *tmp = [NSString stringWithString16:html_filename.c_str()];
    tmp = [tmp stringByDeletingPathExtension];
    // If we ever go back to file urls, rather than loading the data directly
    // from disk, we need to escape web_archive_filename_ here.
    web_archive_filename_ = [tmp stringByAppendingPathExtension:@"webarchive"];
    [web_archive_filename_ retain];
    arguments_ = [[NSString stringWithString16:arguments.c_str()] retain];
    width_ = width;
    height_ = height;
  }
  return self;
}

- (void)dealloc {
  [web_archive_filename_ release];
  [arguments_ release];
  [user_agent_ release];
  [result_string_ release];
  [super dealloc];
}

- (bool)showModal:(std::string16 *)results {

  unsigned int window_style = NSTitledWindowMask;
  if (![self createWindow:window_style]) {
    return false;
  }
  
  // See gears/base/safari/safari_workarounds.m for details.
  EnableWebkitTimersForNestedRunloop();

  // Display the dialog.
  NSWindow *front_window = [NSApp keyWindow];
  [NSApp beginSheet:window_ 
     modalForWindow:front_window 
      modalDelegate:nil 
     didEndSelector:nil
        contextInfo:nil];
 
 // Spin until the sheet is closed.
 // Credit goes to David Sinclair of Dejal software for this method of running
 // a modal WebView.
 NSModalSession session = [NSApp beginModalSessionForWindow:window_];
 NSRunLoop *runloop = [NSRunLoop currentRunLoop];
 NSDate *oneHunderMS = [NSDate dateWithTimeIntervalSinceNow:0.1];
  while (!window_dismissed_ && 
          [NSApp runModalSession:session] == NSRunContinuesResponse) {
     [runloop runMode:NSDefaultRunLoopMode beforeDate:oneHunderMS];
  }
  [NSApp endModalSession:session];
  
// Display Document-Modal dialog, this doesn't work on 10.4.
//  while (!window_dismissed_) {
//    [NSApp setWindowsNeedUpdate:YES];
//    NSEvent *event = [NSApp nextEventMatchingMask:NSAnyEventMask
//                                        untilDate:[NSDate distantFuture]
//                                           inMode:NSDefaultRunLoopMode
//                                          dequeue:YES];
//    [NSApp sendEvent:event];
//  }  
  
  [NSApp endSheet:window_];
  [window_ close];
  window_ = nil;
  
  [result_string_ string16:results];
  return true;
}

// Delegate method called just before any js is run, so this is the perfect
// time to inject vairables into the js environment.
- (void)webView:(WebView *)webView 
            windowScriptObjectAvailable:(WebScriptObject *)windowScriptObject {
      
  // Pass in dialog arguments in the gears_dialogArguments variable.
  [windowScriptObject setValue:arguments_
                        forKey:@"gears_dialogArguments"];
  
  // Register ourselves as the window.gears_dialog object.
  [windowScriptObject setValue:self forKey:@"gears_dialog"];
}

// Called by WebView to determine which of this object's selectors can be called
// from JS.
+ (BOOL)isSelectorExcludedFromWebScript:(SEL)selector {
  NSString *sel = [NSString stringWithCString:sel_getName(selector)
                                     encoding:NSUTF8StringEncoding];
  BOOL is_selector_scriptable = [ScriptableMethods() objectForKey:sel] != nil;
  return !is_selector_scriptable;
}

+ (NSString *)webScriptNameForSelector:(SEL)selector {
  NSString *sel = [NSString stringWithCString:sel_getName(selector)
                                     encoding:NSUTF8StringEncoding];
  return [ScriptableMethods() objectForKey:sel];
}

// Webview UI delegate method.
- (NSArray *)webView:(WebView *)sender 
    contextMenuItemsForElement:(NSDictionary *)element 
              defaultMenuItems:(NSArray *)defaultMenuItems {
  // disable contextual menus for dialogs.
  return nil;
}

// JS callback for asynchronously loading an image.
- (void)loadImageIntoElement: (NSString *)url
                         top: (int)top
                        left: (int)left
                       width: (int)width
                      height: (int)height {
  NSString *frame_rect = NSStringFromRect(NSMakeRect(left, top, width, height));
  NSArray *args = [NSArray arrayWithObjects:url, frame_rect, nil];
  
  // detachNewThreadSelector will retain self until after the thread exits.
  [NSThread detachNewThreadSelector:@selector(loadURL:)
                         toTarget:self
                       withObject:args];
}

// JS callback for setting the result string.
- (void)setResults: (NSString *)window_results {
  result_string_ = [window_results copy];
  window_dismissed_ = true;
}

#pragma mark Methods for dynamic image loading.

// This selector is launched as a thread, it loads the URL with CURL
// and then calls back to displayImage: when done.
- (void)loadURL:(NSArray *)arguments {
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  
  assert([arguments count] == 2);
  NSString *url = [arguments objectAtIndex:0];
  NSRect frame = NSRectFromString([arguments objectAtIndex:1]);
  
  // Make sure URL is escaped.
  url = [url stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding];
  
  // Download Icon Data.
  std::string16 url_utf16;
  [url string16:&url_utf16];
  std::string16 user_agent_utf16;
  [user_agent_ string16:&user_agent_utf16];
  
  NSMutableData *icon_data = [[[NSMutableData alloc] init] autorelease];
  if (!GetURLDataAsNSData(url_utf16, user_agent_utf16, icon_data)) return;
  
  NSArray *args = [NSArray arrayWithObjects:icon_data, NSStringFromRect(frame),
                                            nil];
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
  if (window_dismissed_) return;
  
  assert([arguments count] == 2);
  NSData *image_data = [arguments objectAtIndex:0];
  NSRect frame = NSRectFromString([arguments objectAtIndex:1]);
  NSView *content_view = [window_ contentView];
  
  // Translate from DOM->NSView coordinates.
  NSRect bounds = [content_view bounds];
  frame.origin.y = bounds.size.height - frame.origin.y - frame.size.height;
  
  NSImageView *iv = [[NSImageView alloc] initWithFrame:frame];
  NSImage *img = [[NSImage alloc] initWithData:image_data];
  [iv setImage:img];
  [img release];
  [content_view addSubview:iv];
  
  // Anchor the image view to the top left of the image contents.
  // TODO(playmobil): reposition the image view on screen resize from JS
  // to get more consistent results.
  [iv setAutoresizingMask:NSViewMaxXMargin | NSViewMinYMargin];
  [iv release];
}
@end

// Bring plugin's container tab & window to front.
// This is a bit of a hack because we have no way to tell which window the
// Gears plugin is running in, so once we do this we know it's the frontmost
// window.  It's also a better experience for the user to have the dialog
// appear in relevant context.
static bool FocusPluginContainerWindow() {
  JsCallContext *call_context = BrowserUtils::GetCurrentJsCallContext();
  NPObject *window;
  if (NPN_GetValue(call_context->js_context(), NPNVWindowNPObject, &window) != 
      NPERR_NO_ERROR) {
    return false;
  }
  
  std::string script_utf8("window.focus()");
  NPString script = {script_utf8.data(), script_utf8.length()};
  ScopedNPVariant foo;
  if (!NPN_Evaluate(call_context->js_context(), window, &script, &foo)) {
    return false;
  }
    
  return true;
}

bool HtmlDialog::DoModalImpl(const char16 *html_filename, int width, int height,
                             const char16 *arguments_string) {
  if (!FocusPluginContainerWindow()) {
    return false;
  }
  
  // We've temporarily disalbed HTML dialogs till we can resolve the underlying
  // bug in WebKit.  For now we use Native dialogs.
# if 0  
  HTMLDialogImp *dialog = [[[HTMLDialogImp alloc]
                                 initWithHtmlFile:html_filename 
                                        arguments:arguments_string
                                            width:width
                                           height:height] autorelease];
  
  if (!dialog) {
    return false;
  }
  std::string16 result;
  if (![dialog showModal:&results]) {
    return false;
  }
#else
  std::string16 results;
  if (!ShowNativeModal(html_filename, arguments_string, &results)) {
    return false;
  }
#endif
  
  SetResult(results.c_str());
  return true;
}

bool HtmlDialog::GetLocale(std::string16 *locale) {
  assert(locale);
  NSLocale *current_locale = [NSLocale currentLocale];
  NSString *language = [current_locale objectForKey:NSLocaleLanguageCode];
  NSString *country = [current_locale objectForKey:NSLocaleCountryCode];
  
  // Put together a string like en-US.
  NSString *ret = [NSString stringWithFormat:@"%@-%@", language, country];
  [ret string16:locale];
  return true;
}

