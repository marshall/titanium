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

#if USING_CCTESTS

#include "gears/base/common/event.h"
#include "gears/base/common/string16.h"
#include "gears/localserver/common/async_task.h"

class AsyncTaskImpl : public AsyncTask {
 public:
   AsyncTaskImpl() : AsyncTask(NULL), result_(false) {
    if (!Init()) {
      assert(false);
    }
    if (!Start()) {
      assert(false);
    }
  }
  bool GetResultAndDelete() {
    get_result_event_.Signal();
    if (!have_result_event_.WaitWithTimeout(1000)) {
      return false;
    }
    bool result = result_;
    DeleteWhenDone();
    // Use a local variable since the class has been deleted.
    return result;
  }
 private:
  // Deleted with GetResultAndDelete().
  virtual ~AsyncTaskImpl() {}
  // AsyncTask implementation.
  virtual void Run() {
    get_result_event_.Wait();
    result_ = true;
    have_result_event_.Signal();
  }
  Event get_result_event_;
  Event have_result_event_;
  bool result_;
  DISALLOW_EVIL_CONSTRUCTORS(AsyncTaskImpl);
};

bool TestEvent(std::string16 *error) {
#undef TEST_ASSERT
#define TEST_ASSERT(b) \
{ \
  if (!(b)) { \
    LOG(("TestEvent - failed (%d)\n", __LINE__)); \
    assert(error); \
    *error += STRING16(L"TestEvent failed. "); \
    return false; \
  } \
}

  Event the_event;
  // Test timing out.
  TEST_ASSERT(the_event.WaitWithTimeout(10) == false);
  // Test calling Signal() before Wait() - Wait() should proceed immediately.
  the_event.Signal();
  TEST_ASSERT(the_event.WaitWithTimeout(10));
  // Test event gets reset
  TEST_ASSERT(the_event.WaitWithTimeout(10) == false);
  // Test multiple calls to Signal() make no difference.
  the_event.Signal();
  the_event.Signal();
  TEST_ASSERT(the_event.WaitWithTimeout(10));
  TEST_ASSERT(the_event.WaitWithTimeout(10) == false);
  // Test a simple AsyncTask implementation that uses events from two threads.
  AsyncTaskImpl *async_task = new AsyncTaskImpl();
  TEST_ASSERT(async_task->GetResultAndDelete());

  return true;
}

#endif  // USING_CCTESTS
