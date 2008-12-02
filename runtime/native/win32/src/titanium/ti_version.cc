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

#include "ti_version.h"
#include "base/string_util.h"
#include <shlobj.h>

#include <cstdlib>

namespace TiVersion {

	const int kRuntime = 15001;
	const int kRuntimeResources = 15002;

	bool getPath(int pathId, std::wstring *result)
	{
		if (pathId == kRuntime) {
#ifdef TITANIUM_DEBUGGING
			// Return the current directory
			PathService::Get(base::DIR_EXE, result);
			return true;
#else
			char *homePath;
			size_t size;
			_dupenv_s(&homePath, &size, "USERPROFILE");

			std::wstring path;
			
			if (homePath != NULL) {
				path = UTF8ToWide(homePath);
				file_util::AppendToPath(&path, L".appcelerator");
			}

			if (!file_util::PathExists(path)) {
				// This could be cygwin, try using the HOME env var
				_dupenv_s(&homePath, &size, "HOME");

				if (homePath == NULL) {
					return false;
				}

				path = UTF8ToWide(homePath);
				file_util::AppendToPath(&path, L".appcelerator");

				if (!file_util::PathExists(path)) {
					// ok, dir just doesn't exist
					return false;
				}
			}

			file_util::AppendToPath(&path, L"releases");
			file_util::AppendToPath(&path, L"titanium");
			file_util::AppendToPath(&path, L"win32");
			file_util::AppendToPath(&path, TI_VERSION_W);
			
			(*result) = path;
			return true;
#endif
		}
		else if (pathId == kRuntimeResources) {
			std::wstring runtime;
			if (PathService::Get(kRuntime, &runtime)) {
				file_util::AppendToPath(&runtime, L"Resources");
				(*result) = runtime;
				return true;
			}
			return false;
		}
		return false;
	}

	void init()
	{
		PathService::RegisterProvider(&TiVersion::getPath, TiVersion::kRuntime, TiVersion::kRuntimeResources);
	}

}