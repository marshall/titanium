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

#include "gears/base/common/mime_detect.h"

#include "gears/base/common/file.h"
#include "gears/base/common/string_utils.h"

#if BROWSER_FF
#include <gecko_sdk/include/nsCOMPtr.h>
#include <gecko_sdk/include/nsILocalFile.h>
#include <gecko_sdk/include/nsServiceManagerUtils.h>
#include <gecko_sdk/include/nsStringAPI.h>
#include <gecko_sdk/include/nsXPCOM.h>
#include <gecko_internal/nsIMIMEService.h>

#elif defined(WIN32) && !defined(WINCE)
#include <shlwapi.h>
#include <windows.h>
#endif


std::string16 DetectMimeTypeOfFile(const std::string16 &filename) {
  static const std::string16 kDefaultMimeType(
      STRING16(L"application/octet-stream"));

#if BROWSER_FF
  nsCOMPtr<nsIFile> file;
  nsCOMPtr<nsILocalFile> localFile;
  nsString filename_as_nsstring(filename.c_str());
  if (NS_FAILED(NS_NewLocalFile(filename_as_nsstring, PR_FALSE,
                getter_AddRefs(localFile)))) {
    return kDefaultMimeType;
  }
  file = localFile;

  nsCOMPtr<nsIMIMEService> mime_service = do_GetService("@mozilla.org/mime;1");
  if (!mime_service) {
    return kDefaultMimeType;
  }
  nsCString mime_type;
  if (NS_FAILED(mime_service->GetTypeFromFile(file, mime_type))) {
    return kDefaultMimeType;
  }

  std::string result_as_utf8(mime_type.get());
  std::string16 result;
  UTF8ToString16(result_as_utf8.c_str(), &result);
  return result;

#elif defined(WIN32) && !defined(WINCE)
  uint8 buffer[256];
  memset(buffer, 0, sizeof(uint8) * 256);
  int64 bytes_read_64 = File::ReadFileSegmentToBuffer(filename.c_str(),
                                                      buffer, 0, 256);
  if (bytes_read_64 < 0) {
    return kDefaultMimeType;
  }
  assert(bytes_read_64 <= 256);
  int bytes_read = static_cast<int>(bytes_read_64);
  WCHAR *mime_type = NULL;
  if (FAILED(FindMimeFromData(NULL, PathFindExtensionW(filename.c_str()),
                              buffer, bytes_read, L"application/octet-stream",
                              FMFD_DEFAULT, &mime_type, 0))) {
    return kDefaultMimeType;
  }
  if (!mime_type) {
    return kDefaultMimeType;
  }
  std::string16 result(mime_type);
  CoTaskMemFree(mime_type);
  return result;

#else
  return kDefaultMimeType;
#endif
}
