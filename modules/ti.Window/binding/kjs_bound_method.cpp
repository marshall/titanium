/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "kjs.h"

KJSBoundMethod::KJSBoundMethod(JSContextRef context,
                               JSObjectRef js_object,
                               JSObjectRef this_obj)
	: context(context),
	  object(js_object),
	  this_obj(this_obj)
{
	JSValueProtect(context, js_object);

	if (this_obj != NULL)
		JSValueProtect(context, this_obj);

	this->kjs_bound_object = new KJSBoundObject(context, js_object);
}

KJSBoundMethod::~KJSBoundMethod()
{
	JSValueUnprotect(this->context, this->object);

	if (this->this_obj != NULL)
		JSValueUnprotect(this->context, this->this_obj);

	KR_DECREF(kjs_bound_object);
}

kroll::Value* KJSBoundMethod::Get(const char *name)
{
	return kjs_bound_object->Get(name);
}

void KJSBoundMethod::Set(const char *name, kroll::Value* value)
{
	return kjs_bound_object->Set(name, value);
}

JSObjectRef KJSBoundMethod::GetJSObject()
{
	return this->object;
}

std::vector<std::string> KJSBoundMethod::GetPropertyNames()
{
	return kjs_bound_object->GetPropertyNames();
}

kroll::Value* KJSBoundMethod::Call(const kroll::ValueList& args)
{

	JSValueRef* js_args = new JSValueRef[args.size()];

	for (int i = 0; i < (int) args.size(); i++)
	{
		kroll::Value* arg = args.at(i);
		js_args[i] = KrollValueToJSValue(this->context, arg);
	}

	JSValueRef exception = NULL;
	JSValueRef js_value = JSObjectCallAsFunction(this->context,
	                                             this->object,
	                                             this->this_obj,
	                                             args.size(),
	                                             js_args,
	                                             &exception);
	delete [] js_args; // clean up args

	if (js_value == NULL && exception != NULL) //exception thrown
	{
		kroll::Value* tv_exp = JSValueToKrollValue(this->context, exception, NULL);
		throw tv_exp;
	}

	return JSValueToKrollValue(this->context, js_value, NULL);
}


