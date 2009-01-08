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
#ifndef TI_APP_CONFIG_H_
#define TI_APP_CONFIG_H_

#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
#include <sstream>

#include "../base.h"
#include "ti_window_config.h"

#define TITRUE 1
#define TIFALSE 0
#define nodeNameEquals(n,s) (xmlStrcmp(n->name, (const xmlChar *)s) == 0)
#define nodeValue(n) ((const char *)xmlNodeListGetString(n->doc, n->children, TITRUE))
#define boolValue(n) (TiAppConfig::StringToBool(nodeValue(n)))

namespace ti {

class WindowConfig;

typedef std::vector<WindowConfig*> WindowConfigList ;

class TITANIUM_API AppConfig
{
private:
	const char* error;
	std::string appName, appID, description, copyright, homepage, version, updateSite;
	WindowConfigList windows;

	// icon properties
	std::string icon16, icon32, icon48;
	static AppConfig *instance_;

	AppConfig(std::string& xmlfile);

public:
	~AppConfig();

	static bool StringToBool (const char * str);

	std::string& GetAppName() { return appName; }
	std::string& GetAppID() { return appID; }
	std::string& GetDescription() { return description; }
	std::string& GetCopyright() { return copyright; }
	std::string& GetHomepage() { return homepage; }
	std::string& GetVersion() { return version; }
	std::string& GetUpdateSite() { return updateSite; }

	WindowConfigList& GetWindows() { return windows; }
	WindowConfig* GetWindow(std::string &id);
	WindowConfig* GetMainWindow();

	//icon accessors
	std::string& GetIcon16() { return icon16; }
	std::string& GetIcon32() { return icon32; }
	std::string& GetIcon48() { return icon48; }

	const char* GetError() { return error; }

	static AppConfig* Instance() {
		return instance_;
	}

	static AppConfig* Init(std::string& xmlFile) {
		if (instance_ == NULL) {
			instance_ = new AppConfig(xmlFile);
		}
		return instance_;
	}
};

}
#endif
