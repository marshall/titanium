// Copyright 2007, Google Inc.
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

#import <cstdio>
#import <iostream>
#import <stdio.h>
#import <string.h>
#import <string>
#import <sys/syslimits.h>

#import "gears/base/common//exception_handler_osx//inspector.h"

#import "gears/base/common/exception_handler_osx/google_breakpad.h"
#import "gears/base/common/exception_handler_osx/mach_ipc.h"
#import "third_party/breakpad_osx/src/client/mac/handler/minidump_generator.h"
#import "gears/base/common/exception_handler_osx/simple_string_dictionary.h"

#import <Foundation/Foundation.h>

using std::string;
using google_breakpad::MinidumpGenerator;

namespace google_breakpad {

//=============================================================================
static BOOL EnsureDirectoryPathExists(NSString *dirPath) {
  NSFileManager *mgr = [NSFileManager defaultManager];

  // If we got a relative path, prepend the current directory
  if (![dirPath isAbsolutePath])
    dirPath = [[mgr currentDirectoryPath] stringByAppendingPathComponent:dirPath];

  NSString *path = dirPath;

  // Ensure that no file exists within the path which would block creation
  while (1) {
    BOOL isDir;
    if ([mgr fileExistsAtPath:path isDirectory:&isDir]) {
      if (isDir)
        break;

      return NO;
    }

    path = [path stringByDeletingLastPathComponent];
  }

  // Path now contains the first valid directory (or is empty)
  if (![path length])
    return NO;

  NSString *common =
    [dirPath commonPrefixWithString:path options:NSLiteralSearch];

  // If everything is good
  if ([common isEqualToString:dirPath])
    return YES;

  // Break up the difference into components
  NSString *diff = [dirPath substringFromIndex:[common length] + 1];
  NSArray *components = [diff pathComponents];
  unsigned count = [components count];

  // Rebuild the path one component at a time
  NSDictionary *attrs =
    [NSDictionary dictionaryWithObject:[NSNumber numberWithUnsignedLong:0750]
                                forKey:NSFilePosixPermissions];
  path = common;
  for (unsigned i = 0; i < count; ++i) {
    path = [path stringByAppendingPathComponent:[components objectAtIndex:i]];

    if (![mgr createDirectoryAtPath:path attributes:attrs])
      return NO;
  }

  return YES;
}

//=============================================================================
class MinidumpLocation {
 public:
  MinidumpLocation() {
    NSString *minidumpDirBase = NSHomeDirectory();
    NSString *minidumpDir;
    
    // Put root processes at root
    if (geteuid() == 0)
      minidumpDirBase = @"/";

    minidumpDir =
      [minidumpDirBase stringByAppendingPathComponent:@"Library/Logs/Google"];

    // Ensure that the path exists.  Fallback to /tmp if unable to locate path.
    if (!EnsureDirectoryPathExists(minidumpDir)) {
      DEBUGLOG(stderr, "Unable to create: %s\n", [minidumpDir UTF8String]);
      minidumpDir = @"/tmp";
    }
    
    strlcpy(minidump_dir_path_, [minidumpDir fileSystemRepresentation], 
            sizeof(minidump_dir_path_));

    // now generate a unique ID
    string dump_path(minidump_dir_path_);  
    string next_minidump_id;
    
    string next_minidump_path_ =
      (MinidumpGenerator::UniqueNameInDirectory(dump_path, &next_minidump_id));

    strlcpy(minidump_id_, next_minidump_id.c_str(), sizeof(minidump_id_));
  };

  const char *GetPath() { return minidump_dir_path_; }
  const char *GetID() { return minidump_id_; }

 private:
  char minidump_dir_path_[PATH_MAX];             // Path to minidump directory
  char minidump_id_[128];
};

//=============================================================================
class ConfigFile {
 public:
  ConfigFile() {
    config_file_ = -1;
    config_file_path_[0] = 0;
    has_created_file_ = false;
  };
  
  ~ConfigFile() {
  };
  
  void WriteFile(const SimpleStringDictionary *configurationParameters,
                 const char *dump_dir,
                 const char *minidump_id);

  const char *GetFilePath() { return config_file_path_; }
  
  void Unlink() {
    if (config_file_ != -1)
      unlink(config_file_path_);
      
    config_file_ = -1;
  }

 private:
  BOOL WriteData(const void *data, size_t length);

  BOOL AppendConfigData(const char *key,
                        const void *data,
                        size_t length);

  BOOL AppendConfigString(const char *key,
                          const char *value);

  int   config_file_;                    // descriptor for config file
  char  config_file_path_[PATH_MAX];     // Path to configuration file
  bool  has_created_file_;
};

//=============================================================================
#pragma mark -

//=============================================================================
BOOL ConfigFile::WriteData(const void *data, size_t length) {
  size_t result = write(config_file_, data, length);

  return result == length;
}

//=============================================================================
BOOL ConfigFile::AppendConfigData(const char *key,
                                  const void *data, size_t length) {
  assert(config_file_ != -1);

  if (!key) {
    DEBUGLOG(stderr, "GoogleBreakpad: Missing Key\n");
    return NO;
  }

  if (!data) {
    DEBUGLOG(stderr, "GoogleBreakpad: Missing data for key: %s\n", key ? key :
            "<Unknown Key>");
    return NO;
  }

  // Write the key, \n, length of data (ascii integer), \n, data
  char buffer[16];
  char nl = '\n';
  BOOL result = WriteData(key, strlen(key));

  snprintf(buffer, sizeof(buffer) - 1, "\n%lu\n", length);
  result &= WriteData(buffer, strlen(buffer));
  result &= WriteData(data, length);
  result &= WriteData(&nl, 1);
  return result;
}

//=============================================================================
BOOL ConfigFile::AppendConfigString(const char *key,
                                    const char *value) {
  return AppendConfigData(key, value, strlen(value));
}

//=============================================================================
void ConfigFile::WriteFile(const SimpleStringDictionary *configurationParameters,
                           const char *dump_dir,
                           const char *minidump_id) {

  assert(config_file_ == -1);
  
  // Open and write out configuration file preamble
  strlcpy(config_file_path_, "/tmp/Config-XXXXXX",
          sizeof(config_file_path_));
  config_file_ = mkstemp(config_file_path_);
    
  if (config_file_ == -1)
    return;

  has_created_file_ = true;

  // Add the minidump dir
  AppendConfigString(kReporterMinidumpDirectoryKey, dump_dir);
  AppendConfigString(kReporterMinidumpIDKey, minidump_id);
  
  // Write out the configuration parameters
  BOOL result = YES;
  const SimpleStringDictionary &dictionary = *configurationParameters;
  
  const KeyValueEntry *entry = NULL;
  SimpleStringDictionaryIterator iter(dictionary);
  
  while ((entry = iter.Next())) {
    result = AppendConfigString(entry->GetKey(), entry->GetValue());
    
    if (!result)
      break;
  }
  
  close(config_file_);
  config_file_ = -1;
}

#pragma mark -

//=============================================================================
class Inspector {
 public:
  Inspector() {};

  // given a bootstrap service name, receives mach messages
  // from a crashed process, then inspects it, creates a minidump file
  // and asks the user if he wants to upload it to our Google crash2 server.
  void            Inspect(const char *receive_port_name);

 private:
  kern_return_t   ServiceCheckIn(const char *receive_port_name);
  kern_return_t   ServiceCheckOut(const char *receive_port_name);

  kern_return_t   ReadMessages();

  bool            InspectTask();
  kern_return_t   SendAcknowledgement();
  void            LaunchReporter(const char *inConfigFilePath);

  mach_port_t     service_rcv_port_;

  int             exception_type_;
  int             exception_code_;
  mach_port_t     remote_task_;
  mach_port_t     crashing_thread_;
  mach_port_t     handler_thread_;
  mach_port_t     ack_port_;

  SimpleStringDictionary config_params_;

  ConfigFile      config_file_;
};

//=============================================================================
void Inspector::Inspect(const char *receive_port_name) {
  kern_return_t result = ServiceCheckIn(receive_port_name);

  if (result == KERN_SUCCESS) {
    result = ReadMessages();    

    if (result == KERN_SUCCESS) {
      // Inspect the task and write a minidump file.
      InspectTask();
                  
      // Send acknowledgement to the crashed process that the inspection
      // has finished.  It will then be able to cleanly exit.
      if (SendAcknowledgement() == KERN_SUCCESS) {
        // Ask the user if he wants to upload the crash report to Google,
        // and do so if he agrees.
        LaunchReporter(config_file_.GetFilePath());
      }
      
      // Now that we're done reading messages, cleanup the service
      ServiceCheckOut(receive_port_name);
    } else {
        PRINT_MACH_RESULT(result, "Inspector: WaitForMessage()");
    }    
  }
}

//=============================================================================
kern_return_t Inspector::ServiceCheckIn(const char *receive_port_name) {
  // We need to get the mach port representing this service, so we can
  // get information from the crashed process.
  kern_return_t kr = bootstrap_check_in(bootstrap_port,
                                        (char*)receive_port_name,
                                        &service_rcv_port_);

  if (kr != KERN_SUCCESS) {
#if VERBOSE
    PRINT_MACH_RESULT(kr, "Inspector: bootstrap_check_in()");
#endif
  }
  
  return kr;
}

//=============================================================================
kern_return_t Inspector::ServiceCheckOut(const char *receive_port_name) {
  // We're done receiving mach messages from the crashed process,
  // so clean up a bit.
  kern_return_t kr;

  // DO NOT use mach_port_deallocate() here -- it will fail and the
  // following bootstrap_register() will also fail leaving our service
  // name hanging around forever (until reboot)
  kr = mach_port_destroy(mach_task_self(), service_rcv_port_);

  if (kr != KERN_SUCCESS) {
    PRINT_MACH_RESULT(kr,
      "Inspector: UNREGISTERING: service_rcv_port mach_port_deallocate()");
    return kr;
  }

  // Unregister the service associated with the receive port.
  kr = bootstrap_register(bootstrap_port,
                          (char*)receive_port_name,
                          MACH_PORT_NULL);

  if (kr != KERN_SUCCESS) {
    PRINT_MACH_RESULT(kr, "Inspector: UNREGISTERING: bootstrap_register()");
  }
  
  return kr;
}

//=============================================================================
kern_return_t Inspector::ReadMessages() {
  // Wait for an initial message from the crashed process containing basic
  // information about the crash.
  ReceivePort receive_port(service_rcv_port_);

  MachReceiveMessage message;
  kern_return_t result = receive_port.WaitForMessage(&message, 1000);
 
  if (result == KERN_SUCCESS) {
    InspectorInfo &info = (InspectorInfo &)*message.GetData();
    exception_type_ = info.exception_type;
    exception_code_ = info.exception_code;

#if VERBOSE
    printf("message ID = %d\n", message.GetMessageID());
#endif

    remote_task_ = message.GetTranslatedPort(0);
    crashing_thread_ = message.GetTranslatedPort(1);
    handler_thread_ = message.GetTranslatedPort(2);
    ack_port_ = message.GetTranslatedPort(3);

#if VERBOSE
    printf("exception_type = %d\n", exception_type_);
    printf("exception_code = %d\n", exception_code_);
    printf("remote_task = %d\n", remote_task_);
    printf("crashing_thread = %d\n", crashing_thread_);
    printf("handler_thread = %d\n", handler_thread_);
    printf("ack_port_ = %d\n", ack_port_);
    printf("parameter count = %d\n", info.parameter_count);
#endif

    // The initial message contains the number of key value pairs that
    // we are expected to read.
    // Read each key/value pair, one mach message per key/value pair.    
    for (unsigned int i = 0; i < info.parameter_count; ++i) {
      MachReceiveMessage message;
      result = receive_port.WaitForMessage(&message, 1000);
      
      if(result == KERN_SUCCESS) {
        KeyValueMessageData &key_value_data =
          (KeyValueMessageData&)*message.GetData();
        
        config_params_.SetKeyValue(key_value_data.key, key_value_data.value);
      } else {
        PRINT_MACH_RESULT(result, "Inspector: key/value message");
        break;
      }
    }
  }
  
  return result;
}

//=============================================================================
bool Inspector::InspectTask() {
  // keep the task quiet while we're looking at it
  task_suspend(remote_task_);
  
  MinidumpLocation  minidumpLocation;
  
  config_file_.WriteFile( &config_params_,
                          minidumpLocation.GetPath(),
                          minidumpLocation.GetID());

  
  MinidumpGenerator generator(remote_task_, handler_thread_);
  
  if (exception_type_ && exception_code_) {
    generator.SetExceptionInformation(exception_type_,
                                      exception_code_,
                                      crashing_thread_);
  }

  NSString *minidumpPath = [NSString stringWithFormat:@"%s/%s.dmp",
    minidumpLocation.GetPath(), minidumpLocation.GetID()];

  bool result = generator.Write([minidumpPath fileSystemRepresentation]);

  DEBUGLOG(stderr, "Inspector: finished writing minidump file: %s\n",
    [minidumpPath fileSystemRepresentation]);

  // let the task continue
  task_resume(remote_task_);
  
  return result;
}

//=============================================================================
// The crashed task needs to be told that the inspection has finished.
// It will wait on a mach port (with timeout) until we send acknowledgement.
kern_return_t Inspector::SendAcknowledgement() {
  if (ack_port_ != MACH_PORT_DEAD) {
    MachPortSender sender(ack_port_);
    MachSendMessage ack_message(kMsgType_InspectorAcknowledgement);

    DEBUGLOG(stderr, "Inspector: trying to send acknowledgement to port %d\n",
      ack_port_);

    kern_return_t result = sender.SendMessage(ack_message, 2000);

#if VERBOSE
    PRINT_MACH_RESULT(result, "Inspector: sent acknowledgement");
#endif

    return result;
  }

  DEBUGLOG(stderr, "Inspector: port translation failure!\n");
  return KERN_INVALID_NAME;
}

//=============================================================================
void Inspector::LaunchReporter(const char *inConfigFilePath) {
  // Extract the path to the reporter executable.
  const char *reporterExecutablePath =
          config_params_.GetValueForKey(GOOGLE_BREAKPAD_REPORTER_EXE_LOCATION);
  DEBUGLOG(stderr, "reporter path = %s\n", reporterExecutablePath);
  // Setup and launch our reporter
  const char *argv[3];
  argv[0] = reporterExecutablePath;
  argv[1] = inConfigFilePath;
  argv[2] = NULL;
  
  // Launch the reporter
  pid_t pid = fork();
  
  // If we're in the child, load in our new executable and run.
  // The parent will not wait for the child to complete...
  if (pid == 0) {
    execv(argv[0], (char * const *)argv);
    config_file_.Unlink();  // launch failed - get rid of config file
    DEBUGLOG(stderr, "Inspector: unable to launch reporter app\n");
    _exit(1);
  }
  
  // Wait until the Reporter child process exits...
  //
  
  // We'll use a timeout of one minute
  int timeoutCount = 60;   // 60 seconds
  
  while (timeoutCount-- > 0) {
    int status;
    pid_t result = waitpid(pid, &status, WNOHANG);

    if (result == 0) {
      // child has not yet finished
      sleep(1);  // sleep for one second
    } else if (result == -1) {
      DEBUGLOG(stderr, "Inspector: waitpid error (%d) waiting for reporter app\n",
        errno);
      break;
    } else {
      // child has finished
      break;
    }
  }
}

#pragma mark -

//=============================================================================
extern "C" {

int main(int argc, char *const argv[]) {
#if DEBUG
  // since we're launched on-demand, this is necessary to see debugging
  // output in the console window
  freopen("/dev/console", "w", stdout);
  freopen("/dev/console", "w", stderr);
  //freopen("/tmp/console1", "w", stdout);
  //freopen("/tmp/console2", "w", stderr);
#endif

  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

  DEBUGLOG(stderr, "Inspector has launched!\n");

  if (argc != 2) {
    exit(0);
  }
  
  // Our first command-line argument contains the name of the service
  // that we're providing.
  Inspector inspector;
  inspector.Inspect(argv[1]);  
  
  [pool release];

  return 0;
}

} // extern "C"
} // namespace google_breakpad

