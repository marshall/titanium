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

// Command line tool to create a Safari .webarchive file from a bunch of
// resources.
// All files must be in the same directory.

// To compile:
//  g++ -arch ppc -arch i386 -o webarchiver -framework WebKit \
// -framework Cocoa webarchiver.mm

#import <Cocoa/Cocoa.h>
#import <WebKit/WebKit.h>

void usage() {
  printf("Usage: output.webarchive mainresource [resources]");
  exit(-1);
}

// Returns a WebResource corresponding to the file in the File system.
WebResource *getResource(NSString *filename) {
  NSData *data = [NSData dataWithContentsOfFile:filename];

  // Very basic mime type handling, we may need to edit if we want to add
  // more MIMETypes at some point.
  NSString *mime_type = @"text/html";
  if ([filename hasSuffix:@"png"]) {
    mime_type = @"image/png";
  } else if ([filename hasSuffix:@"gif"]) {
    mime_type = @"image/gif";
  }

  NSString *url = [@"file:///" 
                      stringByAppendingString:[filename lastPathComponent]];
  // Uncomment for debugging purposes.
  // NSLog(@"%@ %@ %@", url, filename, mime_type);

  WebResource *ret = [[[WebResource alloc]
                          initWithData:data
                                   URL:[NSURL URLWithString:url]
                              MIMEType:mime_type
                      textEncodingName:nil
                             frameName:nil] autorelease];
  return ret;
}

int main (int argc, const char * argv[]) {
  if (argc < 2) usage();
  
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  
  NSString *webarchive_name = [NSString stringWithCString:argv[1]];
  NSString *main_resource_name = [NSString stringWithCString:argv[2]];
  
  // Slurp in main page.
  WebResource *main_resource = getResource(main_resource_name);
  NSMutableArray *other_resources = [[[NSMutableArray alloc] init]
                                     autorelease];
  
  
  for (int i = 3; i < argc; i++) {
    NSString *resource_filename = [NSString stringWithCString:argv[i]];
    
    [other_resources addObject:getResource(resource_filename)];
  }
  
  WebArchive *out_archive = [[[WebArchive alloc]
                             initWithMainResource:main_resource
                                     subresources:other_resources
                                 subframeArchives:nil] 
                             autorelease];
  
  if (![[out_archive data] writeToFile:webarchive_name atomically:YES]) {
    printf("Error writing file data!\n");
  }
  
  [pool release];
  return 0;
}
