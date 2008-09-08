// Copyright 2003, Google Inc.
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
//
// Implementation of atomic operations for x86.  This file should not be
// included directly.  Clients should instead include "atomicops.h".

#ifndef GEARS_BASE_COMMON_ATOMIC_OPS_LINUX_H__
#define GEARS_BASE_COMMON_ATOMIC_OPS_LINUX_H__

#include <stdint.h>

typedef intptr_t AtomicWord;
typedef int32_t Atomic32;

// There are a couple places we need to specialize opcodes to account for the
// different AtomicWord sizes on x86_64 and 32-bit platforms.
// This macro is undefined after its last use, below.
#if defined(__x86_64__)
#define ATOMICOPS_WORD_SUFFIX "q"
#else
#define ATOMICOPS_WORD_SUFFIX "l"
#endif

inline AtomicWord CompareAndSwap(volatile AtomicWord* ptr,
                                 AtomicWord old_value,
                                 AtomicWord new_value) {
  AtomicWord prev;
  __asm__ __volatile__("lock; cmpxchg" ATOMICOPS_WORD_SUFFIX " %1,%2"
                       : "=a" (prev)
                       : "q" (new_value), "m" (*ptr), "0" (old_value)
                       : "memory");
  return prev;
}

inline AtomicWord AtomicExchange(volatile AtomicWord* ptr,
                                 AtomicWord new_value) {
  __asm__ __volatile__("xchg" ATOMICOPS_WORD_SUFFIX " %1,%0" // The lock prefix
                       : "=r" (new_value)                    // is implicit for
                       : "m" (*ptr), "0" (new_value)         // xchg.
                       : "memory");
  return new_value;  // Now it's the previous value.
}

inline AtomicWord AtomicIncrement(volatile AtomicWord* ptr, AtomicWord increment) {
  AtomicWord temp = increment;
  __asm__ __volatile__("lock; xadd" ATOMICOPS_WORD_SUFFIX " %0,%1"
                       : "+r" (temp), "+m" (*ptr)
                       : : "memory");
  // temp now contains the previous value of *ptr
  return temp + increment;
}

#undef ATOMICOPS_WORD_SUFFIX


// On a 64-bit machine, Atomic32 and AtomicWord are different types,
// so we need to copy the preceding methods for Atomic32.

#if defined(__x86_64__)

inline Atomic32 CompareAndSwap(volatile Atomic32* ptr,
                               Atomic32 old_value,
                               Atomic32 new_value) {
  Atomic32 prev;
  __asm__ __volatile__("lock; cmpxchgl %1,%2"
                       : "=a" (prev)
                       : "q" (new_value), "m" (*ptr), "0" (old_value)
                       : "memory");
  return prev;
}

inline Atomic32 AtomicExchange(volatile Atomic32* ptr,
                               Atomic32 new_value) {
  __asm__ __volatile__("xchgl %1,%0"  // The lock prefix is implicit for xchg.
                       : "=r" (new_value)
                       : "m" (*ptr), "0" (new_value)
                       : "memory");
  return new_value;  // Now it's the previous value.
}

inline Atomic32 AtomicIncrement(volatile Atomic32* ptr, Atomic32 increment) {
  Atomic32 temp = increment;
  __asm__ __volatile__("lock; xaddl %0,%1"
                       : "+r" (temp), "+m" (*ptr)
                       : : "memory");
  // temp now holds the old value of *ptr
  return temp + increment;
}

#endif /* defined(__x86_64__) */

#undef ATOMICOPS_COMPILER_BARRIER

#endif  // GEARS_BASE_COMMON_ATOMIC_OPS_LINUX_H__
