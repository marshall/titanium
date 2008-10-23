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

// Maintains a small circular buffer of the most recent function calls in
// each thread.  These traces can be extremely helpful in debugging crashes.
// The trace buffers get included in crash reports.
// TODO(cprince): Document how to ensure traces get included in crash reports,
// after we work out the details.
//
// To enable tracing in particular files, compile those files with /fastcap.
//
// To view the trace for the active thread, you can paste the following block
// of commands into WinDbg.  (To view the trace for a different thread, get its
// TEB address using the ~ command, set '@$t0' to that value, and paste all
// except the first two lines below.)
/*
   $$ Set @$t0 to the TEB address of the thread of interest.
   r @$t0 = poi(fs:0x18)
   $$ Compute thread index (from TEB address in @$t0)
   r @$t1 = (@$t0 >> 0xC) & 0x7F
   $$ Retrieve last position in thread's trace buffer (index and limit address)
   r @$t2 = poi(g_trace_positions+(4*@$t1)) & 0x1F
   r @$t3 = g_trace_buffers+(0x100*@$t1)+(0x8*@$t2)+(0x8-1)
   $$ Print the trace, with the last calls at the bottom.
   .printf "Last write was: index %p (last addr = %p)\n", @$t2, @$t3
   dps @$t3+1 g_trace_buffers+(0x100*@$t1)+(0x100-1)
   dps g_trace_buffers+(0x100*@$t1) @$t3
*/
//
// The FastCAP feature is not well documented, but here is an overview:
//
// USAGE:
//   Add the "/fastcap" cl.exe flag to all files you want to instrument.
//   But don't instrument the file containing the instrumentation functions,
//   or you may introduce an infinite loop!
//
// BEHAVIOR:
//   When any function call occurs in an instrumented file:
//   (1) calls _CAP_Start_Profiling right before the 'call' instruction
//       (but after argument setup)
//   (2) calls _CAP_End_Profiling after the function returns (and after the
//       caller does cleanup, if the calling convention requires it)
//
//   _CAP_Start_Profiling has the arguments (addr of caller,
//                                           addr of function being called)
//   _CAP_End_Profiling has the argument (addr of caller)
//
// ILLUSTRATIVE EXAMPLE:
//   void main(void) {
//     [snip]
//     ; AnyFunc(1, 2, 3)
//     00401086 6A 03            push        3    
//     00401088 6A 02            push        2    
//     0040108A 6A 01            push        1    
//     0040108C 68 40 10 40 00   push        offset AnyFunc (401040h) 
//     00401091 68 80 10 40 00   push        offset main (401080h) 
//     00401096 E8 65 00 00 00   call        _CAP_Start_Profiling (401100h) 
//     0040109B E8 A0 FF FF FF   call        AnyFunc (401040h) 
//     004010A0 83 C4 0C         add         esp,0Ch 
//     004010A3 68 80 10 40 00   push        offset main (401080h) 
//     004010A8 E8 73 00 00 00   call        _CAP_End_Profiling (401120h) 
//     [snip]
// }
//
// PROTOTYPES:
//   extern "C"
//   void __stdcall _CAP_Start_Profiling(void* caller, void* callee)
// 
//   extern "C"
//   void __stdcall _CAP_End_Profiling(void* caller)


#ifdef _CAP_PROFILING
#error Do not build this file with /fastcap or /callcap.
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "trace_buffers_win32.h"
#include <windows.h>

// Declarations to use the intrinsic version of InterlockedIncrement.
extern "C" long __cdecl _InterlockedIncrement(long volatile* value);
#pragma intrinsic (_InterlockedIncrement)
#define InterlockedIncrement _InterlockedIncrement


// Note: any named symbols we want to access later must be declared non-static.

// It's fine for these arrays to be uninitialized.
void* g_trace_buffers[kNumTraces][kRecordsPerTrace][kValuesPerRecord];
long  g_trace_positions[kNumTraces];  // 'long' for _InterlockedIncrement

int TraceReturnedTo;  // named symbol, to help annotate trace dump


// Returns an index for the current thread in the range 0 <= index < kNumTraces.
// This implementation uses bits of the thread environment block (TEB) address
// to determine the index.  This isn't perfect, but it works well in practice,
// and the failure case (i.e. possible trace buffer collisions) is not bad.
int CurrentThreadTraceIndex() {
  const int kThreadIndexFromTebShift = 12;
  return (reinterpret_cast<int>(NtCurrentTeb()) >> kThreadIndexFromTebShift)
         & (kNumTraces - 1);
}


// Functions used by /fastcap
//
// Note the use of ASM 'push' and 'pop'.  We must be careful not to change any
// register state inside the CAP functions.  As of VC 8.0, the compiler blindly
// inserts CAP calls around each 'call' instruction.  Modifying any register
// inside the CAP routines can break the original code.  For example:
//   mov eax, FunctionAddr
//   call _CAP_Start_Profiling  <-- inserted by compiler; better not touch eax!
//   call eax
//
// TODO(cprince): Look at improving the generated code in these routines.  The
// generated opcodes have obvious inefficiecies, even with optimizations
// enabled.  However, be careful not to introduce machine-specific assumptions
// or code that breaks when the constants above change.

extern "C"
void __stdcall _CAP_Start_Profiling(void* caller, void* callee) {
  __asm {
    pushad
    pushfd
  }

  int thread_index = CurrentThreadTraceIndex();

  // Interlocked call helps if thread indices collide. Should be fairly cheap.
  int record_index =
      InterlockedIncrement(&g_trace_positions[thread_index]);
  record_index &= (kRecordsPerTrace - 1);

  g_trace_buffers[thread_index][record_index][0] = caller;
  g_trace_buffers[thread_index][record_index][1] = callee;

  __asm {
    popfd
    popad
  }
}

extern "C"
void __stdcall _CAP_End_Profiling(void* caller) {
  __asm {
    pushad
    pushfd
  }

  int thread_index = CurrentThreadTraceIndex();

  // Interlocked call helps if thread indices collide. Should be fairly cheap.
  int record_index =
      InterlockedIncrement(&g_trace_positions[thread_index]);
  record_index &= (kRecordsPerTrace - 1);

  g_trace_buffers[thread_index][record_index][0] = &TraceReturnedTo;
  g_trace_buffers[thread_index][record_index][1] = caller;

  __asm {
    popfd
    popad
  }
}
