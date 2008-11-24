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

#include "ti_window.h"

#define nodeNameEquals(n,s) (xmlStrcmp(n->name, (const xmlChar *)s) == 0)
#define nodeValue(n) ((const char *)xmlNodeListGetString(n->doc, n->children, TRUE))
#define boolValue(n) (TiAppConfig::stringToBool(nodeValue(n)))

class TiWindow;

typedef std::vector<TiWindow*> TiWindowList ;

class TiAppConfig 
{
private:
	std::string appName, description, copyright, homepage, version;
	TiWindowList windows;

	// icon properties
	std::string icon16, icon32, icon48;

public:
	TiAppConfig(std::wstring& xmlfile);
	~TiAppConfig();

	static bool stringToBool (const char * str);

	std::string& getAppName() { return appName; }
	std::string& getDescription() { return description; }
	std::string& getCopyright() { return copyright; }
	std::string& getHomepage() { return homepage; }
	std::string& getVersion() { return version; }

	TiWindowList& getWindows() { return windows; }
	TiWindow* getWindow(std::string &id);
	TiWindow* getMainWindow();
	
	//icon accessors
	std::string& getIcon16() { return icon16; }
	std::string& getIcon32() { return icon32; }
	std::string& getIcon48() { return icon48; }

	std::wstring getResourcePath();

};

#endif
