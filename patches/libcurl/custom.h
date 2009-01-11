/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef CURL_CUSTOM_URL_H_
#define CURL_CUSTOM_URL_H_

extern const struct Curl_handler Curl_handler_custom;
extern struct Curl_local_handler *local_handlers[];
extern int local_handlers_size;

int is_local_handler(const char *proto);
struct Curl_local_handler * get_local_handler(const char *proto);

#endif
