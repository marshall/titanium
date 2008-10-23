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

#include "gears/inspector/inspector_resources.h"

#include "gears/base/common/basictypes.h"  // for ARRAYSIZE
#include "gears/localserver/common/http_constants.h"

// Uncomment this block when using the "modify and test" code below.
/*
#include <vector>
#include "gears/base/common/file.h"
#include "gears/base/common/paths.h"
#include "gears/base/common/string_utils.h"
*/


bool GetInspectorResource(const std::string16 &fullpath,
                          const unsigned char **data, size_t *size,
                          std::string16 *headers) {
  // Early exits, to avoid the cost of a full lookup for most paths.
  if (fullpath.length() < 9) { return false; }  // not long enough to match
  if (fullpath.substr(0, 9) != STRING16(L"/-gears-/")) { return false; }

  std::string16 path = fullpath.substr(9);
  if (path.empty()) { path = STRING16(L"index.html"); }

  // Uncomment this block to modify and test inspector resources more easily.
  // This code first looks for '/-gears-/foo' at 'INSTALLDIR/inspector/foo'.
  // If that file is not found, it falls back to the bundled resource.
  /*
  std::string16 local_file;
  if (GetInstallDirectory(&local_file)) {
    local_file += STRING16(L"/inspector/");
    local_file += path;
#if defined(WIN32)  // kPathSeparator doesn't work -- char16 vs char16[]
    ReplaceAll(local_file, std::string16(L"/"), std::string16(L"\\"));
#endif
    if (File::Exists(local_file.c_str())) {
      // Ignore the mem leak here. This code is never checked-in uncommented.
      std::vector<uint8> *data_alloc = new std::vector<uint8>;
      if (data_alloc &&
          File::ReadFileToVector(local_file.c_str(), data_alloc)) {
        *data = &(*data_alloc)[0];
        *size = (*data_alloc).size();
        headers->clear();
        return true;
      }
    }
  }
  */

  // TODO(cprince): Fix the linear scan if we ever have so many Inspector
  // resources that it matters.  (It's too bad you can't statically initialize
  // a std::map at compile time.)
  int i;
  for (i = 0; i < resource_list_count; ++i) {
    if (path == resource_list[i].path) { break; }
  }
  if (i >= resource_list_count) { return false; }

  *data = resource_list[i].data;
  *size = resource_list[i].size;
  *headers = HttpConstants::kContentTypeHeader;
  *headers += STRING16(L": ");
  *headers += resource_list[i].content_type;
  *headers += HttpConstants::kCrLf;
  *headers += HttpConstants::kCrLf;
  return true;
}


// Data declarations and tables.

extern const unsigned char inspector_index_html[];
extern const size_t        inspector_index_html_size;

extern const unsigned char inspector_console_html[];
extern const size_t        inspector_console_html_size;
extern const unsigned char inspector_database_html[];
extern const size_t        inspector_database_html_size;
extern const unsigned char inspector_localserver_html[];
extern const size_t        inspector_localserver_html_size;

extern const unsigned char inspector_common_alert_35_png[];
extern const size_t        inspector_common_alert_35_png_size;
extern const unsigned char inspector_common_database_gif[];
extern const size_t        inspector_common_database_gif_size;
extern const unsigned char inspector_common_error_35_png[];
extern const size_t        inspector_common_error_35_png_size;
extern const unsigned char inspector_common_ie6hacks_css[];
extern const size_t        inspector_common_ie6hacks_css_size;
extern const unsigned char inspector_common_inspector_links_js[];
extern const size_t        inspector_common_inspector_links_js_size;
extern const unsigned char inspector_common_lightbulb_35_png[];
extern const size_t        inspector_common_lightbulb_35_png_size;
extern const unsigned char inspector_common_localserver_gif[];
extern const size_t        inspector_common_localserver_gif_size;
extern const unsigned char inspector_common_question_35_png[];
extern const size_t        inspector_common_question_35_png_size;
extern const unsigned char inspector_common_styles_css[];
extern const size_t        inspector_common_styles_css_size;
extern const unsigned char inspector_common_workerpool_gif[];
extern const size_t        inspector_common_workerpool_gif_size;

extern const unsigned char sdk_gears_init_js[];
extern const size_t        sdk_gears_init_js_size;
extern const unsigned char ui_common_base_js[];
extern const size_t        ui_common_base_js_size;
extern const unsigned char ui_common_dom_js[];
extern const size_t        ui_common_dom_js_size;
extern const unsigned char ui_common_icon_32x32_png[];
extern const size_t        ui_common_icon_32x32_png_size;


const char16 *kContentTypeCSS  = STRING16(L"text/css");
const char16 *kContentTypeGIF  = STRING16(L"image/gif");
const char16 *kContentTypeHTML = STRING16(L"text/html");
const char16 *kContentTypeJS   = STRING16(L"application/javascript");
const char16 *kContentTypePNG  = STRING16(L"image/png");

const InspectorResource resource_list[] = {
  { STRING16(L"console.html"),
    inspector_console_html,
    inspector_console_html_size,
    kContentTypeHTML },
  { STRING16(L"database.html"),
    inspector_database_html,
    inspector_database_html_size,
    kContentTypeHTML },
  { STRING16(L"index.html"),
    inspector_index_html,
    inspector_index_html_size,
    kContentTypeHTML },
  { STRING16(L"localserver.html"),
    inspector_localserver_html,
    inspector_localserver_html_size,
    kContentTypeHTML },

  { STRING16(L"common/alert-35.png"),
    inspector_common_alert_35_png,
    inspector_common_alert_35_png_size,
    kContentTypePNG },
  { STRING16(L"common/base.js"),
    ui_common_base_js,
    ui_common_base_js_size,
    kContentTypeJS },
  { STRING16(L"common/database.gif"),
    inspector_common_database_gif,
    inspector_common_database_gif_size,
    kContentTypeGIF },
  { STRING16(L"common/dom.js"),
    ui_common_dom_js,
    ui_common_dom_js_size,
    kContentTypeJS },
  { STRING16(L"common/error-35.png"),
    inspector_common_error_35_png,
    inspector_common_error_35_png_size,
    kContentTypePNG },
  { STRING16(L"common/gears_init.js"),
    sdk_gears_init_js,
    sdk_gears_init_js_size,
    kContentTypeJS },
  { STRING16(L"common/icon_32x32.png"),
    ui_common_icon_32x32_png,
    ui_common_icon_32x32_png_size,
    kContentTypePNG },
  { STRING16(L"common/ie6hacks.css"),
    inspector_common_ie6hacks_css,
    inspector_common_ie6hacks_css_size,
    kContentTypeCSS },
  { STRING16(L"common/inspector_links.js"),
    inspector_common_inspector_links_js,
    inspector_common_inspector_links_js_size,
    kContentTypeJS },
  { STRING16(L"common/lightbulb-35.png"),
    inspector_common_lightbulb_35_png,
    inspector_common_lightbulb_35_png_size,
    kContentTypePNG },
  { STRING16(L"common/localserver.gif"),
    inspector_common_localserver_gif,
    inspector_common_localserver_gif_size,
    kContentTypeGIF },
  { STRING16(L"common/question-35.png"),
    inspector_common_question_35_png,
    inspector_common_question_35_png_size,
    kContentTypePNG },
  { STRING16(L"common/styles.css"),
    inspector_common_styles_css,
    inspector_common_styles_css_size,
    kContentTypeCSS },
  { STRING16(L"common/workerpool.gif"),
    inspector_common_workerpool_gif,
    inspector_common_workerpool_gif_size,
    kContentTypeGIF },
};
const int resource_list_count = ARRAYSIZE(resource_list);
