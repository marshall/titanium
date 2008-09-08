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
//
// Implementation of Windows specific functions that don't exist in Windows
// Mobile 5.

// TODO(andreip): remove platform-specific #ifdef guards when OS-specific
// sources (e.g. WIN32_CPPSRCS) are implemented.
#ifdef WINCE
#include "gears/base/common/wince_compatibility.h"

#include <shellapi.h>
#include <wininet.h>  // For CreateUrlCacheEntry etc.

#include "gears/base/common/file.h"
#include "gears/base/common/js_runner.h"
#include "gears/base/common/js_runner_utils.h"  // For EscapeMessage().
#include "gears/base/common/paths.h"
#include "gears/base/common/string_utils.h"
#include "gears/localserver/common/http_constants.h"
#include "third_party/scoped_ptr/scoped_ptr.h"

HANDLE CMutexWince::global_mutex_ = NULL;
CriticalSection CMutexWince::lock_;
const char16* kGlobalMutexName =
    STRING16(PRODUCT_SHORT_NAME L"GearsGlobalMutexWince");

// Used by SHCreateDirectoryEx.
static void SkipTokens(const std::string16 &path,
                       int &pos,
                       bool skip_separators);

// Used by BrowserCache methods.
static void IncrementFiletime(FILETIME *file_time,
                              const int64 &hundreds_of_nanoseconds);
static bool IsFiletimeGreater(const FILETIME &left_hand,
                              const FILETIME &right_hand);
// Returns a pointer to a newly allocated INTERNET_CACHE_ENTRY_INFO structure,
// or NULL on failure.
static INTERNET_CACHE_ENTRY_INFO* GetEntryInfo(const char16 *url);
// Determines if a cache entry is a bogus Gears entry.
static bool IsEntryBogus(INTERNET_CACHE_ENTRY_INFO *info);

// There seem to be no way to implement this properly on Windows Mobile
// since the algorithm for path shortening isn't fully specified, according
// to http://en.wikipedia.org/wiki/8.3_filename. Using FindFirstFileA isn't
// an alternative either since that method isn't exported by coredll.lib.
// FindFirstFileW uses a different structure to return the file information
// and that structure is missing exactly the cAlternateFilename field, which
// would have been the one that contained the short name.
DWORD GetShortPathNameW(LPCTSTR path_long,
                        LPTSTR path_short,
                        DWORD path_short_max_size) {
 int long_path_size = wcslen(path_long) + 1;  // +1 for the ending \0
 if (long_path_size <= static_cast<int>(path_short_max_size)) {
   wcsncpy(path_short, path_long, path_short_max_size);
   return long_path_size - 1;
 } else {
   return long_path_size;
 }
}

// This function creates a file system folder whose fully qualified
// path is given by full_dirpath. If one or more of the intermediate folders
// do not exist, they are created as well.
// According to http://msdn2.microsoft.com/en-us/library/aa365247.aspx
// Windows API functions accept both "\" and "/", so we will do the same.
int SHCreateDirectoryEx(HWND window,
                        LPCTSTR full_dirpath,
                        const SECURITY_ATTRIBUTES *security_attributes) {
  std::string16 path(full_dirpath);
  if (!path.length()) return ERROR_BAD_PATHNAME;

  // Traverse the path and create the directories one by one.
  int pos = 0;
  // Skip leading separators.
  SkipTokens(path, pos, true);
  while (pos < static_cast<int>(path.length())) {
    // Find next separator.
    SkipTokens(path, pos, false);
    // Skip next consecutive separators, if any.
    SkipTokens(path, pos, true);
    // Create the directory.
    if (!CreateDirectory(path.substr(0, pos).c_str(), NULL)) {
      DWORD error = GetLastError();
      if (error != ERROR_ALREADY_EXISTS) return error;
    }
  }
  return GetLastError();
}

HRESULT SHGetFolderPath(HWND hwndOwner,
                        int nFolder,
                        HANDLE hToken,
                        DWORD dwFlags,
                        LPTSTR pszPath) {
  BOOL result = SHGetSpecialFolderPath(hwndOwner, pszPath, nFolder, false);
  return result ? S_OK : E_FAIL;
}

BOOL IsNetworkAlive(LPDWORD lpdwFlags) {
  BOOL alive = false;
  CONNMGR_CONNECTION_DETAILED_STATUS* status_buffer_ptr = NULL;
  DWORD size = 0;
  HRESULT hr = ConnMgrQueryDetailedStatus(status_buffer_ptr, &size);
  if (hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
    uint8* buffer = new uint8[size];
    status_buffer_ptr = reinterpret_cast<CONNMGR_CONNECTION_DETAILED_STATUS*>
        (buffer);
    ZeroMemory(status_buffer_ptr, size);
    hr = ConnMgrQueryDetailedStatus(status_buffer_ptr, &size);
    if (SUCCEEDED(hr)) {
      while (status_buffer_ptr) {
        if (status_buffer_ptr->dwConnectionStatus == CONNMGR_STATUS_CONNECTED &&
            (status_buffer_ptr->pIPAddr != NULL ||
            status_buffer_ptr->dwType == CM_CONNTYPE_PROXY)) {
          // We conclude that the network is alive if there is one
          // connection in the CONNMGR_STATUS_CONNECTED state and
          // the device has an IP address or it is connected in
          // proxy mode (e.g. ActiveSync).
          //
          // Testing shows that on (some?) Windows Mobile 6 Standard devices,
          // the connection entry for an ActiveSync connection erroneously
          // shows a connection status of CONNMGR_STATUS_DISCONNECTED when
          // ActiveSync is in fact connected. As a result, this method will
          // return false when such a device is connected only through
          // ActiveSync. There seems to be now way to get around this problem.
          alive = true;
          break;
        }
        status_buffer_ptr = status_buffer_ptr->pNext;
      }
    }
    delete [] buffer;
  }
  return alive;
}

BOOL CMutexWince::Open(DWORD dwAccess, BOOL bInheritHandle, LPCTSTR pszName) {  
  // On Windows Mobile we are forced to implement CMutex::Open() using
  // the CreateMutex() win32 API function. This will open an existing mutex
  // or, if one doesn't exist already, will create a new mutex. However,
  // given the semantics of ATL CMutex::Open(), the creation of a new mutex
  // is an unwanted side-effect and we need to hide it from other processes
  // that are simultaneously calling this method. We therefore need to
  // serialize Open method calls using a global mutex. Furthermore,
  // we also need to use a critical section to guard against
  // concurrent initialization of this global mutex by different threads.
  CritSecLock locker(lock_);
  assert(m_h == NULL);
  if (!global_mutex_) {
    global_mutex_ = CreateMutex(NULL, FALSE, kGlobalMutexName);
    if (!global_mutex_) return false;  // Returning early!
  }
  // We now have a handle to the global mutex. We may have created
  // it or may have opened the existing one if another process had
  // already created it. Although we cannot have multiple instances
  // of IE mobile running at the same time, the browser control
  // (together with Gears) may be embedded in some other application.
  BOOL success = false;
  DWORD result = WaitForSingleObject(global_mutex_, INFINITE);
  if (result == WAIT_OBJECT_0) {
    // We have ownership of global_mutex_.
    m_h = CreateMutex(NULL, FALSE, pszName);
    if (m_h) {
      // If m_h is not NULL, GetLastError() can only return
      // ERROR_ALREADY_EXISTS or success.
      if (GetLastError() != ERROR_ALREADY_EXISTS) {
        // We didn't mean to create a mutex here, so let's close it.
        CloseHandle(m_h);
        m_h = NULL;
      } else {
        success = true;
      }
    }
    // Give up ownership of global_mutex_.
    ReleaseMutex(global_mutex_);
  }
  return success;
}

// This function is required because on WinCE, throwing a JavaScript exception
// from C++ doesn't trigger the default JS exception handler.
//
// We try to call window.onerror. If this fails, we show an alert if script
// errors are enabled in the browser.
void CallWindowOnerror(JsRunnerInterface *js_runner,
                       const std::string16 &message) {
  std::string16 escaped_message = EscapeMessage(message);
  const std::string16 kEndBracket(STRING16(L"')"));
  // Protect against recursion when we call Eval, which detects an exception and
  // calls us again.
  static bool is_calling_eval = false;
  if (is_calling_eval) {
    return;
  }
  // Try window.onerror first.
  std::string16 onerror_string(L"window.onerror('");
  onerror_string.append(escaped_message);
  onerror_string.append(kEndBracket);
  is_calling_eval = true;
  bool ret = js_runner->Eval(onerror_string.c_str());
  is_calling_eval = false;
  if (ret == true) {
    return;
  }
  // Calling window.onerror failed. Try to read the registry setting
  // that determines whether JS errors are shown to the user or not.
  CRegKey key;
  if (key.Open(HKEY_CURRENT_USER,
               L"Software\\Microsoft\\Internet Explorer\\Main",
               KEY_READ) != ERROR_SUCCESS) {
    // This key should always exist. Failure to open it signals an error.
    return;
  }
  DWORD show_script_errors = 0;
  if (key.QueryDWORDValue(L"ShowScriptErrors", show_script_errors) !=
      ERROR_SUCCESS) {
    // The key is not set, so we don't need to do anything.
    return;
  }
  if (show_script_errors == 1) {
    std::string16 alert_string(L"alert('");
    alert_string.append(escaped_message);
    alert_string.append(kEndBracket);
    is_calling_eval = true;
    js_runner->Eval(alert_string.c_str());
    is_calling_eval = false;
  } else {
    // The key was set to 0 (or some other value than 1), so we
    // don't need to do anything.
    return;
  }
}

// BrowserCache

// A cache entry inserted with NORMAL_CACHE_ENTRY is used whenever the
// LocalServer can not serve the resource it represents, even if the device is
// online and the resource is available. This means that we have stale
// empty cache entries still present for resources that are no longer in the
// LocalServer, they will be served in preference to the real resource when
// online.
//
// A cache entry inserted with EDITED_CACHE_ENTRY does not suffer from this
// problem. The presence of the cache entry prevents the device from showing
// the 'Cannot connect' popup when the resource is served by the LocalServer,
// but when LocalServer can not serve the resource, the cache entry is not
// used. This means that stale cache entries for resources no longer in the
// LocalServer do not cause a problem.
static const DWORD kGearsBogusEntryType = EDITED_CACHE_ENTRY;
static const char16 *kGearsBogusEntryHeader = L"GearsBogusEntry: 1\r\n";
static const char16 *kGearsBogusEntryFileExtension = L"nul";

// static
bool BrowserCache::EnsureBogusEntry(const char16 *url) {
  // Prepare the expire time. This is used in multiple cases below.
  const __int64 kHundredsOfNanosecondsPerYear = 315360000000000;

  SYSTEMTIME systemtime;
  GetSystemTime(&systemtime);
  FILETIME current_time;
  SystemTimeToFileTime(&systemtime, &current_time);
  FILETIME expire_time = current_time;
  IncrementFiletime(&expire_time, kHundredsOfNanosecondsPerYear);
#ifdef DEBUG
  SYSTEMTIME check_time;
  FileTimeToSystemTime(&expire_time, &check_time);
  LOG16((L"BrowserCache: Using cache expire time: %d/%d/%d\n",
         check_time.wMonth, check_time.wDay, check_time.wYear));
#endif
  // This will only fail if there's no cache entry.
  scoped_array<INTERNET_CACHE_ENTRY_INFO> info(GetEntryInfo(url));
  if (info.get()) {
    // If the existing entry is a bogus Gears entry, we update the expire time.
    // Note that a bogus cache entry should always have an expire time set, but
    // it's possible that the entry was modified by another application.
    if (IsEntryBogus(info.get())) {
      INTERNET_CACHE_ENTRY_INFO new_info;
      new_info.ExpireTime = expire_time;
      if (SetUrlCacheEntryInfo(url, &new_info, CACHE_ENTRY_EXPTIME_FC) ==
          FALSE) {
        LOG16((L"Failed to update bogus cache entry for %s : %s.\n",
               url,
               GetLastErrorString().c_str()));
        return false;
      }
      LOG16((L"BrowserCache: Updated bogus cache entry for %s.\n", url));
      return true;
    }
    // If the cache entry is not a bogus Gears entry, the expire time may not be
    // valid. Such a cache entry will not prevent the 'Cannot Connect' popup
    // when using LocalServer. In this case, rather than extend the validity of
    // such an entry, we create a new bogus entry. Otherwise, we leave existing
    // non-bogus entries untouched.
    if (IsFiletimeGreater(info.get()->ExpireTime, current_time)) {
      LOG16((L"BrowserCache: Non-bogus cache entry with valid expire time "
             L"already exists for %s.\n", url));
      return true;
    }
    LOG16((L"BrowserCache: Non-bogus cache entry exists but has invalid expire "
           L"time for %s.\n", url));
  }
  // If there's no entry, or a non-bogus entry without an expire time, we
  // create a new entry. First we get the local file name that will be used to
  // store this cache entry.
  char16 local_file[MAX_PATH + 1];
  if (FALSE == CreateUrlCacheEntry(
                   url,             // URL
                   0,               // Expected file size (0 for unknown)
                   kGearsBogusEntryFileExtension,  // Local file name extension
                   local_file,      // File name
                   0)) {            // Reserved
    LOG16((L"Failed to create bogus cache entry for %s : %s\n",
           url,
           GetLastErrorString().c_str()));
    return false;
  }
  // This header value is required for the cache entry to be used correctly. See
  // http://msdn2.microsoft.com/en-us/library/aa383943(VS.85).aspx.
  std::string16 cache_header = L"HTTP/1.0 200 OK\r\n";
  cache_header += kGearsBogusEntryHeader;
  cache_header += HttpConstants::kCrLf;
  FILETIME zero = {0};
  // Add the entry to the cache.
  // TODO(steveblock): Investigate why this occasionally fails.
  if (CommitUrlCacheEntry(
         url,                            // URL
         local_file,                     // Local file
         expire_time,                    // Expire time (zero for unknown)
         zero,                           // Last modified time (zero OK)
         kGearsBogusEntryType,           // Cache entry type
         const_cast<char16*>(cache_header.c_str()),  // Header
         cache_header.size(),            // Header size
         kGearsBogusEntryFileExtension,  // Local file name extension
         0) == FALSE) {                  // Reserved
    LOG16((L"Failed to insert bogus cache entry for %s : %s\n",
           url,
           GetLastErrorString().c_str()));
    return false;
  }
  LOG16((L"BrowserCache: Inserted bogus cache entry for %s.\n", url));
  return true;
}

// static
bool BrowserCache::RemoveBogusEntry(const char16 *url) {
  scoped_array<INTERNET_CACHE_ENTRY_INFO> info(GetEntryInfo(url));
  if (info.get()) {
    if (IsEntryBogus(info.get())) {
      // This entry is bogus, so we can remove it.
      std::string16 file_name = info.get()->lpszLocalFileName;
      // It seems that for files which we have created ourselves, the local file
      // name is reported incorrectly. A file name \Windows\path\filename.nul
      // gets corrupted to \Windows\path\\Windows\path\filename.nul.
      // TODO(steveblock): Add a unit test to confirm that this work-around is
      // always successful and investigate whether the behaviour is the same on
      // all versions of Windows Mobile.
      const char16 *corrected_file_name = file_name.c_str();
      unsigned pos = file_name.find(L"\\\\");
      if (std::string16::npos != pos) {
        corrected_file_name += pos + 1;
      }
      bool success = true;
      if (!File::Delete(corrected_file_name)) {
        LOG16((L"BrowserCache: Failed to delete local file %s for %s.\n",
               corrected_file_name,
               url));
        success = false;
      }
      // Remove the cache entry.
      if (DeleteUrlCacheEntry(url) == FALSE) {
        LOG16((L"BrowserCache: Failed to remove cache entry for : %s\n",
               url,
               GetLastErrorString().c_str()));
        success = false;
      }
      LOG16((L"BrowserCache: Removed bogus cache entry for %s.\n", url));
      return success;
    }
  }
  LOG16((L"BrowserCache: No bogus cache entry for %s, not removing.\n", url));
  return true;
}

// Internal

static bool IsSeparator(const char16 token) {
  static const char16 kPathSeparatorAlternative = L'/';
  return ((token == kPathSeparator) || (token == kPathSeparatorAlternative));
}

// Skips tokens of the given type (separators or non-separators).
static void SkipTokens(const std::string16 &path,
                       int &pos,
                       bool skip_separators) {
  while (pos < static_cast<int>(path.length()) &&
        (IsSeparator(path[pos]) == skip_separators)) {
    pos++;
  }
}

static void IncrementFiletime(FILETIME *file_time,
                              const int64 &hundreds_of_nanoseconds) {
  int64 file_time_integer = static_cast<int64>(file_time->dwHighDateTime) << 32;
  file_time_integer += file_time->dwLowDateTime;
  file_time_integer += hundreds_of_nanoseconds;
  file_time->dwLowDateTime  = static_cast<DWORD>(file_time_integer);
  file_time->dwHighDateTime = static_cast<DWORD>(file_time_integer >> 32);
}

static bool IsFiletimeGreater(const FILETIME &left_hand,
                              const FILETIME &right_hand) {
  if (left_hand.dwHighDateTime > right_hand.dwHighDateTime) {
    return true;
  } else if (left_hand.dwHighDateTime == right_hand.dwHighDateTime) {
    return left_hand.dwLowDateTime > right_hand.dwLowDateTime;
  } else {
    return false;
  }
}

static INTERNET_CACHE_ENTRY_INFO* GetEntryInfo(const char16 *url) {
  DWORD info_size = 0;
  // This call should always fail because we pass NULL for the pointer to the
  // info structure. If the URL is present in the cache, GetLastError() will
  // return ERROR_INSUFFICIENT_BUFFER, whereas if the URL is not present,
  // GetLastError() will return ERROR_FILE_NOT_FOUND.
  BOOL ret = GetUrlCacheEntryInfo(url, NULL, &info_size);
  assert(FALSE == ret);
  if (GetLastError() == ERROR_FILE_NOT_FOUND) {
    return NULL;
  }
  INTERNET_CACHE_ENTRY_INFO *info =
      reinterpret_cast<INTERNET_CACHE_ENTRY_INFO*>(new char16[info_size]);
  info->dwStructSize = info_size;
  // This may fail if the cache entry has been deleted since we called
  // GetUrlCacheEntryInfo above.
  if (GetUrlCacheEntryInfo(url, info, &info_size) == FALSE) {
    delete[] info;
    return NULL;
  }
  return info;
}

static bool IsEntryBogus(INTERNET_CACHE_ENTRY_INFO *info) {
  assert(info);
  int64 size = static_cast<int64>(info->dwSizeHigh) << 32;
  size += static_cast<int64>(info->dwSizeLow);
  std::string16 header_info = info->lpHeaderInfo;

  // We test for ...
  // - size == 0
  // - CacheEntryType includes the correct flag
  // - lpHeaderInfo constains special Gears bogus entry string
  // - lpszFileExtension is the Gears bogus entry file extension
  return 0 == size &&
         info->CacheEntryType & kGearsBogusEntryType &&
         header_info.find(kGearsBogusEntryHeader) != -1 &&
         wcscmp(info->lpszFileExtension, kGearsBogusEntryFileExtension) == 0;
}

#ifdef USING_CCTESTS
// These methods are defined in cctests\test.h and are used only for testing as
// a means to access the static functions defined here.
INTERNET_CACHE_ENTRY_INFO* GetEntryInfoTest(const char16 *url) {
  return GetEntryInfo(url);
}
bool IsEntryBogusTest(INTERNET_CACHE_ENTRY_INFO *info) {
  return IsEntryBogus(info);
}
#endif

#endif  // WINCE
