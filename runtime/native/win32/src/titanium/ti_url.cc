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
#include "ti_web_shell.h"

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
bool TiURL::isHost(std::string &host)
{
	// GURL likes to append onto the top level resource as if it's a host.. we can "fix" that
	// there might be a better way to do this?
		
	return (!stringEndsWith(host, ".html")
			&& !stringEndsWith(host, ".xml"));
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
			if (TiURL::isHost(url.host()) || url.path() == "/") {
				file_util::AppendToPath(&path, UTF8ToWide(url.host()));
			}

			if (url.path() != "/") {
				std::string urlPath = url.path().substr(1);
				stringReplaceAll(&urlPath, "/", "\\");

				file_util::AppendToPath(&path, UTF8ToWide(urlPath));
			}
		}

		printf("%s url (%s host, %s path) => %ls\n", url.spec().c_str(), url.host().c_str(), url.path().c_str(), path.c_str());
		return path;
	} else if (url.scheme() == APP_SCHEME) {
		std::wstring path = base_dir;
		file_util::AppendToPath(&path, L"Resources");

		if (TiURL::isHost(url.host()) || url.path() == "/") {
			file_util::AppendToPath(&path, UTF8ToWide(url.host()));
		}

		if (url.path() != "/") {
			std::string urlPath = url.path().substr(1);
			stringReplaceAll(&urlPath, "/", "\\");

			file_util::AppendToPath(&path, UTF8ToWide(urlPath));
		}
		
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

	std::string GetContentType(std::wstring& ext) {
		if (ext == L"js") { return "text/javascript"; }
		else if (ext == L"html" || ext == L"htm" ) { return "text/html"; }
		else if (ext == L"xhtml") { return "text/xhtml"; }
		else return "text/plain";
	}

	// simulate an HTTP response so we can return a response code
	// -- this allows app:// etc resources to be requested/executed dynamically
	// -- in frameworks like jQuery
	virtual void GetResponseInfo(net::HttpResponseInfo *info)
	{
		std::wstring ext = file_util::GetFileExtensionFromPath(file_path_);
		int64 filesize;
		file_util::GetFileSize(file_path_, &filesize);
		std::string raw_headers = "HTTP/1.1 200 OK\n";
		raw_headers += "Content-Type: " + GetContentType(ext) + "\n";
		raw_headers += "Content-Length: " + Int64ToString(filesize) + "\n";

		// ParseRawHeaders expects \0 to end each header line.
		ReplaceSubstringsAfterOffset(&raw_headers, 0, "\n", std::string("\0", 1));
		info->headers = new net::HttpResponseHeaders(raw_headers);
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
