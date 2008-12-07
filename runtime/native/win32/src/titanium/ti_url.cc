/*
* Copyright 2006-2008 Appcelerator, Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#include "ti_url.h"
#include "ti_chrome_window.h"
#include "ti_version.h"

#include "webkit/glue/webframeloader.h"
#include "base/string_util.h"

#define TI_SCHEME "ti"
#define APP_SCHEME "app"
// an internal only scheme for the inspector
#define TI_RESOURCE_SCHEME "ti-resource"

std::vector<std::string> TiURL::tiResourceTypes = TiURL::setupTiResourceTypes();

void replaceSlashes (std::string* path)
{
	size_t pos = 0;
	while ((pos = path->find("/", pos)) != std::string::npos)
	{
		path->replace(pos, 1, "\\");
	}
}

int wildcmp(const char *wild, const char *string) {
  const char *cp = NULL, *mp = NULL;

  while ((*string) && (*wild != '*')) {
    if ((*wild != *string) && (*wild != '?')) {
      return 0;
    }
    wild++;
    string++;
  }

  while (*string) {
    if (*wild == '*') {
      if (!*++wild) {
        return 1;
      }
      mp = wild;
      cp = string+1;
    } else if ((*wild == *string) || (*wild == '?')) {
      wild++;
      string++;
    } else {
      wild = mp;
      string = cp++;
    }
  }

  while (*wild == '*') {
    wild++;
  }
  return !*wild;
}

void stringReplaceAll(std::string *string, std::string pattern, std::string replacement)
{
	size_t pos = 0;
	while ((pos = string->find(pattern, pos)) != std::string::npos)
	{
		string->replace(pos, pattern.length(), replacement);
	}
}

bool stringEndsWith(std::string &string, std::string substr)
{
	size_t pos = string.rfind(substr);
	return (pos != std::string::npos) && (pos == (string.length() - substr.length()));
}

/*static*/
bool TiURL::urlMatchesPattern(GURL& url, std::string& pattern)
{
	if (url.spec().length() <= 0) {
		return false;
	}

	//remove the trailing slash
	std::string spec = url.spec();
	if (spec.at(spec.length()-1) == '/') {
		spec = spec.substr(0, spec.length() - 1);
	}

	if (spec.find("?") != std::string::npos) {
		spec = spec.substr(0, spec.find("?"));
	}

	
	/*if (LowerCaseEqualsASCII(TiAppConfig::instance()->getAppID(), url.host().c_str())) {
		std::string newURL = spec;
		std::string appId = TiAppConfig::instance()->getAppID();
		std::transform(appId.begin(), appId.end(), appId.begin(), (int(*)(int)) tolower);

		stringReplaceAll(&newURL, appId+"/", "");

		if (wildcmp(pattern.c_str(), newURL.c_str()))
			return true;
		return false;
	}*/

	if (wildcmp(pattern.c_str(), spec.c_str()))
		return true;

	return false;
}

void appendURLPath(std::wstring *path, GURL& url)
{
	if (!LowerCaseEqualsASCII(TiAppConfig::instance()->getAppID(), url.host().c_str())) {
		file_util::AppendToPath(path, UTF8ToWide(url.host()));
	}

	if (url.path() != "/") {
		std::string urlPath = url.path().substr(1);
		stringReplaceAll(&urlPath, "/", "\\");

		file_util::AppendToPath(path, UTF8ToWide(urlPath));
	}
}


/*static*/
std::wstring TiURL::getPathForURL(GURL& url)
{
	std::wstring base_dir;
	PathService::Get(base::DIR_EXE, &base_dir);
	
	if (url.scheme() == TI_SCHEME) {
		std::wstring path = base_dir;
		file_util::AppendToPath(&path, L"Resources");
		file_util::AppendToPath(&path, L"titanium");

		if (url.host() == "plugin") {
			file_util::AppendToPath(&path, UTF8ToWide(url.path()));
		}
		else {
			appendURLPath(&path, url);
		}

		printf("%s => %ls\n", url.spec().c_str(), path.c_str());
		return path;
	} else if (url.scheme() == APP_SCHEME) {
		std::wstring path = base_dir;
		file_util::AppendToPath(&path, L"Resources");

		appendURLPath(&path, url);
		
		printf("%s => %ls\n", url.spec().c_str(), path.c_str());
		return path;
	} else if (url.scheme() == TI_RESOURCE_SCHEME) {
		std::wstring path;
		
		PathService::Get(TiVersion::kRuntimeResources, &path);
		appendURLPath(&path, url);

		printf("%s => %ls\n", url.spec().c_str(), path.c_str());
		return path;
	}

	return std::wstring(L"");
}

class TiURLRequestFileJob : public URLRequestFileJob
{
public:
	TiURLRequestFileJob(URLRequest *request, std::wstring path) : URLRequestFileJob(request) {
		file_path_ = path;
	}

	// simulate an HTTP response so we can return a response code
	// -- this allows app:// etc resources to be requested/executed dynamically
	// -- in frameworks like jQuery
	virtual void GetResponseInfo(net::HttpResponseInfo *info)
	{
		std::wstring ext = file_util::GetFileExtensionFromPath(file_path_);
		int64 filesize;
		file_util::GetFileSize(file_path_, &filesize);
		std::string mimeType;
		GetMimeType(&mimeType);

		std::string raw_headers = "HTTP/1.1 200 OK\n";
		raw_headers += "Content-Type: " + mimeType + "\n";
		raw_headers += "Content-Length: " + Int64ToString(filesize) + "\n";

		// ParseRawHeaders expects \0 to end each header line.
		ReplaceSubstringsAfterOffset(&raw_headers, 0, "\n", std::string("\0", 1));
		info->headers = new net::HttpResponseHeaders(raw_headers);
	}

	~TiURLRequestFileJob(){}
};

/*static*/
URLRequestJob* TiURL::createTiResourceRequestJob(URLRequest *urlRequest, std::string& resourceType, GURL& url)
{
	if (resourceType == "notification") {
		std::wstring path;
		PathService::Get(base::DIR_EXE, &path);
		file_util::AppendToPath(&path, L"Resources");
		file_util::AppendToPath(&path, L"titanium");
		file_util::AppendToPath(&path, L"notification.html");

		return new TiURLRequestFileJob(urlRequest, path);
	}

	return NULL;
}

/*static*/
URLRequestJob* TiURL::createURLRequestJob(URLRequest* request, const std::string& scheme)
{
	GURL url = static_cast<GURL>(request->url());
	if (url.SchemeIs(TI_SCHEME)) {
		if (std::find(tiResourceTypes.begin(), tiResourceTypes.end(), url.host()) != tiResourceTypes.end()) {
			URLRequestJob *job = createTiResourceRequestJob(request, url.host(), url);
			if (job != NULL) {
				return job;
			}
		}
	}

	std::wstring path = getPathForURL(static_cast<GURL>(request->url()));
	if (path.size() > 0) {
		TiURLRequestFileJob *job = new TiURLRequestFileJob(request, path);
		return job;
	}

	// we shouldn't get here?
	return NULL;
}

/*static*/
void TiURL::init ()
{
	url_util::AddStandardScheme(TI_SCHEME);
	url_util::AddStandardScheme(APP_SCHEME);
	url_util::AddStandardScheme(TI_RESOURCE_SCHEME);

	URLRequest::RegisterProtocolFactory(TI_SCHEME, &TiURL::createURLRequestJob);
	URLRequest::RegisterProtocolFactory(APP_SCHEME, &TiURL::createURLRequestJob);
	URLRequest::RegisterProtocolFactory(TI_RESOURCE_SCHEME, &TiURL::createURLRequestJob);

	WebFrameLoader::RegisterURLSchemeAsLocal(TI_SCHEME);
	WebFrameLoader::RegisterURLSchemeAsLocal(APP_SCHEME);
}
