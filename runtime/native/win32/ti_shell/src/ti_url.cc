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

#define RESOURCE_SCHEME "resource:///"
#define TI_SCHEME "ti:///"
/*static*/
std::string TiURL::absolutePathForURL(TiAppConfig *appConfig, std::string url)
{
	std::wstring base_dir;
	PathService::Get(base::DIR_EXE, &base_dir);

	if (url.find(RESOURCE_SCHEME) != std::string::npos) {
		std::wstring resource = base_dir;
		file_util::AppendToPath(&resource, L"Resources");
		std::string path = url.substr(strlen(RESOURCE_SCHEME));
		replaceSlashes(&path);

		file_util::AppendToPath(&resource, UTF8ToWide(path));
		return WideToUTF8(resource);
	}
	else if (url.find(TI_SCHEME) != std::string::npos) {
		std::wstring ti_internal = base_dir;
		file_util::AppendToPath(&ti_internal, L"Resources");
		file_util::AppendToPath(&ti_internal, L"titanium");
		std::string path = url.substr(strlen(TI_SCHEME));
		replaceSlashes(&path);

		file_util::AppendToPath(&ti_internal, UTF8ToWide(path));
		return WideToUTF8(ti_internal);
	}
}