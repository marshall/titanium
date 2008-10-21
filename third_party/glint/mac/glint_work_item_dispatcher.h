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

// GlintWorkItemDispatcher: dispatch a Glint Work Item

#import <Foundation/Foundation.h>

#import "glint/include/root_ui.h"
#import "glint/include/work_item.h"

using glint::WorkItem;
using glint::RootUI;

// Used to hold Glint WorkItem and manage its execution.
// When Glint creates a Workitem for async execution on UI thread, an instance
// of this class is created, WorkItem pointer stored in it and it is registered
// with main UI thread message pump by means of performSelectorOnMainThread.
// This will eventually cause 'dispatch' selector to be invoked by message pump.
// We also keep a reference to this object in a global list attached to Glint
// window, to 'cancel' all scheduled but not yet executed work items when the
// window is closed.
@interface GlintWorkItemDispatcher : NSObject {
  NSMutableSet* dispatchers_;  // weak reference to containing list.
  RootUI* ui_;  // weak C++ reference.
  WorkItem* workItem_;  // strong C++ reference, we release it.
  BOOL isCanceled_;
}

// Creates a GlintWorkItemDispatcher for the given work item for
// the given Glint ui.
+ (GlintWorkItemDispatcher*)dispatcherForWorkItem:(WorkItem*)item
                                           withUI:(RootUI*)ui
                                        container:(NSMutableSet*)dispatchers;

// Designated initializer, initializes the dispatcher with the given work item
// for the given Glint ui.
// Adds the dispatcher to the list of work items so it can be canceled when the 
// ui closes.
- (id)initForWorkItem:(WorkItem*)item
               withUI:(RootUI*)ui
            container:(NSMutableSet*)dispatchers;

// Dispatches the work item on the Glint ui it was created for. This is never
// called directly. It only serves as a target for a run loop.
- (void)dispatch;

// Once dispatcher is queued for a callback on UI thread, it is not always
// possible to cancel the callback itself. This method marks dispatcher
// 'canceled' so subsequent callback is a noop.
- (void)cancel;

@end
