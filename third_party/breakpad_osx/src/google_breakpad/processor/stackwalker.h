// Copyright (c) 2006, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// stackwalker.h: Generic stackwalker.
//
// The Stackwalker class is an abstract base class providing common generic
// methods that apply to stacks from all systems.  Specific implementations
// will extend this class by providing GetContextFrame and GetCallerFrame
// methods to fill in system-specific data in a StackFrame structure.
// Stackwalker assembles these StackFrame strucutres into a CallStack.
//
// Author: Mark Mentovai


#ifndef GOOGLE_BREAKPAD_PROCESSOR_STACKWALKER_H__
#define GOOGLE_BREAKPAD_PROCESSOR_STACKWALKER_H__

#include <vector>

namespace google_breakpad {

class CallStack;
class CodeModules;
template<typename T> class linked_ptr;
class MemoryRegion;
class MinidumpContext;
class SourceLineResolverInterface;
struct StackFrame;
struct StackFrameInfo;
class SymbolSupplier;
class SystemInfo;

using std::vector;


class Stackwalker {
 public:
  virtual ~Stackwalker() {}

  // Populates the given CallStack by calling GetContextFrame and
  // GetCallerFrame.  The frames are further processed to fill all available
  // data.  Returns true if the stackwalk completed, or false if it was
  // interrupted by SymbolSupplier::GetSymbolFile().
  bool Walk(CallStack *stack);

  // Returns a new concrete subclass suitable for the CPU that a stack was
  // generated on, according to the CPU type indicated by the context
  // argument.  If no suitable concrete subclass exists, returns NULL.
  static Stackwalker* StackwalkerForCPU(const SystemInfo *system_info,
                                        MinidumpContext *context,
                                        MemoryRegion *memory,
                                        const CodeModules *modules,
                                        SymbolSupplier *supplier,
                                        SourceLineResolverInterface *resolver);

 protected:
  // system_info identifies the operating system, NULL or empty if unknown.
  // memory identifies a MemoryRegion that provides the stack memory
  // for the stack to walk.  modules, if non-NULL, is a CodeModules
  // object that is used to look up which code module each stack frame is
  // associated with.  supplier is an optional caller-supplied SymbolSupplier
  // implementation.  If supplier is NULL, source line info will not be
  // resolved.  resolver is an instance of SourceLineResolverInterface
  // (see source_line_resolver_interface.h and basic_source_line_resolver.h).
  // If resolver is NULL, source line info will not be resolved.
  Stackwalker(const SystemInfo *system_info,
              MemoryRegion *memory,
              const CodeModules *modules,
              SymbolSupplier *supplier,
              SourceLineResolverInterface *resolver);

  // Information about the system that produced the minidump.  Subclasses
  // and the SymbolSupplier may find this information useful.
  const SystemInfo *system_info_;

  // The stack memory to walk.  Subclasses will require this region to
  // get information from the stack.
  MemoryRegion *memory_;

  // A list of modules, for populating each StackFrame's module information.
  // This field is optional and may be NULL.
  const CodeModules *modules_;

 private:
  // Obtains the context frame, the innermost called procedure in a stack
  // trace.  Returns NULL on failure.  GetContextFrame allocates a new
  // StackFrame (or StackFrame subclass), ownership of which is taken by
  // the caller.
  virtual StackFrame* GetContextFrame() = 0;

  // Obtains a caller frame.  Each call to GetCallerFrame should return the
  // frame that called the last frame returned by GetContextFrame or
  // GetCallerFrame.  To aid this purpose, stack contains the CallStack
  // made of frames that have already been walked.  GetCallerFrame should
  // return NULL on failure or when there are no more caller frames (when
  // the end of the stack has been reached).  GetCallerFrame allocates a new
  // StackFrame (or StackFrame subclass), ownership of which is taken by
  // the caller.
  virtual StackFrame* GetCallerFrame(
      const CallStack *stack,
      const vector< linked_ptr<StackFrameInfo> > &stack_frame_info) = 0;

  // The optional SymbolSupplier for resolving source line info.
  SymbolSupplier *supplier_;

  // The SourceLineResolver implementation
  SourceLineResolverInterface *resolver_;
};


}  // namespace google_breakpad


#endif  // GOOGLE_BREAKPAD_PROCESSOR_STACKWALKER_H__
