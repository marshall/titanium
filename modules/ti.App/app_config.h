/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef TI_APP_CONFIG_H_
#define TI_APP_CONFIG_H_

#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
#include <sstream>

#include "app_api.h"
#include "window_config.h"

#define TITRUE 1
#define TIFALSE 0
#define nodeNameEquals(n,s) (xmlStrcmp(n->name, (const xmlChar *)s) == 0)
#define nodeValue(n) ((const char *)xmlNodeListGetString(n->doc, n->children, TITRUE))
#define boolValue(n) (AppConfig::StringToBool(nodeValue(n)))

namespace ti {

class WindowConfig;

typedef std::vector<WindowConfig*> WindowConfigList ;

class TITANIUM_APP_API AppConfig
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
	WindowConfig* GetWindowByURL(std::string url);
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
