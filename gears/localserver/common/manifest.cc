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

#include <assert.h>
#include <set>

#include "gears/localserver/common/manifest.h"

#include "gears/base/common/common.h"
#include "gears/base/common/string_utils.h"
#include "gears/base/common/url_utils.h"
#include "third_party/jsoncpp/json.h"

//------------------------------------------------------------------------------
// JsonUtils
//------------------------------------------------------------------------------

class JsonUtils {
public:
  static int GetInteger(const Json::Value &object,
                        const char *name) {
    return GetInteger(object, name, 0);
  }

  static int GetInteger(const Json::Value &object,
                        const char *name,
                        int default_value);

  static bool GetString16(const Json::Value &object,
                          const char *name,
                          std::string16 *out);

  static bool GetBool(const Json::Value &object,
                      const char *name,
                      bool default_value);

 private:
  static bool GetString(const Json::Value &object,
                        const char *name,
                        std::string *out);
};

int JsonUtils::GetInteger(const Json::Value &object,
                          const char *name,
                          int default_value) {
  assert(object.isObject());
  const Json::Value value = object.get(name, Json::Value::null);
  if (value.isInt())
    return value.asInt();
  else
    return default_value;
}

bool JsonUtils::GetString(const Json::Value &object,
                          const char *name,
                          std::string *out) {
  assert(object.isObject());
  const Json::Value value = object.get(name, Json::Value::null);
  if (!value.isString())
    return false;
  *out = value.asString();
  return true;
}

bool JsonUtils::GetString16(const Json::Value &object,
                            const char *name,
                            std::string16 *out) {
  assert(object.isObject());
  const Json::Value value = object.get(name, Json::Value::null);
  if (!value.isString())
    return false;
  return UTF8ToString16(value.asCString(), out);
}

bool JsonUtils::GetBool(const Json::Value &object,
                        const char *name,
                        bool default_value) {
  assert(object.isObject());
  const Json::Value value = object.get(name, Json::Value::null);
  if (value.isBool())
    return value.asBool();
  else
    return default_value;
}

//------------------------------------------------------------------------------
// Parse
//------------------------------------------------------------------------------
bool Manifest::Parse(const char16 *manifest_url, const char *json, int len) {
#define MANIFEST_VERSION_FIELD "betaManifestVersion"
#define L_MANIFEST_VERSION_FIELD L"betaManifestVersion"
  static const char *kManifestVersionField = MANIFEST_VERSION_FIELD;
  static const char *kVersionField = "version";
  static const char *kRedirectUrlField = "redirectUrl";
  static const char *kEntriesField = "entries";
  static const char *kUrlField = "url";
  static const char *kSrcField = "src";
  static const char *kRedirectField = "redirect";
  static const char *kIgnoreQueryField = "ignoreQuery";
  static const int kExpectedManifestVersion = 1;

  assert(manifest_url && manifest_url[0]);
  assert(json);
  assert(len >= 0);

#ifdef DEBUG
  // Log a portion of manifest file we're parsing
  std::string json_null_terminated(json, (len < 1024) ? len : 1024);
  LOG(("Manifest::Parse - parsing\n"));
  LOG(("%s", json_null_terminated.c_str()));
  LOG(("\n"));
#endif


  is_valid_ = false;
  manifest_url_ = manifest_url;
  if (!manifest_origin_.InitFromUrl(manifest_url)) {
    error_message_ = STRING16(L"Failed to get manifest url origin");
    return false;
  }

  version_.clear();
  entries_.clear();
  redirect_url_.clear();
  error_message_.clear();

  // Parse the JSON data
  Json::Value root;
  Json::Reader reader;
  bool ok = reader.parse(json, json + len, root, false);
  if (!ok) {
    UTF8ToString16(reader.getFormatedErrorMessages().c_str(),
                   &error_message_);
    return false;
  }
  if (!root.isObject()) {
    error_message_ = STRING16(L"Not an object");
    return false;
  }

  // Verify the data format is the version we're expecting
  if (JsonUtils::GetInteger(root, kManifestVersionField) !=
      kExpectedManifestVersion) {
    error_message_ = STRING16(L"Invalid '"
                              L_MANIFEST_VERSION_FIELD
                              L"' attribute");
    return false;
  }

  // Get the values of interest

  if (!JsonUtils::GetString16(root, kVersionField, &version_) ||
      version_.empty()) {
    error_message_ = STRING16(L"Missing 'version' attribute");
    return false;   // version is a required field
  }
  JsonUtils::GetString16(root, 
                         kRedirectUrlField,
                         &redirect_url_);

  // We use the array indexer to get a reference rather than a copy
  // of the entries value
  const Json::Value &entries = root[kEntriesField];
  if (!entries.isArray()) {
    error_message_ = STRING16(L"Missing 'entries' array");
    return false;
  }


  for (size_t i = 0; i < entries.size(); ++i) {
    entries_.push_back(Entry());
    Entry *entry = &entries_.back();

    // url is a required field
    if (!JsonUtils::GetString16(entries[i], kUrlField, &entry->url)) {
      error_message_ = STRING16(L"Invalid entry - missing 'url' attribute");
      return false;
    }

    // ignoreQuery is an optional field (defaults to false)
    entry->ignore_query = JsonUtils::GetBool(entries[i], kIgnoreQueryField,
                                             false);
    if (entry->ignore_query && (entry->url.find('?') != std::string16::npos)) {
      error_message_ = STRING16(
          L"Invalid entry - ignoreQuery will never match "
          L"a url containing a '?'");
      return false;
    }

    // src and redirect are optional but mutually exclusive fields
    JsonUtils::GetString16(entries[i], kSrcField, &entry->src);
    JsonUtils::GetString16(entries[i], kRedirectField, &entry->redirect);
    if (!entry->src.empty() && !entry->redirect.empty()) {
      error_message_ = STRING16(L"Invalid entry - 'src' and 'redirect'"
                                L" attributes cannot both be present");
      return false;
    }
  }

  is_valid_ = ResolveRelativeUrls();

  return is_valid_;
}

//------------------------------------------------------------------------------
// ResolveRelativeUrls
//------------------------------------------------------------------------------
bool Manifest::ResolveRelativeUrls() {
  const bool kCheckOrigin = true;
  const bool kDontCheckOrigin = false;
  const char16 *base = manifest_url_.c_str();

  if (!redirect_url_.empty()) {
    if (!ResolveRelativeUrl(base, &redirect_url_, kDontCheckOrigin)) {
      return false;
    }
  }

  for (std::vector<Entry>::iterator iter = entries_.begin();
       iter != entries_.end();
       ++iter) {
    if (!ResolveRelativeUrl(base, &iter->url, kCheckOrigin)) {
      return false;
    }
    if (!iter->src.empty() &&  // src is optional
        !ResolveRelativeUrl(base, &iter->src, kCheckOrigin)) {
      return false;
    }
    if (!iter->redirect.empty() && // redirect is optional
        !ResolveRelativeUrl(base, &iter->redirect, kDontCheckOrigin)) {
      return false;
    }
  }

  return true;
}

bool Manifest::ResolveRelativeUrl(const char16 *base,
                                  std::string16 *url,
                                  bool check_origin) {
  const char16 *kResolveErrorMessage = STRING16(L"Failed to resolve url - ");
  const char16 *kNotSameOriginErrorMessage =
      STRING16(L"Url is not from the same origin - ");
  std::string16 resolved;
  if (!::ResolveAndNormalize(base, url->c_str(), &resolved)) {
    error_message_ = kResolveErrorMessage;
    error_message_ += *url;
    return false;
  }
  if (check_origin && !manifest_origin_.IsSameOriginAsUrl(resolved.c_str())) {
    error_message_ = kNotSameOriginErrorMessage;
    error_message_ += *url;
    return false;
  }

  *url = resolved;
  return true;
}
