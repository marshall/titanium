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

#ifndef GEARS_BASE_COMMON_PERMISSIONS_DB_H__
#define GEARS_BASE_COMMON_PERMISSIONS_DB_H__

#include <map>
#include "gears/base/common/database_name_table.h"
#include "gears/base/common/name_value_table.h"
#include "gears/base/common/security_model.h"
#include "gears/base/common/shortcut_table.h"
#include "gears/base/common/sqlite_wrapper.h"
#include "gears/base/common/thread_locals.h"
#include "gears/database2/database2_metadata.h"

// TODO(cprince): Consider merging this with PermissionsDB::PermissionValue.
// But note the _TEMPORARY values hae no meaning to PermissionsDB.
enum PermissionState {
  NOT_SET,
  ALLOWED_PERMANENTLY,
  ALLOWED_TEMPORARILY,
  DENIED_PERMANENTLY,
  DENIED_TEMPORARILY,
};

// This class provides an API to manage the capabilities of pages within
// Gears. Right now, it manages two capabilities: the ability to access local
// data (e.g. database and localserver) and the ability to access location data
// (e.g. geolocation). We anticipate it growing into a somewhat bigger API, 
// which would manage more fine-grained capabilities, such as the
// ability to store more than 1MB on disk, etc.
//
// TODO(aa): Think about factoring some of the commonalities between this class
// and WebCacheDB into a common base class.
//
// TODO(cprince): Should rename this class and file to SettingsDB, as we store
// shortcut info here too. (But be careful to preserve the on-disk filename.)
class PermissionsDB {
 public:
  // When shortcuts are added or removed, this topic is broadcast
  // thru MessageService.NotifyObservers with a NULL data object.
  static const char16 *kShortcutsChangedTopic;

  // The allowable values of a permission.
  enum PermissionValue {
    PERMISSION_NOT_SET = 0,  // origin has no persisted value
    PERMISSION_ALLOWED = 1,
    PERMISSION_DENIED = 2,
  };

  // The existing types of permissions.
  enum PermissionType {
    // database(manager), localserver, workerpool, audio(recorder), canvas
    PERMISSION_LOCAL_DATA,
    // geolocation
    PERMISSION_LOCATION_DATA,
  };

  // Gets a thread-specific PermissionsDB instance.
  static PermissionsDB *GetDB();

  // Sets the Gears access level for a given SecurityOrigin and PermissionType.
  void SetPermission(const SecurityOrigin &origin,
                     PermissionType type,
                     PermissionValue value);

  // Gets the Gears access level for a given SecurityOrigin and PermissionType.
  PermissionsDB::PermissionValue GetPermission(const SecurityOrigin &origin,
                                               PermissionType type);

  // Returns whether Gears should supress any dialogs. This setting is used by
  // applications that want to do automated testing of Gears where dialogs are
  // an annoyance.
  bool ShouldSupressDialogs();

  // Returns true if the origin has the requested permission type.
  bool IsOriginAllowed(const SecurityOrigin &origin, PermissionType type) {
    return GetPermission(origin, type) == PERMISSION_ALLOWED;
  }

  // Gets all the origins with a specific value for the given PermissionType.
  bool GetOriginsByValue(PermissionValue value,
                         PermissionType type,
                         std::vector<SecurityOrigin> *result);

  // Returns a vector of pairs of origin names and sets of
  // (PermissionType, PermissionValue) pairs.
  typedef std::map<PermissionType, PermissionValue> OriginPermissions;
  typedef std::vector<std::pair<std::string16, OriginPermissions> >
      PermissionsList;
  bool GetPermissionsSorted(PermissionsList *permission_list_out);

  // Attempts to enable access to Gears for a worker with the given
  // SecurityOrigin.
  bool EnableGearsForWorker(const SecurityOrigin &worker_origin,
                            const SecurityOrigin &host_origin);

  // The key used to cache instances of PermissionsDB in ThreadLocals.
  static const ThreadLocals::Slot kThreadLocalKey;

  // Adds (or overwrites) a shortcut for origin/name, with appUrl,
  // iconUrl, msg as data, and whether or not to allow creation of this
  // shortcut.
  bool SetShortcut(const SecurityOrigin &origin, const char16 *name,
                   const char16 *app_url,
                   const char16 *icon16x16_url,
                   const char16 *icon32x32_url,
                   const char16 *icon48x48_url,
                   const char16 *icon128x128_url,
                   const char16 *msg,
                   const bool allow);

  // Gets the set of origins which have shortcuts.
  bool GetOriginsWithShortcuts(std::vector<SecurityOrigin> *result);

  // Gets the set of named shortcuts for a specific origin.
  bool GetOriginShortcuts(const SecurityOrigin &origin,
                          std::vector<std::string16> *names);

  // Gets the data for a specific shortcut.
  bool GetShortcut(const SecurityOrigin &origin, const char16 *name,
                   std::string16 *app_url,
                   std::string16 *icon16x16_url,
                   std::string16 *icon32x32_url,
                   std::string16 *icon48x48_url,
                   std::string16 *icon128x128_url,
                   std::string16 *msg,
                   bool *allow);

  // Deletes a specific shortcut.
  bool DeleteShortcut(const SecurityOrigin &origin, const char16 *name);

  // Deletes all shortcuts for an origin.
  bool DeleteShortcuts(const SecurityOrigin &origin);

  // For a given database_name, fill basename with the name of the
  // file to use in origin's directory, and returns true if
  // successful.
  bool GetDatabaseBasename(const SecurityOrigin &origin,
                           const char16 *database_name,
                           std::string16 *basename);

  // Mark the given database basename corrupt so that future calls to
  // GetDatabaseBasename will no longer return it.  The basename is
  // required because another thread of control could have already
  // invalidated the database for the origin.
  bool MarkDatabaseCorrupt(const SecurityOrigin &origin,
                           const char16 *database_name,
                           const char16 *basename);

  Database2Metadata *database2_metadata() { return &database2_metadata_; }

 private:
  // Private constructor, callers must use GetDB().
  PermissionsDB();

  // Initializes the database. Must be called before other methods.
  bool Init();

  // Creates the database's schema.
  bool CreateDatabase();

  // Upgrades the database's schema to the current version.
  bool UpgradeDatabase(int current_version);

  // Schema upgrade scripts. Don't forget to add new scripts to
  // kSchemaUpgradeScripts as well.
  bool UpgradeToVersion9();
  bool UpgradeToVersion8();
  bool UpgradeToVersion7();
  bool UpgradeToVersion6();
  bool UpgradeToVersion5();
  bool UpgradeToVersion4();
  bool UpgradeToVersion3();
  bool UpgradeToVersion2();

  // Destructor function called by ThreadLocals to dispose of a thread-specific
  // DB instance when a thread dies.
  static void DestroyDB(void *context);

  // Internal utility that tries to allow the given permission for the
  // given origin.
  bool TryAllow(const SecurityOrigin &origin,
                PermissionType type);

  // Maps a permission type to an access table.
  NameValueTable* GetTableForPermissionType(PermissionType type);

  // Database we use to store capabilities information.
  SQLDatabase db_;

  // Stores simple name/value pairs for internal settings and configuration.
  NameValueTable settings_table_;

  // Maps origins to ability to access local data in Gears.
  NameValueTable local_data_access_table_;

  // Maps origins to ability to access location data in Gears.
  NameValueTable location_access_table_;

  // Shortcuts origins have defined.
  ShortcutTable shortcut_table_;
  
  DatabaseNameTable database_name_table_;
  Database2Metadata database2_metadata_;

  DISALLOW_EVIL_CONSTRUCTORS(PermissionsDB);
  DECL_SINGLE_THREAD
};

#endif  // GEARS_BASE_COMMON_PERMISSIONS_DB_H__
