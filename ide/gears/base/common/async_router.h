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

#ifndef GEARS_BASE_COMMON_ASYNC_ROUTER_H__
#define GEARS_BASE_COMMON_ASYNC_ROUTER_H__

#include "gears/base/common/basictypes.h" // for DISALLOW_EVIL_CONSTRUCTORS
#include "gears/base/common/message_queue.h"

// Clients implement this interface to receive an asynchronous call.
class AsyncFunctor : public MessageData {
public:
  virtual void Run() = 0;
};

// This is a simple class that facilitates running a method asynchronously on
// a target thread.  The ThreadMessageQueue with the given thread_id must be
// initialized.
class AsyncRouter : public ThreadMessageQueue::HandlerInterface {
 public:
  // Returns a pointer to the AsyncRouter singleton.
  static AsyncRouter *GetInstance();

  // Calls the functor's Run() method from the thread specified by thread_id.
  // Ownership of the functor is transferred to the AsyncRouter.
  bool CallAsync(ThreadId thread_id, AsyncFunctor *functor);

 private:
  AsyncRouter();
  ~AsyncRouter();

  // ThreadMessageQueue::HandlerInterface override
  virtual void HandleThreadMessage(int message_type, MessageData *message_data);

  DISALLOW_EVIL_CONSTRUCTORS(AsyncRouter);
};

#endif  // GEARS_BASE_COMMON_ASYNC_ROUTER_H__
