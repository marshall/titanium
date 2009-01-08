/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KJS_BOUND_METHOD_H_
#define _KJS_BOUND_METHOD_H_

#include "kjs.h"

#include <vector>
#include <string>
#include <map>

class KJSBoundMethod : public TiBoundMethod
{

public:

	KJSBoundMethod(JSContextRef context, JSObjectRef js_object, JSObjectRef this_obj);
	~KJSBoundMethod();

	void Set(const char *name, TiValue* value, TiBoundObject *context);
	TiValue* Get(const char *name, TiBoundObject *context);
	TiValue* Call(const TiValueList& args, TiBoundObject* context);
	std::vector<std::string> GetPropertyNames();
	JSObjectRef GetJSObject();

protected:
	JSContextRef context;
	JSObjectRef object;
	JSObjectRef this_obj;
	KJSBoundObject* kjs_bound_object;


};

#endif
