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

void replaceSlashes (std::string* path)
{
	size_t pos = 0;
	while ((pos = path->find("/", pos)) != std::string::npos)
	{
		path->replace(pos, 1, "\\");
	}
}

#define TI_SCHEME "ti"
#define APP_SCHEME "app"

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
/*static*/

bool TiURL::urlMatchesPattern(GURL& url, std::string& pattern)
{
	//remove the trailing slash
	std::string spec = url.spec();
	if (spec.at(spec.length()-1) == '/') {
		spec = spec.substr(0, spec.length() - 1);
	}

	if (wildcmp(pattern.c_str(), spec.c_str()))
		return true;

	return false;
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
			file_util::AppendToPath(&path, UTF8ToWide(url.host()));
			if (url.path() != "/")
				file_util::AppendToPath(&path, UTF8ToWide(url.path()));
		}

		printf("%s url (%s host, %s path) => %ls\n", url.spec().c_str(), url.host().c_str(), url.path().c_str(), path.c_str());
		return path;
	} else if (url.scheme() == APP_SCHEME) {
		std::wstring path = base_dir;
		file_util::AppendToPath(&path, L"Resources");
		file_util::AppendToPath(&path, UTF8ToWide(url.host()));

		if (url.path() != "/")
			file_util::AppendToPath(&path, UTF8ToWide(url.path()));
		
		printf("%s url (%s host, %s path) => %ls\n", url.spec().c_str(), url.host().c_str(), url.path().c_str(), path.c_str());
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
	~TiURLRequestFileJob(){}
};

/*static*/
URLRequestJob* TiURL::createURLRequestJob(URLRequest* request, const std::string& scheme)
{
	std::wstring path = getPathForURL(static_cast<GURL>(request->url()));
	if (path.size() > 0) {
		return new TiURLRequestFileJob(request, path);
	}

	// we shouldn't get here?
	return NULL;
}

/*static*/
void TiURL::init ()
{
	url_util::AddStandardScheme(TI_SCHEME);
	url_util::AddStandardScheme(APP_SCHEME);

	URLRequest::RegisterProtocolFactory(TI_SCHEME, &TiURL::createURLRequestJob);
	URLRequest::RegisterProtocolFactory(APP_SCHEME, &TiURL::createURLRequestJob);
}
