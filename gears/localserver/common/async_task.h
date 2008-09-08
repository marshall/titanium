// Copyright 2006, Google Inc.
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

#ifndef GEARS_LOCALSERVER_COMMON_ASYNC_TASK_H__
#define GEARS_LOCALSERVER_COMMON_ASYNC_TASK_H__

//------------------------------------------------------------------------------
// AsyncTask is a base class for two types of asynchronous tasks in the
// webcache system. The base class provides:
//
// * framing to execute the Run() method of derived classes in a worker thread
// * a method to perform HTTP requests which appear synchronous to the calling
//   code in the worker thread
// * a means to send notification messages to a listener
// * a means to gracefully abort a running task
//
// The implementation is browser specific.
// 
// See ie_async_task.h, ff_async_task.h (browser specific implementations)
// See update_task.h, web_capture_task.h (derived classes)
//------------------------------------------------------------------------------

#if BROWSER_IE
#include "gears/localserver/ie/async_task_ie.h"
#elif BROWSER_FF
#include "gears/localserver/firefox/async_task_ff.h"
#elif BROWSER_SAFARI
#include "gears/localserver/safari/async_task_sf.h"
#elif BROWSER_NPAPI
#include "gears/localserver/npapi/async_task_np.h"
#endif

#endif  // GEARS_LOCALSERVER_COMMON_ASYNC_TASK_H__
