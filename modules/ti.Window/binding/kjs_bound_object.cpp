/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "kjs.h"

KJSBoundObject::KJSBoundObject(JSContextRef context, JSObjectRef js_object)
	: context(context), object(js_object)
{
	JSValueProtect(context, js_object);
}

KJSBoundObject::~KJSBoundObject()
{
	JSValueUnprotect(this->context, this->object);
}

JSObjectRef KJSBoundObject::GetJSObject()
{
	return this->object;
}

kroll::Value* KJSBoundObject::Get(const char *name, kroll::BoundObject *context)
{
	JSStringRef s = JSStringCreateWithUTF8CString(name);
	JSValueRef exception = NULL;
	JSValueRef js_value =
	    JSObjectGetProperty(this->context,
	                        this->object,
	                        s,
	                        NULL);
	JSStringRelease(s);

	if (exception != NULL) //exception thrown
	{
		kroll::Value* tv_exp = JSValueToKrollValue(this->context, exception, NULL);
		throw tv_exp;
	}

	return JSValueToKrollValue(this->context, js_value, this->object);
}

void KJSBoundObject::Set(const char *name, kroll::Value* value, kroll::BoundObject *context)
{
	JSValueRef js_value = KrollValueToJSValue(this->context, value);
	JSStringRef s = JSStringCreateWithUTF8CString(name);

	JSValueRef exception = NULL;
	JSObjectSetProperty(this->context,
	                    this->object,
	                    s,
	                    js_value,
	                    NULL, // attributes
	                    &exception);
	JSStringRelease(s);

	if (exception != NULL) //exception thrown
	{
		kroll::Value* tv_exp = JSValueToKrollValue(this->context, exception, NULL);
		throw tv_exp;
	}
}

std::vector<std::string> KJSBoundObject::GetPropertyNames()
{
	std::vector<std::string> string_names;
	JSPropertyNameArrayRef names =
	                 JSObjectCopyPropertyNames(this->context, this->object);

	JSPropertyNameArrayRetain(names);

	size_t count = JSPropertyNameArrayGetCount(names);
	for (size_t i = 0; i < count; i++)
	{
		JSStringRef js_name = JSPropertyNameArrayGetNameAtIndex(names, i);
		char* name = JSStringToChars(js_name);
		string_names.push_back(std::string(name));
		free(name);
	}

	JSPropertyNameArrayRelease(names);
	return string_names;
}
