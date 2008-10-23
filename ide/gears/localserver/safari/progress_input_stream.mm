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

#import "gears/localserver/safari/progress_input_stream.h"

#import "gears/localserver/common/progress_event.h"
#import "gears/localserver/safari/http_request_sf.h"

//------------------------------------------------------------------------------
// ProgressInputStream implementation
//------------------------------------------------------------------------------
@implementation ProgressInputStream

#pragma mark Public instance methods

- (id)initFromStream:(NSInputStream *)input_stream
             request:(SFHttpRequest *)request
               total:(int64)total {
  self = [super init];
  if (self != nil) {
    input_stream_ = [input_stream retain];
    request_ = request;
    request_->Ref();
    total_ = total;
  }
  return self;
}

- (void)dealloc {
  request_->Unref();
  [input_stream_ release];
  [super dealloc];
}

#pragma mark NSInputStream function overrides.

- (NSInteger)read:(uint8_t *)buffer maxLength:(NSUInteger)len {
  NSInteger bytes_read = [input_stream_ read:buffer maxLength:len];
  if (bytes_read > 0) {
    position_ += bytes_read;
    ProgressEvent::Update(request_, request_, position_, total_);
  }
  return bytes_read;
}

- (BOOL)getBuffer:(uint8_t **)buffer length:(NSUInteger *)len {
  return [input_stream_ getBuffer:buffer length:len];
}

- (BOOL)hasBytesAvailable {
  return [input_stream_ hasBytesAvailable];
}

#pragma mark -
// we'll forward all unhandled messages to the NSInputStream class
// or to the encapsulated input stream.  This is needed
// for all messages sent to NSInputStream which aren't
// handled by our superclass; that includes various private run
// loop calls.
+ (NSMethodSignature*)methodSignatureForSelector:(SEL)selector {
  return [NSInputStream methodSignatureForSelector:selector];
}

+ (void)forwardInvocation:(NSInvocation*)invocation {  
  [invocation invokeWithTarget:[NSInputStream class]];
}

- (NSMethodSignature*)methodSignatureForSelector:(SEL)selector {
  return [input_stream_ methodSignatureForSelector:selector];
}

- (void)forwardInvocation:(NSInvocation*)invocation {    
  [invocation invokeWithTarget:input_stream_];
}

#pragma mark Standard messages
// We want our encapsulated NSInputStream to handle the standard messages;
// we don't want the superclass to handle them.

- (void)open {
  [input_stream_ open]; 
}

- (void)close {
  [input_stream_ close]; 
}

- (id)delegate {
  return [input_stream_ delegate]; 
}

- (void)setDelegate:(id)delegate {
  [input_stream_ setDelegate:delegate]; 
}

- (id)propertyForKey:(NSString *)key {
  return [input_stream_ propertyForKey:key]; 
}
- (BOOL)setProperty:(id)property forKey:(NSString *)key {
  return [input_stream_ setProperty:property forKey:key]; 
}

- (void)scheduleInRunLoop:(NSRunLoop *)aRunLoop forMode:(NSString *)mode {
  [input_stream_ scheduleInRunLoop:aRunLoop forMode:mode]; 
}
- (void)removeFromRunLoop:(NSRunLoop *)aRunLoop forMode:(NSString *)mode {
  [input_stream_ removeFromRunLoop:aRunLoop forMode:mode]; 
}

- (NSStreamStatus)streamStatus {
  return [input_stream_ streamStatus]; 
}

- (NSError *)streamError {
  return [input_stream_ streamError];
}

@end
