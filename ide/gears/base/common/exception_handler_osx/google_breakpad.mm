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

#define VERBOSE 0

#if VERBOSE
  static bool gDebugLog = true;
#else
  static bool gDebugLog = false;
#endif

#define DEBUGLOG if (gDebugLog) fprintf
#define IGNORE_DEBUGGER "GOOGLE_BREAKPAD_IGNORE_DEBUGGER"

#import "gears/base/common/exception_handler_osx/google_breakpad.h"

#import "gears/base/common/exception_handler_osx/scoped_releaser.h"
#import "gears/base/common/exception_handler_osx/inspector.h"
#import "gears/base/common/exception_handler_osx/mach_ipc.h"
#import "gears/base/common/exception_handler_osx/on_demand_server.h"
#import "third_party/breakpad_osx/src/client/mac/handler/protected_memory_allocator.h"
#import "gears/base/common/exception_handler_osx/simple_string_dictionary.h"

#import <sys/stat.h>
#import <sys/sysctl.h>

#import <Foundation/Foundation.h>

#import "third_party/breakpad_osx/src/client/mac/handler/exception_handler.h"
#import "third_party/breakpad_osx/src/common/mac/string_utilities.h"

using google_breakpad::KeyValueEntry;
using google_breakpad::SimpleStringDictionary;
using google_breakpad::SimpleStringDictionaryIterator;

//=============================================================================
// We want any memory allocations which are used by breakpad during the
// exception handling process (after a crash has happened) to be read-only
// to prevent them from being smashed before a crash occurs.  Unfortunately
// we cannot protect against smashes to our exception handling thread's
// stack...
//
// NOTE: Any memory allocations which are not used during the exception
// handling process may be allocated in the normal ways.
//
// The ProtectedMemoryAllocator class provides an Allocate() method which
// we'll using in conjunction with placement operator new() to control
// allocation of C++ objects.  Note that we don't use operator delete()
// but instead call the objects destructor directly:  object->~ClassName();
//
ProtectedMemoryAllocator *gMasterAllocator = NULL;
ProtectedMemoryAllocator *gKeyValueAllocator = NULL;
ProtectedMemoryAllocator *gBreakpadAllocator = NULL;

// Mutex for thread-safe access to the key/value dictionary used by breakpad.
// It's a global instead of an instance variable of GoogleBreakpad
// since it can't live in a protected memory area.
pthread_mutex_t gDictionaryMutex;

//=============================================================================
// Stack-based object for thread-safe access to a memory-protected region.
// It's assumed that normally the memory block (allocated by the allocator)
// is protected (read-only).  Creating a stack-based instance of
// ProtectedMemoryLocker will unprotect this block after taking the lock.
// Its destructor will first re-protect the memory then release the lock.
class ProtectedMemoryLocker {
public:
  // allocator may be NULL, in which case no Protect() or Unprotect() calls
  // will be made, but a lock will still be taken
  ProtectedMemoryLocker(pthread_mutex_t *mutex,
                        ProtectedMemoryAllocator *allocator)
  : mutex_(mutex), allocator_(allocator) {
    // Lock the mutex
    assert(pthread_mutex_lock(mutex_) == 0);
    
    // Unprotect the memory
    if (allocator_ ) {
      allocator_->Unprotect();
    }
  }
  
  ~ProtectedMemoryLocker() {
    // First protect the memory
    if (allocator_) {
      allocator_->Protect();
    }
    
    // Then unlock the mutex
    assert(pthread_mutex_unlock(mutex_) == 0);
  };
  
private:
  //  Keep anybody from ever creating one of these things not on the stack.
  ProtectedMemoryLocker() { }
  ProtectedMemoryLocker(const ProtectedMemoryLocker&);
  ProtectedMemoryLocker & operator=(ProtectedMemoryLocker&);
  
  pthread_mutex_t           *mutex_;
  ProtectedMemoryAllocator  *allocator_;
};

//=============================================================================
class GoogleBreakpad {
 public:
  // factory method
  static GoogleBreakpad *Create(NSDictionary *parameters) {
    // Allocate from our special allocation pool
    GoogleBreakpad *breakpad =
      new (gBreakpadAllocator->Allocate(sizeof(GoogleBreakpad)))
        GoogleBreakpad();

    if (!breakpad)
      return NULL;

    if (!breakpad->Initialize(parameters)) {
      // Don't use operator delete() here since we allocated from special pool
      breakpad->~GoogleBreakpad();
      return NULL;
    }

    return breakpad;
  }

  ~GoogleBreakpad();

  void SetKeyValue(NSString *key, NSString *value);
  NSString *KeyValue(NSString *key);
  void RemoveKeyValue(NSString *key);

  void GenerateAndSendReport();

 private:
  GoogleBreakpad()
    : handler_(NULL),
      config_params_(NULL),
      send_and_exit_(true) {
    inspector_path_[0] = 0;
  }

  bool Initialize(NSDictionary *parameters);

  bool ExtractParameters(NSDictionary *parameters);

  // Dispatches to HandleException()
  static bool ExceptionHandlerDirectCallback(void *context,
                                             int exception_type,
                                             int exception_code,
                                             mach_port_t crashing_thread);

  bool HandleException(int exception_type,
                       int exception_code,
                       mach_port_t crashing_thread);

  // Since ExceptionHandler (w/o namespace) is defined as typedef in OSX's
  // MachineExceptions.h, we have to explicitly name the handler.
  google_breakpad::ExceptionHandler *handler_; // The actual handler (STRONG)

  char                    inspector_path_[PATH_MAX];  // Path to inspector tool

  SimpleStringDictionary  *config_params_; // Create parameters (STRONG)

  OnDemandServer          inspector_;

  bool                    send_and_exit_;  // Exit after sending, if true
};

#pragma mark -
#pragma mark Helper functions

//=============================================================================
// Helper functions

//=============================================================================
static BOOL IsDebuggerActive() {
  BOOL result = NO;
  NSUserDefaults *stdDefaults = [NSUserDefaults standardUserDefaults];
  
  // We check both defaults and the environment variable here

  BOOL ignoreDebugger = [stdDefaults boolForKey:@IGNORE_DEBUGGER];
  
  if (!ignoreDebugger) {
    char *ignoreDebuggerStr = getenv(IGNORE_DEBUGGER);
    ignoreDebugger = (ignoreDebuggerStr ? strtol(ignoreDebuggerStr, NULL, 10) : 0) != 0;
  }

  if (!ignoreDebugger) {
    pid_t pid = getpid();
    int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_PID, pid};
    int mibSize = sizeof(mib) / sizeof(int);
    size_t actualSize;

    if (sysctl(mib, mibSize, NULL, &actualSize, NULL, 0) == 0) {
      struct kinfo_proc *info = (struct kinfo_proc *)malloc(actualSize);

      if (info) {
        // This comes from looking at the Darwin xnu Kernel
        if (sysctl(mib, mibSize, info, &actualSize, NULL, 0) == 0)
          result = (info->kp_proc.p_flag & P_TRACED) ? YES : NO;

        free(info);
      }
    }
  }

  return result;
}

//=============================================================================
bool GoogleBreakpad::ExceptionHandlerDirectCallback(void *context,
                                                    int exception_type,
                                                    int exception_code,
                                                    mach_port_t crashing_thread) {
  GoogleBreakpad *breakpad = (GoogleBreakpad *)context;

  // If our context is damaged or something, just return false to indicate that
  // the handler should continue without us.
  if (!breakpad)
    return false;

  return breakpad->HandleException( exception_type,
                                    exception_code,
                                    crashing_thread);
}

//=============================================================================
#pragma mark -

#import <mach-o/dyld.h>

//=============================================================================
// Returns the pathname to the Resources directory for this version of
// Breakpad which we are now running.
//
// Don't make the function static, since _dyld_lookup_and_bind_fully needs a
// simple non-static C name
//
extern "C" {
NSString * GetResourcePath();
NSString * GetResourcePath() {
  NSString *resourcePath = nil;
  
  // If there are multiple breakpads installed then calling bundleWithIdentifier
  // will not work properly, so only use that as a backup plan.
  // We want to find the bundle containing the code where this function lives
  // and work from there
  //

  // Get the pathname to the code which contains this function
  void *address = nil;
  NSModule module = nil;
  _dyld_lookup_and_bind_fully("_GetResourcePath",
                              &address,
                              &module);
  //printf("TestFunction: address = %p : module = %p\n", address, module);

  if (module && address) {
    const char* moduleName = NSNameOfModule(module);
    if (moduleName) {
      // The "Resources" directory should be in the same directory as the
	  // executable code, since that's how the Breakpad framework is built
      resourcePath = [NSString stringWithUTF8String:moduleName];
      resourcePath = [resourcePath stringByDeletingLastPathComponent]; 
      resourcePath = [resourcePath stringByAppendingPathComponent:@"Resources/"]; 
     } else {
      DEBUGLOG(stderr, "Missing moduleName\n");
    }
  } else {
    DEBUGLOG(stderr, "Could not find GetResourcePath\n");
    // fallback plan
    NSBundle *bundle =
      [NSBundle bundleWithIdentifier:@"com.Google.BreakpadFramework"];
    resourcePath = [bundle resourcePath];
  }

  return resourcePath;
}
}  // extern "C"

//=============================================================================
bool GoogleBreakpad::Initialize(NSDictionary *parameters) {
  // Initialize
  config_params_ = NULL;
  handler_ = NULL;
  
  // Check for debugger
  if (IsDebuggerActive()) {
    DEBUGLOG(stderr, "Debugger is active:  Not installing handler\n");
    return true;
  }

  // Gather any user specified parameters
  if (!ExtractParameters(parameters)) {
    return false;
  }
  
  // Get path to Inspector executable.
  NSString *inspectorPathString = KeyValue(@GOOGLE_BREAKPAD_INSPECTOR_LOCATION);
  
  // Standardize path (resolve symlinkes, etc.)  and escape spaces
  inspectorPathString = [inspectorPathString stringByStandardizingPath];
  inspectorPathString = [[inspectorPathString componentsSeparatedByString:@" "]
                                              componentsJoinedByString:@"\\ "];

  // Create an on-demand server object representing the Inspector.
  // In case of a crash, we simply need to call the LaunchOnDemand()
  // method on it, then send a mach message to its service port.
  // It will then launch and perform a process inspection of our crashed state.
  // See the HandleException() method for the details.
#define RECEIVE_PORT_NAME "com.Google.BreakpadInspector"

  name_t portName;
  snprintf(portName, sizeof(name_t),  "%s%d", RECEIVE_PORT_NAME, getpid());

  // Save the location of the Inspector
  strlcpy(inspector_path_, [inspectorPathString fileSystemRepresentation],
          sizeof(inspector_path_));

  // Append a single command-line argument to the Inspector path
  // representing the bootstrap name of the launch-on-demand receive port.
  // When the Inspector is launched, it can use this to lookup the port
  // by calling bootstrap_check_in().
  strlcat(inspector_path_, " ", sizeof(inspector_path_));
  strlcat(inspector_path_, portName, sizeof(inspector_path_));

  kern_return_t kr = inspector_.Initialize(inspector_path_,
                                           portName,
                                           true);        // shutdown on exit
  
  if (kr != KERN_SUCCESS) {
    return false;
  }

  // Create the handler (allocating it in our special protected pool)
  handler_ =
    new (gBreakpadAllocator->Allocate(sizeof(google_breakpad::ExceptionHandler)))
      google_breakpad::ExceptionHandler(
        GoogleBreakpad::ExceptionHandlerDirectCallback, this, true);

  return true;
}

//=============================================================================
GoogleBreakpad::~GoogleBreakpad() {
  // Note that we don't use operator delete() on these pointers,
  // since they were allocated by ProtectedMemoryAllocator objects.
  //
  if (config_params_) {
    config_params_->~SimpleStringDictionary();
  }

  if (handler_)
    handler_->~ExceptionHandler();
}

//=============================================================================
bool GoogleBreakpad::ExtractParameters(NSDictionary *parameters) {
  NSUserDefaults *stdDefaults = [NSUserDefaults standardUserDefaults];
  NSString *display = [parameters objectForKey:@GOOGLE_BREAKPAD_PRODUCT_DISPLAY];
  NSString *product = [parameters objectForKey:@GOOGLE_BREAKPAD_PRODUCT];
  NSString *version = [parameters objectForKey:@GOOGLE_BREAKPAD_VERSION];
  NSString *urlStr = [parameters objectForKey:@GOOGLE_BREAKPAD_URL];
  NSString *interval = [parameters objectForKey:@GOOGLE_BREAKPAD_REPORT_INTERVAL];
  NSString *inspectorPathString = 
                [parameters objectForKey:@GOOGLE_BREAKPAD_INSPECTOR_LOCATION];
  NSString *reporterPathString = 
                [parameters objectForKey:@GOOGLE_BREAKPAD_REPORTER_EXE_LOCATION];

  // skipConfirm and sendAndExit can be overridden by user defaults.
  NSString *skipConfirm = [stdDefaults stringForKey:@GOOGLE_BREAKPAD_SKIP_CONFIRM];
  if (!skipConfirm) {
    skipConfirm = [parameters objectForKey:@GOOGLE_BREAKPAD_SKIP_CONFIRM];
  }
  NSString *sendAndExit = [stdDefaults stringForKey:@GOOGLE_BREAKPAD_SEND_AND_EXIT];
  if (!sendAndExit) {
    sendAndExit = [parameters objectForKey:@GOOGLE_BREAKPAD_SEND_AND_EXIT];
  }

  if (!product)
    product = [parameters objectForKey:@"CFBundleName"];

  if (!display)
    display = product;

  if (!version)
    version = [parameters objectForKey:@"CFBundleVersion"];

  if (!urlStr)
    urlStr = @"https://www.google.com/cr/report";

  if (!interval)
    interval = @"3600";
  
  // Normalize the values
  if (skipConfirm) {
    skipConfirm = [skipConfirm uppercaseString];

    if ([skipConfirm isEqualToString:@"YES"] ||
        [skipConfirm isEqualToString:@"TRUE"] ||
        [skipConfirm isEqualToString:@"1"])
      skipConfirm = @"YES";
    else
      skipConfirm = @"NO";
  } else {
    skipConfirm = @"NO";
  }

  send_and_exit_ = true;
  if (sendAndExit) {
    sendAndExit = [sendAndExit uppercaseString];

    if ([sendAndExit isEqualToString:@"NO"] ||
        [sendAndExit isEqualToString:@"FALSE"] ||
        [sendAndExit isEqualToString:@"0"])
      send_and_exit_ = false;
  }
  
  // Find the helper applications if not specified in user config.
  NSString *resourcePath = nil;
  if (!inspectorPathString || !reporterPathString) {
    resourcePath = GetResourcePath();
    if (!resourcePath) {
      DEBUGLOG(stderr, "Could not get resource path\n");
      return false;
    }
  }
  
  // Find Inspector.
  if (!inspectorPathString) {
    inspectorPathString = 
        [resourcePath stringByAppendingPathComponent:@"Inspector"];
  }
  
  // Verify that there is an Inspector tool
  if (![[NSFileManager defaultManager] fileExistsAtPath:inspectorPathString]) {
    DEBUGLOG(stderr, "Cannot find Inspector tool\n");
    return false;
  }
  
  // Find Reporter.
  if (!reporterPathString) {
    reporterPathString =
      [resourcePath stringByAppendingPathComponent:@"Reporter.app"];
    reporterPathString = [[NSBundle bundleWithPath:reporterPathString] 
                              executablePath];
  }

  // Verify that there is a Reporter application
  if (![[NSFileManager defaultManager] 
             fileExistsAtPath:reporterPathString]) {
    DEBUGLOG(stderr, "Cannot find Reporter tool\n");
    return false;
  }

  // The product and version are required values
  if (![product length] || ![version length]) {
    DEBUGLOG(stderr, "Missing required product and/or version keys\n");
    return false;
  }

  config_params_ =
      new (gKeyValueAllocator->Allocate(sizeof(SimpleStringDictionary)) )
        SimpleStringDictionary();

  SimpleStringDictionary &dictionary = *config_params_;

  dictionary.SetKeyValue(GOOGLE_BREAKPAD_PRODUCT_DISPLAY, [display UTF8String]);
  dictionary.SetKeyValue(GOOGLE_BREAKPAD_PRODUCT,         [product UTF8String]);
  dictionary.SetKeyValue(GOOGLE_BREAKPAD_VERSION,         [version UTF8String]);
  dictionary.SetKeyValue(GOOGLE_BREAKPAD_URL,             [urlStr UTF8String]);
  dictionary.SetKeyValue(GOOGLE_BREAKPAD_REPORT_INTERVAL, [interval UTF8String]);
  dictionary.SetKeyValue(GOOGLE_BREAKPAD_SKIP_CONFIRM,    [skipConfirm UTF8String]);
  dictionary.SetKeyValue(GOOGLE_BREAKPAD_INSPECTOR_LOCATION, 
                         [inspectorPathString fileSystemRepresentation]);
    dictionary.SetKeyValue(GOOGLE_BREAKPAD_REPORTER_EXE_LOCATION, 
                           [reporterPathString fileSystemRepresentation]);

#if 0 // for testing
  GoogleBreakpadSetKeyValue(this, @"UserKey1", @"User Value 1");
  GoogleBreakpadSetKeyValue(this, @"UserKey2", @"User Value 2");
  GoogleBreakpadSetKeyValue(this, @"UserKey3", @"User Value 3");
  GoogleBreakpadSetKeyValue(this, @"UserKey4", @"User Value 4");
#endif

  return true;
}

//=============================================================================
void        GoogleBreakpad::SetKeyValue(NSString *key, NSString *value) {
  // We allow nil values. This is the same as removing the keyvalue.
  if (!config_params_ || !key)
    return;

  config_params_->SetKeyValue([key UTF8String], [value UTF8String]);
}

//=============================================================================
NSString *  GoogleBreakpad::KeyValue(NSString *key) {
  if (!config_params_ || !key)
    return nil;
  
  const char *value = config_params_->GetValueForKey([key UTF8String]);
  return value ? [NSString stringWithUTF8String:value] : nil;
}

//=============================================================================
void        GoogleBreakpad::RemoveKeyValue(NSString *key) {
  if (!config_params_ || !key)
    return;

  config_params_->RemoveKey([key UTF8String]);
}

//=============================================================================
void        GoogleBreakpad::GenerateAndSendReport() {
  HandleException(0, 0, 0);
}

//=============================================================================
bool GoogleBreakpad::HandleException( int           exception_type,
                                      int           exception_code,
                                      mach_port_t   crashing_thread) {
  // commit the server to being launched when we mach message it
  inspector_.LaunchOnDemand();

  // The Inspector should send a message to this port to verify it
  // received our information and has finished the inspection.
  ReceivePort acknowledge_port;

  // Send initial information to the Inspector.
  MachSendMessage message(kMsgType_InspectorInitialInfo);
  message.AddDescriptor(mach_task_self());          // our task
  message.AddDescriptor(crashing_thread);           // crashing thread
  message.AddDescriptor(mach_thread_self());        // exception-handling thread
  message.AddDescriptor(acknowledge_port.GetPort());// message receive port

  InspectorInfo info;
  info.exception_type = exception_type;
  info.exception_code = exception_code;
  info.parameter_count = config_params_->GetCount();
  message.SetData(&info, sizeof(info));

  MachPortSender sender(inspector_.GetServicePort());

  kern_return_t result = sender.SendMessage(message, 2000);

  if (result == KERN_SUCCESS) {
    // Now, send a series of key-value pairs to the Inspector.
    const KeyValueEntry *entry = NULL;
    SimpleStringDictionaryIterator iter(*config_params_);

    while ( (entry = iter.Next()) ) {
      KeyValueMessageData keyvalue_data(*entry);

      MachSendMessage keyvalue_message(kMsgType_InspectorKeyValuePair);
      keyvalue_message.SetData(&keyvalue_data, sizeof(keyvalue_data));

      result = sender.SendMessage(keyvalue_message, 2000);

      if (result != KERN_SUCCESS) {
        break;
      }
    }

    if (result == KERN_SUCCESS) {
      // Wait for acknowledgement that the inspection has finished.
      MachReceiveMessage acknowledge_messsage;
      result = acknowledge_port.WaitForMessage(&acknowledge_messsage, 2000);
    }
  }

#if VERBOSE
  PRINT_MACH_RESULT(result, "GoogleBreakpad: SendMessage ");
  printf("GoogleBreakpad: Inspector service port = %#x\n",
    inspector_.GetServicePort());
#endif

  // If we don't want any forwarding, return true here to indicate that we've
  // processed things as much as we want.
  if (send_and_exit_)
    return true;

  return false;
}

//=============================================================================
//=============================================================================

#pragma mark -
#pragma mark Public API

//=============================================================================
GoogleBreakpadRef GoogleBreakpadCreate(NSDictionary *parameters) {  
    // Gears; this code originally used try/catch for error handling, here
    // we've ported it to use the std::nothrow variant of new.  Placement new
    // won't throw unless we pass it a NULL ptr, which we explicitly check for.
    // If new throws an exception when compiled with exceptions turned off, then
    // the application terminates.
    
    // This is confusing.  Our two main allocators for breakpad memory are:
    //    - gKeyValueAllocator for the key/value memory
    //    - gBreakpadAllocator for the GoogleBreakpad, ExceptionHandler, and other
    //      breakpad allocations which are accessed at exception handling time.
    //
    // But in order to avoid these two allocators themselves from being smashed,
    // we'll protect them as well by allocating them with gMasterAllocator.
    //
    // gMasterAllocator itself will NOT be protected, but this doesn't matter,
    // since once it does its allocations and locks the memory, smashes to itself
    // don't affect anything we care about.
    // With the current compiler, gBreakpadAllocator is allocating 1444 bytes.
    // Let's round up to the nearest page size.
    //
    int breakpad_pool_size = 4096;
    
    gMasterAllocator =
      new (std::nothrow) 
        ProtectedMemoryAllocator(sizeof(ProtectedMemoryAllocator) * 2);
    if (!gMasterAllocator) goto cleanup_google_breakpad_create;
    
    void *tmp_gKeyValueAllocator = 
        gMasterAllocator->Allocate(sizeof(ProtectedMemoryAllocator));
    if (!tmp_gKeyValueAllocator) goto cleanup_google_breakpad_create;
    gKeyValueAllocator =
      new (tmp_gKeyValueAllocator)
        ProtectedMemoryAllocator(sizeof(SimpleStringDictionary));
    
    // Create a mutex for use in accessing the SimpleStringDictionary
    int mutexResult = pthread_mutex_init(&gDictionaryMutex, NULL);
    if (mutexResult != 0) {
      goto cleanup_google_breakpad_create;
    }
    
    /*
     sizeof(GoogleBreakpad)
     + sizeof(google_breakpad::ExceptionHandler)
     + sizeof( STUFF ALLOCATED INSIDE ExceptionHandler )
     */
    
    void *tmp_gBreakpadAllocator = 
      gMasterAllocator->Allocate(sizeof(ProtectedMemoryAllocator));
    if (!tmp_gBreakpadAllocator) goto cleanup_google_breakpad_create;
    gBreakpadAllocator =
      new (tmp_gBreakpadAllocator)
        ProtectedMemoryAllocator(breakpad_pool_size);
    
    // Stack-based autorelease pool for GoogleBreakpad::Create() obj-c code.
    {
      GMAutoreleasePool pool;
      GoogleBreakpad *breakpad = GoogleBreakpad::Create(parameters);
      
      if (breakpad) {
        // Make read-only to protect against memory smashers
        gMasterAllocator->Protect();
        gKeyValueAllocator->Protect();
        gBreakpadAllocator->Protect();
      } else {
        goto cleanup_google_breakpad_create;
      }

      // Can uncomment this line to figure out how much space was actually
      // allocated using this allocator
      //
      
      // printf("gBreakpadAllocator allocated size = %d\n", gBreakpadAllocator->GetAllocatedSize() );
      
      return (GoogleBreakpadRef)breakpad;
    }
    
cleanup_google_breakpad_create:
  if (gKeyValueAllocator) {
    gKeyValueAllocator->~ProtectedMemoryAllocator();
    gKeyValueAllocator = NULL;
  }
  
  if (gBreakpadAllocator) {
    gBreakpadAllocator->~ProtectedMemoryAllocator();
    gBreakpadAllocator = NULL;
  }
  
  operator delete (gMasterAllocator, std::nothrow);
  gMasterAllocator = NULL;
  
  return NULL;
}

//=============================================================================
void GoogleBreakpadRelease(GoogleBreakpadRef ref) {
#if 0
  try {
#endif
    GoogleBreakpad *breakpad = (GoogleBreakpad *)ref;

    if (gMasterAllocator) {
      gMasterAllocator->Unprotect();
      gKeyValueAllocator->Unprotect();
      gBreakpadAllocator->Unprotect();
      
      breakpad->~GoogleBreakpad();

      // Unfortunately, it's not possible to deallocate this stuff
      // because the exception handling thread is still finishing up
      // asynchronously at this point...  OK, it could be done with locks, etc.
      // But since GoogleBreakpadRelease() should usually only be called
      // right before the process exits, it's not worth deallocating this stuff.
#if 0 
      gKeyValueAllocator->~ProtectedMemoryAllocator();
      gBreakpadAllocator->~ProtectedMemoryAllocator();
      delete gMasterAllocator;
      
      gMasterAllocator = NULL;
      gKeyValueAllocator = NULL;
      gBreakpadAllocator = NULL;
#endif
      
      pthread_mutex_destroy(&gDictionaryMutex);
    }
#if 0
  } catch(...) {    // don't let exception leave this C API
    fprintf(stderr, "GoogleBreakpadRelease() : error\n");
  }
#endif
}

//=============================================================================
void GoogleBreakpadSetKeyValue(GoogleBreakpadRef ref, NSString *key, NSString *value) {
#if 0
  try {
#endif
    // Not called at exception time
    GoogleBreakpad *breakpad = (GoogleBreakpad *)ref;

    if (breakpad && key && gKeyValueAllocator) {
      ProtectedMemoryLocker locker(&gDictionaryMutex, gKeyValueAllocator);

      breakpad->SetKeyValue(key, value);
    }
#if 0
  } catch(...) {    // don't let exception leave this C API
    fprintf(stderr, "GoogleBreakpadSetKeyValue() : error\n");
  }
#endif
}

//=============================================================================
NSString *GoogleBreakpadKeyValue(GoogleBreakpadRef ref, NSString *key) {
  NSString *value = nil;
#if 0  
  try {
#endif
    // Not called at exception time
    GoogleBreakpad *breakpad = (GoogleBreakpad *)ref;

    if (!breakpad || !key || !gKeyValueAllocator)
      return nil;

    ProtectedMemoryLocker locker(&gDictionaryMutex, gKeyValueAllocator);

    value = breakpad->KeyValue(key);
#if 0
  } catch(...) {    // don't let exception leave this C API
    fprintf(stderr, "GoogleBreakpadKeyValue() : error\n");
  }
#endif

  return value;
}

//=============================================================================
void GoogleBreakpadRemoveKeyValue(GoogleBreakpadRef ref, NSString *key) {
#if 0
  try {
#endif
    // Not called at exception time
    GoogleBreakpad *breakpad = (GoogleBreakpad *)ref;

    if (breakpad && key && gKeyValueAllocator) {
      ProtectedMemoryLocker locker(&gDictionaryMutex, gKeyValueAllocator);

      breakpad->RemoveKeyValue(key);
    }
#if 0
  } catch(...) {    // don't let exception leave this C API
    fprintf(stderr, "GoogleBreakpadRemoveKeyValue() : error\n");
  }
#endif
}

//=============================================================================
void GoogleBreakpadGenerateAndSendReport(GoogleBreakpadRef ref) {
#if 0
  try {
#endif
    GoogleBreakpad *breakpad = (GoogleBreakpad *)ref;

    if (breakpad && gKeyValueAllocator) {
      ProtectedMemoryLocker locker(&gDictionaryMutex, gKeyValueAllocator);

      breakpad->GenerateAndSendReport();
    }
#if 0
  } catch(...) {    // don't let exception leave this C API
    fprintf(stderr, "GoogleBreakpadGenerateAndSendReport() : error\n");
  }
#endif
}
