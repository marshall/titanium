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

// crash_report.mm: Convert the contents of a minidump into a format that
// looks more like Apple's CrashReporter format

#include <unistd.h>

#include <mach/machine.h>
#include <mach-o/arch.h>

#include <string>

#include <Foundation/Foundation.h>

#include "google_breakpad/processor/basic_source_line_resolver.h"
#include "google_breakpad/processor/call_stack.h"
#include "google_breakpad/processor/code_module.h"
#include "google_breakpad/processor/minidump.h"
#include "google_breakpad/processor/minidump_processor.h"
#include "google_breakpad/processor/process_state.h"
#include "google_breakpad/processor/stack_frame_cpu.h"
#include "google_breakpad/processor/system_info.h"
#include "processor/pathname_stripper.h"
#include "processor/scoped_ptr.h"
#include "processor/simple_symbol_supplier.h"

#include "on_demand_symbol_supplier.h"

using std::string;

using google_breakpad::BasicSourceLineResolver;
using google_breakpad::CallStack;
using google_breakpad::CodeModule;
using google_breakpad::CodeModules;
using google_breakpad::MinidumpProcessor;
using google_breakpad::OnDemandSymbolSupplier;
using google_breakpad::PathnameStripper;
using google_breakpad::ProcessState;
using google_breakpad::scoped_ptr;
using google_breakpad::StackFrame;
using google_breakpad::StackFramePPC;
using google_breakpad::StackFrameX86;
using google_breakpad::SystemInfo;

typedef struct {
  NSString *minidumpPath;
  NSString *searchDir;
} Options;

//=============================================================================
static int PrintRegister(const char *name, u_int32_t value, int sequence) {
  if (sequence % 4 == 0) {
    printf("\n");
  }
  printf("%6s = 0x%08x ", name, value);
  return ++sequence;
}

//=============================================================================
static void PrintStack(const CallStack *stack, const string &cpu) {
  int frame_count = stack->frames()->size();
  char buffer[1024];
  for (int frame_index = 0; frame_index < frame_count; ++frame_index) {
    const StackFrame *frame = stack->frames()->at(frame_index);
    const CodeModule *module = frame->module;
    printf("%2d ", frame_index);

    if (module) {
      // Module name (20 chars max)
      strcpy(buffer, PathnameStripper::File(module->code_file()).c_str());
      int maxStr = 20;
      buffer[maxStr] = 0;
      printf("%-*s", maxStr, buffer);
      u_int64_t instruction = frame->instruction;

      // PPC only: Adjust the instruction to match that of Crash reporter.  The
      // instruction listed is actually the return address.  See the detailed
      // comments in stackwalker_ppc.cc for more information.
      if (cpu == "ppc" && frame_index)
        instruction += 4;

      printf(" 0x%08llx ", instruction);

      // Function name
      if (!frame->function_name.empty()) {
        printf("%s", frame->function_name.c_str());
        if (!frame->source_file_name.empty()) {
          string source_file = PathnameStripper::File(frame->source_file_name);
          printf(" + 0x%llx (%s:%d)",
                 instruction - frame->source_line_base,
                 source_file.c_str(), frame->source_line);
        } else {
          printf(" + 0x%llx", instruction - frame->function_base);
        }
      }
    }
    printf("\n");
  }
}

//=============================================================================
static void PrintRegisters(const CallStack *stack, const string &cpu) {
  int sequence = 0;
  const StackFrame *frame = stack->frames()->at(0);
  if (cpu == "x86") {
    const StackFrameX86 *frame_x86 =
      reinterpret_cast<const StackFrameX86*>(frame);

    if (frame_x86->context_validity & StackFrameX86::CONTEXT_VALID_EIP)
      sequence = PrintRegister("eip", frame_x86->context.eip, sequence);
    if (frame_x86->context_validity & StackFrameX86::CONTEXT_VALID_ESP)
      sequence = PrintRegister("esp", frame_x86->context.esp, sequence);
    if (frame_x86->context_validity & StackFrameX86::CONTEXT_VALID_EBP)
      sequence = PrintRegister("ebp", frame_x86->context.ebp, sequence);
    if (frame_x86->context_validity & StackFrameX86::CONTEXT_VALID_EBX)
      sequence = PrintRegister("ebx", frame_x86->context.ebx, sequence);
    if (frame_x86->context_validity & StackFrameX86::CONTEXT_VALID_ESI)
      sequence = PrintRegister("esi", frame_x86->context.esi, sequence);
    if (frame_x86->context_validity & StackFrameX86::CONTEXT_VALID_EDI)
      sequence = PrintRegister("edi", frame_x86->context.edi, sequence);
    if (frame_x86->context_validity == StackFrameX86::CONTEXT_VALID_ALL) {
      sequence = PrintRegister("eax", frame_x86->context.eax, sequence);
      sequence = PrintRegister("ecx", frame_x86->context.ecx, sequence);
      sequence = PrintRegister("edx", frame_x86->context.edx, sequence);
      sequence = PrintRegister("efl", frame_x86->context.eflags, sequence);
    }
  } else if (cpu == "ppc") {
    const StackFramePPC *frame_ppc =
      reinterpret_cast<const StackFramePPC*>(frame);

    if (frame_ppc->context_validity & StackFramePPC::CONTEXT_VALID_ALL ==
        StackFramePPC::CONTEXT_VALID_ALL) {
      sequence = PrintRegister("srr0", frame_ppc->context.srr0, sequence);
      sequence = PrintRegister("srr1", frame_ppc->context.srr1, sequence);
      sequence = PrintRegister("cr", frame_ppc->context.cr, sequence);
      sequence = PrintRegister("xer", frame_ppc->context.xer, sequence);
      sequence = PrintRegister("lr", frame_ppc->context.lr, sequence);
      sequence = PrintRegister("ctr", frame_ppc->context.ctr, sequence);
      sequence = PrintRegister("mq", frame_ppc->context.mq, sequence);
      sequence = PrintRegister("vrsave", frame_ppc->context.vrsave, sequence);

      sequence = 0;
      char buffer[5];
      for (int i = 0; i < MD_CONTEXT_PPC_GPR_COUNT; ++i) {
        sprintf(buffer, "r%d", i);
        sequence = PrintRegister(buffer, frame_ppc->context.gpr[i], sequence);
      }
    } else {
      if (frame_ppc->context_validity & StackFramePPC::CONTEXT_VALID_SRR0)
        sequence = PrintRegister("srr0", frame_ppc->context.srr0, sequence);
      if (frame_ppc->context_validity & StackFramePPC::CONTEXT_VALID_GPR1)
        sequence = PrintRegister("r1", frame_ppc->context.gpr[1], sequence);
    }
  }

  printf("\n");
}

//=============================================================================
static void Start(Options *options) {
  string minidump_file([options->minidumpPath fileSystemRepresentation]);

  BasicSourceLineResolver resolver;
  string search_dir = options->searchDir ?
    [options->searchDir fileSystemRepresentation] : "";
  scoped_ptr<OnDemandSymbolSupplier> symbol_supplier(
    new OnDemandSymbolSupplier(search_dir));
  scoped_ptr<MinidumpProcessor>
    minidump_processor(new MinidumpProcessor(symbol_supplier.get(), &resolver));
  ProcessState process_state;
  if (minidump_processor->Process(minidump_file, &process_state) !=
      MinidumpProcessor::PROCESS_OK) {
    fprintf(stderr, "MinidumpProcessor::Process failed\n");
    return;
  }

  const SystemInfo *system_info = process_state.system_info();
  string cpu = system_info->cpu;

  // Convert the time to a string
  u_int32_t time_date_stamp = process_state.time_date_stamp();
  struct tm timestruct;
  gmtime_r(reinterpret_cast<time_t*>(&time_date_stamp), &timestruct);
  char timestr[20];
  strftime(timestr, 20, "%Y-%m-%d %H:%M:%S", &timestruct);
  printf("Date: %s GMT\n", timestr);

  printf("Operating system: %s (%s)\n", system_info->os.c_str(),
         system_info->os_version.c_str());
  printf("Architecture: %s\n", cpu.c_str());

  if (process_state.crashed()) {
    printf("Crash reason:  %s\n", process_state.crash_reason().c_str());
    printf("Crash address: 0x%llx\n", process_state.crash_address());
  } else {
    printf("No crash\n");
  }

  int requesting_thread = process_state.requesting_thread();
  if (requesting_thread != -1) {
    printf("\n");
    printf("Thread %d (%s)\n",
           requesting_thread,
           process_state.crashed() ? "crashed" :
           "requested dump, did not crash");
    PrintStack(process_state.threads()->at(requesting_thread), cpu);
  }

  // Print all of the threads in the dump.
  int thread_count = process_state.threads()->size();
  for (int thread_index = 0; thread_index < thread_count; ++thread_index) {
    if (thread_index != requesting_thread) {
      // Don't print the crash thread again, it was already printed.
      printf("\n");
      printf("Thread %d\n", thread_index);
      PrintStack(process_state.threads()->at(thread_index), cpu);
    }
  }

  // Print the crashed registers
  if (requesting_thread != -1) {
    printf("\nThread %d:", requesting_thread);
    PrintRegisters(process_state.threads()->at(requesting_thread), cpu);
  }
}

//=============================================================================
static void Usage(int argc, const char *argv[]) {
  fprintf(stderr, "Convert a minidump to a crash report.  Breakpad symbol files\n");
  fprintf(stderr, "will be used (or created if missing) in /tmp.\n");
  fprintf(stderr, "Usage: %s [-s search-dir] minidump-file\n", argv[0]);
  fprintf(stderr, "\t-s: Specify a search directory to use for missing modules\n");
  fprintf(stderr, "\t-h: Usage\n");
  fprintf(stderr, "\t-?: Usage\n");
}

//=============================================================================
static void SetupOptions(int argc, const char *argv[], Options *options) {
  extern int optind;
  char ch;

  while ((ch = getopt(argc, (char * const *)argv, "s:h?")) != -1) {
    switch (ch) {
      case 's':
        options->searchDir = [[NSFileManager defaultManager]
          stringWithFileSystemRepresentation:optarg
                                      length:strlen(optarg)];
        break;

      case 'h':
      case '?':
        Usage(argc, argv);
        exit(1);
        break;
    }
  }

  if ((argc - optind) != 1) {
    fprintf(stderr, "%s: Missing minidump file\n", argv[0]);
    Usage(argc, argv);
    exit(1);
  }

  options->minidumpPath = [[NSFileManager defaultManager]
    stringWithFileSystemRepresentation:argv[optind]
                                length:strlen(argv[optind])];
}

//=============================================================================
int main (int argc, const char * argv[]) {
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  Options options;

  bzero(&options, sizeof(Options));
  SetupOptions(argc, argv, &options);
  Start(&options);
  [pool release];

  return 0;
}
