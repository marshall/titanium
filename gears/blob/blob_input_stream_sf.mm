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

#import <cassert>

#import "gears/blob/blob_input_stream_sf.h"
#import "gears/blob/blob_interface.h"

@implementation BlobInputStream

#pragma mark Public Instance methods
//------------------------------------------------------------------------------
// BlobInputStream implementation
//------------------------------------------------------------------------------

- (id)initFromBlob:(BlobInterface *)blob {
  self = [super init];
  if (self) {
    blob->Ref();
    blob_ = blob;

    [self setDelegate:self];

    // We use a dummy input stream to handle all the various undocumented
    // messages the system sends to an input stream.

    // Contrary to documentation, inputStreamWithData neither copies nor
    // retains the data in Mac OS X 10.4, so we must retain it.
    dummy_data_ = [[NSData alloc] initWithBytes:"x" length:1];
    dummy_stream_ = [[NSInputStream alloc] initWithData:dummy_data_];
  }
  return self;
}

- (void)dealloc {
  if (blob_) {
    blob_->Unref();
  }
  [dummy_stream_ release];
  [dummy_data_ release];
  [super dealloc];
}

- (void)reset:(BlobInterface *)blob {
  if (blob) {
    blob->Ref();
  }
  if (blob_) {
    blob_->Unref();
  }
  blob_ = blob;
  offset_ = 0;

  // Create a new dummy stream, in case the old one is in the fully-read state.
  if (!dummy_data_) {
    dummy_data_ = [[NSData alloc] initWithBytes:"x" length:1];
  }
  [dummy_stream_ release];
  dummy_stream_ = [[NSInputStream alloc] initWithData:dummy_data_];
}

#pragma mark NSInputStream function overrides.
//------------------------------------------------------------------------------
// NSInputStream implementation
//------------------------------------------------------------------------------

- (NSInteger)read:(uint8_t *)buffer maxLength:(NSUInteger)len {
  if (blob_ == NULL) {
    return 0;
  }
  int64 numBytesRead = blob_->Read(buffer, offset_, len);
  if (numBytesRead < 0) {
    // It doesn't appear to be documented anywhere, but returning -1
    // can be used to indicate an error.  Testable by uploading a large
    // file using http://localhost:8001/manual/httprequest_progress.html
    // and touching the file during the upload.
    return -1;
  }
  // Confirm that BlobInterface::Read returned <= bytes requested
  assert(numBytesRead <= static_cast<int64>(len));
  offset_ += numBytesRead;
  if (numBytesRead == 0) {
    // We are at the end our our stream, so we read all of the data on our
    // dummy input stream to make sure it is in the "fully read" state.
    uint8_t buffer[2];
    (void) [dummy_stream_ read:buffer maxLength:sizeof(buffer)];
  }
  return static_cast<NSInteger>(numBytesRead);
}

- (BOOL)getBuffer:(uint8_t **)buffer length:(NSUInteger *)len {
  // From the NSInputStream documentation:
  // "If this method is not appropriate for your type of stream,
  //  you may return NO."
  return NO;
}

- (BOOL)hasBytesAvailable {
#if 1
  // There appears to be a bug in OSX 10.4 that requires us to always return
  // YES, otherwise the NSURLConnection may not completely read the stream.
  return YES;
#else
  if (blob_ == NULL) {
    return NO;
  }
  int64 remaining = blob_->Length() - offset_;
  if (remaining <= 0) {
    return NO;
  }
  return YES;
#endif
}

#pragma mark -
// Pass other expected messages on to the dummy input stream.

- (void)open {
  [dummy_stream_ open];
}

- (void)close {
  [dummy_stream_ close];
}

- (void)stream:(NSStream *)theStream handleEvent:(NSStreamEvent)streamEvent {
  if (delegate_ != self) {
    [delegate_ stream:self handleEvent:streamEvent];
  }
}

- (id)delegate {
  return delegate_;
}

- (void)setDelegate:(id)delegate {
  if (delegate == nil) {
    delegate_ = self;
    [dummy_stream_ setDelegate:nil];
  } else {
    delegate_ = delegate;
    [dummy_stream_ setDelegate:self];
  }
}

- (id)propertyForKey:(NSString *)key {
  return [dummy_stream_ propertyForKey:key];
}

- (BOOL)setProperty:(id)property forKey:(NSString *)key {
  return [dummy_stream_ setProperty:property forKey:key];
}

- (void)scheduleInRunLoop:(NSRunLoop *)aRunLoop forMode:(NSString *)mode {
  [dummy_stream_ scheduleInRunLoop:aRunLoop forMode:mode];
}

- (void)removeFromRunLoop:(NSRunLoop *)aRunLoop forMode:(NSString *)mode {
  [dummy_stream_ removeFromRunLoop:aRunLoop forMode:mode];
}

- (NSStreamStatus)streamStatus {
  return [dummy_stream_ streamStatus];
}
- (NSError *)streamError {
  return [dummy_stream_ streamError];
}

#pragma mark -

// We'll forward all unexpected messages to our dummy stream

+ (NSMethodSignature*)methodSignatureForSelector:(SEL)selector {
  return [NSInputStream methodSignatureForSelector:selector];
}

+ (void)forwardInvocation:(NSInvocation*)invocation {
  [invocation invokeWithTarget:[NSInputStream class]];
}

- (NSMethodSignature*)methodSignatureForSelector:(SEL)selector {
  return [dummy_stream_ methodSignatureForSelector:selector];
}

- (void)forwardInvocation:(NSInvocation*)invocation {
  [invocation invokeWithTarget:dummy_stream_];
}

@end
