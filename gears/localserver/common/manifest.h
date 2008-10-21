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

#ifndef GEARS_LOCALSERVER_COMMON_MANIFEST_H__
#define GEARS_LOCALSERVER_COMMON_MANIFEST_H__

#include <vector>
#include "gears/base/common/security_model.h"
#include "gears/base/common/string16.h"

//------------------------------------------------------------------------------
// Representation of a manifest that knows how to parse the JSON format.
//------------------------------------------------------------------------------
class Manifest {
 public:
  struct Entry {
    std::string16 url;
    std::string16 src;
    std::string16 redirect;
    bool ignore_query;

    Entry() : ignore_query(false) {}
  };

  Manifest() : is_valid_(false) {}

  // Parses JSON utf8 encoded data populates the Manifest object's data members
  bool Parse(const char16 *full_manifest_url, const char *json, int len);
  bool Parse(const char16 *full_manifest_url, const char *json) {
    return Parse(full_manifest_url, json, strlen(json));
  }

  // Whether or not parsing was successful
  bool IsValid() const { return is_valid_; }

  // Returns the manifest url passed into Parse
  const char16 *GetManifestUrl() const { return manifest_url_.c_str(); }

  // Returns the version property of the Manifest
  const char16 *GetVersion() const { return version_.c_str(); }

  // Returns the redirect property of the Manifest
  const char16 *GetRedirectUrl() const {
    return redirect_url_.c_str();
  }

  // Returns the array of entries from the Manifest
  const std::vector<Entry> *GetEntries() { return &entries_; }

  // Returns an error message indicating why parsing failed
  const char16 *GetErrorMessage() { return error_message_.c_str(); }

 private:
  bool ResolveRelativeUrls();
  bool ResolveRelativeUrl(const char16 *base,
                          std::string16 *url,
                          bool check_origin);

  bool is_valid_;
  std::string16 manifest_url_;
  std::string16 version_;
  std::string16 redirect_url_;
  std::vector<Entry> entries_;
  std::string16 error_message_;
  SecurityOrigin manifest_origin_;
};

#endif  // GEARS_LOCALSERVER_COMMON_MANIFEST_H__
