/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 *
 * A custom URL handler for app:// URLs
 */

#include "app_url.h"

#ifdef OS_WIN32
#include <windows.h>
#endif

#ifdef OS_LINUX
#include <cstdlib>
#include <string>
#include <cstring>
#endif

#define BUFSIZE 4096

struct Curl_local_handler Titanium_app_url_handler = {
	"app",
	ti::AppURLGetAbsolutePath
};

namespace ti {
	static const char *kAppURLPrefix = "/Resources";


	/* TODO: Memory leak here */
	const char * AppURLGetAbsolutePath(const char *full_path)
	{
#ifdef OS_WIN32
		DWORD length = 0;
		LPTSTR kr_home = (LPTSTR) malloc(BUFSIZE*sizeof(TCHAR));
		length = GetEnvironmentVariable("KR_HOME", kr_home, BUFSIZE);
		length += strlen(full_path) + strlen(kAppURLPrefix) + 1;

		char *app_resource_path = (char *) calloc(length + 1, sizeof(char));

		strcat(app_resource_path, kr_home);
		strcat(app_resource_path, kAppURLPrefix);
		strcat(app_resource_path, "/");
		strcat(app_resource_path, full_path);

		return app_resource_path;
#endif

#ifdef OS_LINUX
		char* kr_home = getenv("KR_HOME");
		std::string app_resources_path(kr_home);
		app_resources_path += std::string(kAppURLPrefix) + "/" + std::string(full_path);
		return strdup(app_resources_path.c_str());
#endif

	}
}
