/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "kjs.h"
#include <cstdio>

KJSBoundList::KJSBoundList(JSContextRef context, JSObjectRef js_object)
	: context(context), object(js_object)
{
	JSValueProtect(context, js_object);
	this->kjs_bound_object = new KJSBoundObject(context, js_object);
}

KJSBoundList::~KJSBoundList()
{
	JSValueUnprotect(this->context, this->object);
	KR_DECREF(kjs_bound_object);
}

kroll::Value* KJSBoundList::Get(const char *name)
{
	return kjs_bound_object->Get(name);
}

void KJSBoundList::Set(const char *name, kroll::Value* value)
{
	return kjs_bound_object->Set(name, value);
}

JSObjectRef KJSBoundList::GetJSObject()
{
	return this->object;
}

std::vector<std::string> KJSBoundList::GetPropertyNames()
{
	return kjs_bound_object->GetPropertyNames();
}

void KJSBoundList::Append(kroll::Value* value)
{
	kroll::Value *push_method = this->kjs_bound_object->Get("push");
	kroll::ScopedDereferencer s(push_method);

	if (push_method->IsMethod())
	{
		push_method->ToMethod()->Call(value);
	}
	else
	{
		throw (new kroll::Value("Could not find push method on KJS array."));
	}
}

int KJSBoundList::Size()
{
	kroll::Value *length_val = this->kjs_bound_object->Get("length");
	kroll::ScopedDereferencer s(length_val);

	if (length_val->IsInt())
	{
		return length_val->ToInt();
	}
	else
	{
		return 0;
	}
}

kroll::Value* KJSBoundList::At(int index)
{
	char* name = KJSBoundList::IntToChars(index);
	kroll::Value *value = this->kjs_bound_object->Get(name);
	delete [] name;
	return value;
}

char* KJSBoundList::IntToChars(int value)
{
	int digits = (int) floor(log((double)value)) + 1;
	char* str = new char[digits + 1];
#if defined(OS_WIN32)
	_snprintf(str, digits + 1, "%d", value);
#else
	snprintf(str, digits + 1, "%d", value);
#endif
	return str;
}

