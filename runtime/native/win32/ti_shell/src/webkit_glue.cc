#include <windows.h>
#include <atlbase.h>
#include <commdlg.h>
#include <objbase.h>
#include <shlwapi.h>
#include <wininet.h>

#include "webkit/glue/webkit_glue.h"
#include "webkit/glue/plugins/plugin_list.h"
#include "base/path_service.h"
#include "base/file_util.h"
#include "googleurl/src/gurl.h"
#include "googleurl/src/url_util.h"
#include "base/logging.h"
#include "base/win_util.h"
#include "net/base/mime_util.h"

namespace webkit_glue {

void PrefetchDns(const std::string& hostname) {}

void PrecacheUrl(const char16* url, int url_length) {}

void AppendToLog(const char* file, int line, const char* msg) {
  logging::LogMessage(file, line).stream() << msg;
}

bool GetMimeTypeFromExtension(const std::wstring &ext, std::string *mime_type) {
  return net::GetMimeTypeFromExtension(ext, mime_type);
}

bool GetMimeTypeFromFile(const std::wstring &file_path,
                         std::string *mime_type) {
  return net::GetMimeTypeFromFile(file_path, mime_type);
}

bool GetPreferredExtensionForMimeType(const std::string& mime_type,
                                      std::wstring* ext) {
  return net::GetPreferredExtensionForMimeType(mime_type, ext);
}

IMLangFontLink2* GetLangFontLink() {
  return webkit_glue::GetLangFontLinkHelper();
}

std::wstring GetLocalizedString(int message_id) {
  const ATLSTRINGRESOURCEIMAGE* image =
      AtlGetStringResourceImage(_AtlBaseModule.GetModuleInstance(),
                                message_id);
  if (!image) {
    NOTREACHED();
    return L"No string for this identifier!";
  }
  return std::wstring(image->achString, image->nLength);
}

std::string GetDataResource(int resource_id) {
	return std::string();
	/*
	if (resource_id == IDR_BROKENIMAGE) {
    // Use webkit's broken image icon (16x16)
    static std::string broken_image_data;
    if (broken_image_data.empty()) {
      std::wstring path;
      PathService::Get(base::DIR_SOURCE_ROOT, &path);
      file_util::AppendToPath(&path, L"webkit");
      file_util::AppendToPath(&path, L"tools");
      file_util::AppendToPath(&path, L"test_shell");
      file_util::AppendToPath(&path, L"resources");
      file_util::AppendToPath(&path, L"missingImage.gif");
      bool success = file_util::ReadFileToString(path, &broken_image_data);
      if (!success) {
        LOG(FATAL) << "Failed reading: " << path;
      }
    }
    return broken_image_data;
  } else if (resource_id == IDR_FEED_PREVIEW) {
    // It is necessary to return a feed preview template that contains
    // a {{URL}} substring where the feed URL should go; see the code 
    // that computes feed previews in feed_preview.cc:MakeFeedPreview. 
    // This fixes issue #932714.    
    return std::string("Feed preview for {{URL}}");
  } else {
    return std::string();
  }*/
}

HCURSOR LoadCursor(int cursor_id) {
  return NULL;
}

SkBitmap* GetBitmapResource(int resource_id) {
  return NULL;
}

bool GetApplicationDirectory(std::wstring *path) {
  return PathService::Get(base::DIR_EXE, path);
}

GURL GetInspectorURL() {
  return GURL("test-shell-resource://inspector/inspector.html");
}

std::string GetUIResourceProtocol() {
  return "test-shell-resource";
}

bool GetExeDirectory(std::wstring *path) {
  return PathService::Get(base::DIR_EXE, path);
}

bool SpellCheckWord(const wchar_t* word, int word_len,
                    int* misspelling_start, int* misspelling_len) {
  // Report all words being correctly spelled.
  *misspelling_start = 0;
  *misspelling_len = 0;
  return true;
}

bool GetPlugins(bool refresh, std::vector<WebPluginInfo>* plugins) {
  return NPAPI::PluginList::Singleton()->GetPlugins(refresh, plugins);
}

bool webkit_glue::IsPluginRunningInRendererProcess() {
  return true;
}

bool EnsureFontLoaded(HFONT font) {
  return true;
}

MONITORINFOEX GetMonitorInfoForWindow(HWND window) {
  return webkit_glue::GetMonitorInfoForWindowHelper(window);
}

bool DownloadUrl(const std::string& url, HWND caller_window) {
  return false;
}

bool GetPluginFinderURL(std::string* plugin_finder_url) {
  return false;
}

bool IsDefaultPluginEnabled() {
  return false;
}

std::wstring GetWebKitLocale() {
  return L"en-US";
}

} 