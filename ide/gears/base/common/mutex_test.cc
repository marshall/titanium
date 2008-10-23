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

#ifdef USING_CCTESTS

#include "gears/base/common/mutex.h"
#include "gears/base/common/string_utils.h"
#include "gears/base/common/thread.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define LOCATION __FILE__ ", line " TOSTRING(__LINE__)
#undef TEST_ASSERT
#define TEST_ASSERT(b) \
{ \
  if (!(b)) { \
    LOG(("failed at " LOCATION)); \
    assert(error); \
    if (!error->empty()) *error += STRING16(L", "); \
    *error += STRING16(L"failed at "); \
    std::string16 location; \
    UTF8ToString16(LOCATION, &location); \
    *error += location; \
    return false; \
  } \
}

static bool TestCondition(std::string16 *error);
static bool TestCondVar(std::string16 *error);
static bool TestMutex(std::string16 *error);
static bool TestMutexLock(std::string16 *error);

bool TestAllMutex(std::string16 *error) {
  bool ok = true;
  ok &= TestCondition(error);
  ok &= TestCondVar(error);
  ok &= TestMutex(error);
  ok &= TestMutexLock(error);
  return ok;
}

//------------------------------------------------------------------------------
// TestCondition
//------------------------------------------------------------------------------

static bool TestCondition(std::string16 *error) {
  // TODO(bgarcia): implement Condition tests
  return true;
}

//------------------------------------------------------------------------------
// TestCondVar
//------------------------------------------------------------------------------

namespace {

class WaitCondVar : public Thread {
 public:
  WaitCondVar(CondVar *cond_var, Mutex *mutex, bool* gate, int timeout = -1)
      : ran_(false), cond_var_(cond_var), external_mutex_(mutex),
        gate_(gate), timeout_(timeout) {
  }
  virtual ~WaitCondVar() {
  }
  void Run() {
    {
      MutexLock locker(external_mutex_);
      while (!*gate_) {
        if (timeout_ < 0) {
          cond_var_->Wait(external_mutex_);
        } else {
          cond_var_->WaitWithTimeout(external_mutex_, timeout_);
        }
      }
    }
    MutexLock locker(&mutex_);
    ran_ = true;
  }
  bool HasRun() const {
    MutexLock locker(&mutex_);
    return ran_;
  }
 private:
  mutable Mutex mutex_;
  bool ran_;

  CondVar *cond_var_;
  Mutex *external_mutex_;
  bool *gate_;
  int timeout_;
};

}  // namespace

static bool TestCondVar(std::string16 *error) {
  bool ok = true;

  Mutex mutex;
  CondVar cond_var;

  // Test WaitWithTimeout with timeout set to zero.
  {
    MutexLock locker(&mutex);
    bool result = cond_var.WaitWithTimeout(&mutex, 0);
    TEST_ASSERT(result);
  }

  // Test WaitWithTimeout with a non-zero timeout.
  {
    MutexLock locker(&mutex);
    bool result = cond_var.WaitWithTimeout(&mutex, 10);
    TEST_ASSERT(result);
  }

  // Test SignalAll and Wait.
  bool gate;
  {
    gate = false;
    WaitCondVar wait_thread(&cond_var, &mutex, &gate);
    wait_thread.Start();
    while (!wait_thread.IsRunning()) {
      TEST_ASSERT(!wait_thread.HasRun());
      ThreadYield();
    }
    TEST_ASSERT(!wait_thread.HasRun());

    {
      MutexLock locker(&mutex);
      gate = true;
    }
    cond_var.SignalAll();

    while (wait_thread.IsRunning()) {
      ThreadYield();
    }
    TEST_ASSERT(wait_thread.HasRun());
    wait_thread.Join();
  }

  // Test SignalAll and Wait with multiple waiters.
  {
    gate = false;
    const int num = 9;
    WaitCondVar *wait_thread[num];
    for (int i = 0; i < num; ++i) {
      wait_thread[i] = new WaitCondVar(&cond_var, &mutex, &gate);
      wait_thread[i]->Start();
    }
    for (int i = 0; i < num; ++i) {
      while (!wait_thread[i]->IsRunning()) {
        TEST_ASSERT(!wait_thread[i]->HasRun());
        ThreadYield();
      }
      TEST_ASSERT(!wait_thread[i]->HasRun());
    }

    {
      MutexLock locker(&mutex);
      gate = true;
    }
    cond_var.SignalAll();

    for (int i = 0; i < num; ++i) {
      while (wait_thread[i]->IsRunning()) {
        ThreadYield();
      }
      TEST_ASSERT(wait_thread[i]->HasRun());
      wait_thread[i]->Join();
      delete wait_thread[i];
    }
  }

  return ok;
}

//------------------------------------------------------------------------------
// TestMutex
//------------------------------------------------------------------------------

static bool TestMutex(std::string16 *error) {
  // TODO(bgarcia): implement Mutex tests
  return true;
}

//------------------------------------------------------------------------------
// TestMutexLock
//------------------------------------------------------------------------------

static bool TestMutexLock(std::string16 *error) {
  // TODO(bgarcia): implement MutexLock tests
  return true;
}

#endif  // USING_CCTESTS
