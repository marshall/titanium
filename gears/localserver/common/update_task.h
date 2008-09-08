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

#ifndef GEARS_LOCALSERVER_COMMON_UPDATE_TASK_H__
#define GEARS_LOCALSERVER_COMMON_UPDATE_TASK_H__

#include <map>
#include "gears/base/common/common.h"
#include "gears/base/common/message_service.h"
#include "gears/base/common/mutex.h"
#include "gears/base/common/security_model.h"
#include "gears/base/common/serialization.h"
#include "gears/localserver/common/async_task.h"
#include "gears/localserver/common/managed_resource_store.h"

//------------------------------------------------------------------------------
// An UpdateTask checks for an update to a ManagedResourceStore, and if found
// downloads the updated files and stores them locally in the WebCacheDB.
// Upon completion of a successful update, the version that was downloaded
// will be in the VERSION_CURRENT ready state.
//------------------------------------------------------------------------------
class UpdateTask : public AsyncTask {
 public:
  // UpdateTasks broadcast notifications via the MessageService as they go.
  enum EventType {
    PROGRESS_EVENT = 0,
    COMPLETION_EVENT = 1,
    ERROR_EVENT = 2
  };

  // Returns the topic string that should be used to observe update task
  // notifications for a particular store. Each store uses a unique topic,
  // notification of all EventTypes for a particular store use the same topic.
  static std::string16 GetNotificationTopic(ManagedResourceStore *store);
  static void RegisterEventClasses();

  class Event : public NotificationData {
   public:
    Event(EventType event_type) : event_type_(event_type) {}
    EventType event_type() const { return event_type_; }
   private:
    const EventType event_type_;
  };

  // Raised once prior to downloading any files listed in the manifest.
  // Then raised every time an entry if the manifest file is completes.
  class ProgressEvent : public Event {
   public:
    ProgressEvent()
      : Event(PROGRESS_EVENT),
        files_total_(0), files_complete_(0) {}
    ProgressEvent(int files_total, int files_complete)
      : Event(PROGRESS_EVENT),
        files_total_(files_total), files_complete_(files_complete) {}
    int files_total() const { return files_total_; }
    int files_complete() const { return files_complete_; }

    virtual SerializableClassId GetSerializableClassId() const {
      return SERIALIZABLE_UPDATE_TASK_PROGRESS_EVENT;
    }
    virtual bool Serialize(Serializer *out) const;
    virtual bool Deserialize(Deserializer *in);

    static Serializable *SerializableFactoryMethod() {
      return new ProgressEvent;
    }
   private:
    int files_total_;
    int files_complete_;
  };

  // Raised on successful completion of an UpdateTask.
  class CompletionEvent : public Event {
   public:
    CompletionEvent() : Event(COMPLETION_EVENT) {}
    CompletionEvent(const char16 *new_version_string)
      : Event(COMPLETION_EVENT),
        new_version_string_(new_version_string) {}

    // Indicates if a new version has been swapped into use by this
    // update task. May be empty.
    const std::string16 &new_version_string() const {
      return new_version_string_;
    }

    virtual SerializableClassId GetSerializableClassId() const {
      return SERIALIZABLE_UPDATE_TASK_COMPLETION_EVENT;
    }
    virtual bool Serialize(Serializer *out) const;
    virtual bool Deserialize(Deserializer *in);

    static Serializable *SerializableFactoryMethod() {
      return new CompletionEvent;
    }
   private:
    std::string16 new_version_string_;
  };

  // Raised on failed completion of an UpdateTask
  class ErrorEvent : public Event {
   public:
    ErrorEvent()
      : Event(ERROR_EVENT) {}
    ErrorEvent(const char16 *error_message)
      : Event(ERROR_EVENT), error_message_(error_message) {}
    const std::string16 &error_message() const { return error_message_; }

    virtual SerializableClassId GetSerializableClassId() const {
      return SERIALIZABLE_UPDATE_TASK_ERROR_EVENT;
    }
    virtual bool Serialize(Serializer *out) const;
    virtual bool Deserialize(Deserializer *in);

    static Serializable *SerializableFactoryMethod() {
      return new ErrorEvent;
    }
   private:
    std::string16 error_message_;
  };

  enum {
    UPDATE_TASK_COMPLETE = 0
  };

  UpdateTask(BrowsingContext *browsing_context)
      : AsyncTask(browsing_context), startup_signal_(false),
        task_503_failure_(false) {}

  // Starts an auto update task within rate limits.
  // Returns true if a task was started.
  bool MaybeAutoUpdate(int64 store_id);

  // Starts an update without checking rate limits
  virtual bool StartUpdate(ManagedResourceStore *store);

  // Waits until the update task is up and running.
  // In most cases, upon return the task will have updated the 
  // store.updateStatus property to reflect a running task.
  // In the rare case where this updateTask detects another updateTask
  // is already running and does not run itself, the updateStatus
  // property will be unaffected by this task.
  void AwaitStartup() {
    MutexLock locker(&startup_signal_lock_);
    startup_signal_lock_.Await(Condition(&startup_signal_));
  }

  // Returns true if an UpdateTask for the managed store is running
  // Note: This static method is declared here without providing an
  // implementation as the implementation is platform/browser specific.
  // See ff_update_task.cc and ie_update_task.cc
  static bool IsUpdateTaskForStoreRunning(int64 store_id);

  // Creates the right type of update task for the current platform.
  // See ff_update_task.cc, ie_update_task.cc, etc for implementation.
  static UpdateTask *CreateUpdateTask(BrowsingContext *context);

 protected:
  // Runs the update task
  virtual void Run();

  // Notifies our task listener of completion
  void NotifyTaskComplete(bool success);

  // The ManagedResourceStore being updated
  ManagedResourceStore store_;

 private:
  std::string16 notification_topic_;

  // The error message that occurred during Run(). If not set, 
  // a generic error message will be used if the process fails.
  std::string16 error_msg_;

  Mutex startup_signal_lock_;
  bool startup_signal_;

  bool task_503_failure_;

  // Initializes an update task for the store without starting it
  bool Init(ManagedResourceStore *store);

  // Sets our startup signal. Setting to true will unblock AwaitStartup callers
  void SetStartupSignal(bool startup) {
    MutexLock locker(&startup_signal_lock_);
    startup_signal_ = startup;
  }

  void NotifyObservers(Event *event);

  // Checks for a new manifest file. If found, inserts the version
  // described in the new manifest into the WebCacheDB. The newly inserted
  // version will be in the downloading state. Any pre-existing version
  // in the downloading ready state is deleted. An update task fetches the
  // manifest two times, once prior to downloading listed resources, and
  // again after having downloaded everything. The task succeeds only if
  // the manifest file from the start and end match. The 'validate_manifest'
  // argument indicates where we are in this process.
  bool UpdateManifest(std::string16 *downloading_version,
                      bool validate_manifest);

  // If there is a version in the downloading ready state, downloads all
  // entries that have not yet been downloaded. If a version is completely
  // downloaded, upon return completed_version will contain the version string.
  bool DownloadVersion(std::string16 *completed_version);

  bool HttpGetUrl(const char16 *full_url,
                  bool is_capturing,
                  const char16 *reason_header_value,
                  const char16 *if_mod_since_date,
                  WebCacheDB::PayloadInfo *payload,
                  bool *was_redirected,
                  std::string16 *full_redirect_url);

  // Helper method called by DownloadVersion.
  bool ProcessUrl(const std::string16 &url, 
                  WebCacheDB::VersionInfo *version,
                  int64 *payload_id);

  // Helper method called by DownloadVersion,
  bool FindPreviousVersionPayload(int64 server_id,
                                  const char16* url,
                                  int64 *payload_id,
                                  std::string16 *redirect_url,
                                  std::string16 *mod_date);

  // Helper method used to set the http error messages during update.
  // 'http_status' and 'optional_message' may be NULL. If non-NULL,
  // the status code and/or optional message will be included as part
  // of the error message.
  bool SetHttpError(const char16 *url, const int *http_status,
                    const char16 *optional_message);
};

#endif  // GEARS_LOCALSERVER_COMMON_UPDATE_TASK_H__
