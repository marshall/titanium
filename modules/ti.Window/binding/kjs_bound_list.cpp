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
	TI_DECREF(kjs_bound_object);
}

TiValue* KJSBoundList::Get(const char *name, TiBoundObject *context)
{
	return kjs_bound_object->Get(name, context);
}

void KJSBoundList::Set(const char *name, TiValue* value, TiBoundObject *context)
{
	return kjs_bound_object->Set(name, value, context);
}

JSObjectRef KJSBoundList::GetJSObject()
{
	return this->object;
}

std::vector<std::string> KJSBoundList::GetPropertyNames()
{
	return kjs_bound_object->GetPropertyNames();
}

void KJSBoundList::Append(TiValue* value)
{
	TiValue *push_method = this->kjs_bound_object->Get("push", NULL);
	TiScopedDereferencer s(push_method);

	if (push_method->IsMethod())
	{
		push_method->ToMethod()->Call(value);
	}
	else
	{
		throw (new TiValue("Could not find push method on KJS array."));
	}
}

int KJSBoundList::Size()
{
	TiValue *length_val = this->kjs_bound_object->Get("length", NULL);
	TiScopedDereferencer s(length_val);

	if (length_val->IsInt())
	{
		return length_val->ToInt();
	}
	else
	{
		return 0;
	}
}

TiValue* KJSBoundList::At(int index)
{
	char* name = KJSBoundList::IntToChars(index);
	TiValue *value = this->kjs_bound_object->Get(name, NULL);
	delete [] name;
	return value;
}

char* KJSBoundList::IntToChars(int value)
{
	int digits = (int) floor(log(value)) + 1;
	char* str = new char[digits + 1];
#if defined(OS_WIN32)
	_snprintf(str, digits + 1, "%d", value);
#else
	snprintf(str, digits + 1, "%d", value);
#endif
	return str;
}

