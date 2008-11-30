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
#ifndef TI_DEV_MODE_H_
#define TI_DEV_MODE_H_

#include <string>
#include <map>

class TiDevMode
{
private:
	static TiDevMode* _instance;
	
public:
	static TiDevMode* instance() {
		if (_instance == NULL)
			_instance = new TiDevMode();
		return _instance;
	}

	static bool isDevMode;

	TiDevMode() { }

	std::wstring projectPath;
	std::wstring runtimePath;
	std::map<std::wstring, std::wstring> pluginPaths;
};


#endif