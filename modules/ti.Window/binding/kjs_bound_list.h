/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KJS_BOUND_LIST_H_
#define _KJS_BOUND_LIST_H_

#include "kjs.h"

#include <vector>
#include <string>
#include <map>
#include <cmath>

class KJSBoundList : public kroll::BoundList
{

public:

	KJSBoundList(JSContextRef context, JSObjectRef js_object);
	~KJSBoundList();


	void Set(const char *name, kroll::Value* value);
	kroll::Value* Get(const char *name);
	std::vector<std::string> GetPropertyNames();

	void Append(kroll::Value* value);
	int Size();
	kroll::Value* At(int index);

	JSObjectRef GetJSObject();

protected:
	JSContextRef context;
	JSObjectRef object;
	KJSBoundObject* kjs_bound_object;

	static char* IntToChars(int value);
};

#endif
