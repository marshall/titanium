/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef CURL_LOCAL_HANDLER_H_
#define CURL_LOCAL_HANDLER_H_

struct Curl_local_handler
{
	const char *protocol;
	const char * (*get_absolute_path)(const char *fullpath);
};

#endif
