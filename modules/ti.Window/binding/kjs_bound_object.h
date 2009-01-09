/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KJS_BOUND_OBJECT_H_
#define _KJS_BOUND_OBJECT_H_

#include "kjs.h"

#include <vector>
#include <string>
#include <map>

class KJSBoundObject : public kroll::BoundObject
{
public:
	KJSBoundObject(JSContextRef context, JSObjectRef js_object);
	~KJSBoundObject();

	void Set(const char *name, kroll::Value* value, kroll::BoundObject *context);
	kroll::Value* Get(const char *name, kroll::BoundObject *context);
	std::vector<std::string> GetPropertyNames();

	JSObjectRef GetJSObject();

protected:
	JSContextRef context;
	JSObjectRef object;

};

#endif
