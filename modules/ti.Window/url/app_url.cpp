/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 *
 * A custom URL handler for app:// URLs
 */

#include "app_url.h"
#include <Poco/Environment.h>
#include <string>
#include <cstring>

struct Curl_local_handler Titanium_app_url_handler = {
	"app",
	ti::AppURLGetAbsolutePath
};

namespace ti {

	static const char *kAppURLPrefix = "/Resources";
	/* TODO: Memory leak here */
	const char * AppURLGetAbsolutePath(const char *url)
	{
		std::string path = Poco::Environment::get("KR_HOME", "");
		path = path + kAppURLPrefix + "/" + url;
		return strdup(path.c_str());
	}
}
