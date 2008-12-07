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
#include <vector>
#include <algorithm>

#include "libxml/parser.h"
#include "libxml/tree.h"
#include "libxml/xpath.h"

#include "base/string_util.h"
#include "base/file_util.h"
#include "base/path_service.h"

#include "ti_window_config.h"

#define nodeNameEquals(n,s) (xmlStrcmp(n->name, (const xmlChar *)s) == 0)
#define nodeValue(n) ((const char *)xmlNodeListGetString(n->doc, n->children, TRUE))
#define boolValue(n) (TiAppConfig::stringToBool(nodeValue(n)))

class TiWindowConfig;

typedef std::vector<TiWindowConfig*> TiWindowConfigList ;

class TiAppConfig 
{
private:
	const char* error;
	std::string appName, appID, description, copyright, homepage, version, updateSite;
	TiWindowConfigList windows;

	// icon properties
	std::string icon16, icon32, icon48;		
	static TiAppConfig *instance_;

	TiAppConfig(std::wstring& xmlfile);

public:
	~TiAppConfig();

	static bool stringToBool (const char * str);

	std::string& getAppName() { return appName; }
	std::string& getAppID() { return appID; }
	std::string& getDescription() { return description; }
	std::string& getCopyright() { return copyright; }
	std::string& getHomepage() { return homepage; }
	std::string& getVersion() { return version; }
	std::string& getUpdateSite() { return updateSite; }

	TiWindowConfigList& getWindows() { return windows; }
	TiWindowConfig* getWindow(std::string &id);
	TiWindowConfig* getMainWindow();
	
	//icon accessors
	std::string& getIcon16() { return icon16; }
	std::string& getIcon32() { return icon32; }
	std::string& getIcon48() { return icon48; }

	std::wstring getResourcePath();
	const char* getError() { return error; }

	static TiAppConfig* instance() {
		return instance_;
	}

	static TiAppConfig* init(std::wstring& xmlFile) {
		if (instance_ == NULL) {
			instance_ = new TiAppConfig(xmlFile);
		}
		return instance_;
	}
};

#endif
