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

#ifndef GEARS_DATABASE2_INTERPRETER_H__
#define GEARS_DATABASE2_INTERPRETER_H__

#include "gears/base/common/common.h"
#include "gears/base/common/scoped_refptr.h"

// forward class declarations
class Database2Command;

// simple (non-threaded) command interpreter
class Database2Interpreter : public RefCounted {
 public:
  // TODO(dimitri.glazkov): Because the sub-class depends on ref-counting to
  // keep track of when to shut down a thread, this base class is also
  // ref-counted. However, its counting is effectively rendered useless by
  // Ref'ing in constructor. This is awkward. Must think of a better pattern
  // here.
  Database2Interpreter() {
    // increment ref to prevent from ever being destroyed
    if (!async()) Ref();
  }
  ~Database2Interpreter() {}

  virtual void Run(Database2Command *command);
  virtual bool async() const { return false; }

 private:
  bool async_;

  DISALLOW_EVIL_CONSTRUCTORS(Database2Interpreter);
};

// threaded interpreter
class Database2ThreadedInterpreter : public Database2Interpreter {
 public:
  Database2ThreadedInterpreter() {}
  ~Database2ThreadedInterpreter() {
    // shut down thread, if started
  }

  virtual void Run(Database2Command *command);
  virtual bool async() const { return true; }

 private:
  DISALLOW_EVIL_CONSTRUCTORS(Database2ThreadedInterpreter);
};

#endif // GEARS_DATABASE2_INTERPRETER_H__
