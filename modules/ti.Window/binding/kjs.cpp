/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "kjs.h"
#include <typeinfo>

JSClassRef tibo_class = NULL;
JSClassRef tibm_class = NULL;
JSClassRef tibl_class = NULL;


void BindPropertyToJSObject(JSContextRef ctx,
                            JSObjectRef o,
                            std::string name,
                            JSValueRef property)
{
	JSStringRef name_str = JSStringCreateWithUTF8CString(name.c_str());
	JSObjectSetProperty(ctx, o, name_str, property, kJSPropertyAttributeNone, NULL);
	JSStringRelease(name_str);
}

JSObjectRef KrollBoundObjectToJSValue(
            JSContextRef js_context,
            kroll::BoundObject* instance)
{
	if (tibo_class == NULL)
	{
		JSClassDefinition js_class_def = kJSClassDefinitionEmpty;
		js_class_def.className = strdup("TitaniumJSObject");
		js_class_def.getPropertyNames = get_property_names_cb;
		js_class_def.finalize = finalize_cb;
		js_class_def.hasProperty = has_property_cb;
		js_class_def.getProperty = get_property_cb;
		js_class_def.setProperty = set_property_cb;
		tibo_class = JSClassCreate (&js_class_def);
	}

	KR_ADDREF(instance);
	return JSObjectMake (js_context, tibo_class, instance);
}

JSObjectRef KrollBoundMethodToJSValue(
            JSContextRef js_context,
            kroll::BoundMethod *method)
{

	if (tibm_class == NULL)
	{
		JSClassDefinition js_class_def = kJSClassDefinitionEmpty;
		js_class_def.className = strdup("TitaniumJSMethod");
		js_class_def.getPropertyNames = get_property_names_cb;
		js_class_def.finalize = finalize_cb;
		js_class_def.hasProperty = has_property_cb;
		js_class_def.getProperty = get_property_cb;
		js_class_def.setProperty = set_property_cb;
		js_class_def.callAsFunction = call_as_function_cb;
		tibm_class = JSClassCreate (&js_class_def);
	}

	KR_ADDREF(method);
	return JSObjectMake (js_context, tibm_class, method);
}

void inline CopyJSProperty(JSContextRef ctx, JSObjectRef js_object,
                           BoundObject *ti_object, JSObjectRef ti_js_object,  const char *prop_name)
{
	JSValueRef prop = JSObjectGetProperty(ctx, js_object,
	                     JSStringCreateWithUTF8CString(prop_name), NULL);
	kroll::Value *prop_val = JSValueToKrollValue(ctx, prop, ti_js_object);
	ScopedDereferencer s(prop_val);
	ti_object->Set(prop_name, prop_val);
}


JSObjectRef KrollBoundListToJSValue(
            JSContextRef js_context,
            kroll::BoundList *list)
{

	if (tibl_class == NULL)
	{
		JSClassDefinition js_class_def = kJSClassDefinitionEmpty;
		js_class_def.className = strdup("TitaniumJSList");
		//js_class_def.attributes = kJSClassAttributeNoAutomaticPrototype;
		js_class_def.getPropertyNames = get_property_names_cb;
		js_class_def.finalize = finalize_cb;
		js_class_def.hasProperty = has_property_cb;
		js_class_def.getProperty = get_property_cb;
		js_class_def.setProperty = set_property_cb;
		tibl_class = JSClassCreate (&js_class_def);
	}

	KR_ADDREF(list);
	JSObjectRef object = JSObjectMake (js_context, tibl_class, list);

	JSValueRef args[1] = { JSValueMakeNumber(js_context, 3) };
	JSObjectRef array = JSObjectMakeArray(js_context, 1, args, NULL);

	// move some array methods
	CopyJSProperty(js_context, array, list, object, "push");
	CopyJSProperty(js_context, array, list, object, "pop");
	CopyJSProperty(js_context, array, list, object, "shift");
	CopyJSProperty(js_context, array, list, object, "unshift");
	CopyJSProperty(js_context, array, list, object, "reverse");
	CopyJSProperty(js_context, array, list, object, "splice");
	CopyJSProperty(js_context, array, list, object, "join");
	CopyJSProperty(js_context, array, list, object, "slice");

	// for some reason setting the prototype in the following way is
	// not working, so we must implement lots of Javascript array methods
	// Hopefully we'll be able to do it this way in the future:
	//JSValueRef array_prototype = JSObjectGetPrototype(js_context, array);
	//JSObjectSetPrototype(js_context, object, array_prototype);

	return object;
}

void finalize_cb(JSObjectRef js_object)
{
	kroll::BoundObject* object = (kroll::BoundObject*) JSObjectGetPrivate (js_object);
	KR_DECREF(object);
}

void get_property_names_cb (JSContextRef js_context,
                            JSObjectRef js_object,
                            JSPropertyNameAccumulatorRef js_properties)
{
	kroll::BoundObject* object = (kroll::BoundObject*) JSObjectGetPrivate (js_object);

	if (object == NULL)
		return;

	std::vector<std::string> props = object->GetPropertyNames();
	for (size_t i = 0; i < props.size(); i++)
	{
		JSStringRef name = JSStringCreateWithUTF8CString(props.at(i).c_str());
		JSPropertyNameAccumulatorAddName(js_properties, name);
		JSStringRelease(name);
	}
}

bool has_property_cb (JSContextRef js_context,
                      JSObjectRef  js_object,
                      JSStringRef  js_property)
{
	kroll::BoundObject* object = (kroll::BoundObject*) JSObjectGetPrivate (js_object);
	if (object == NULL)
		return false;

	char *name = JSStringToChars(js_property);
	std::string str_name(name);
	free(name);

	std::vector<std::string> names = object->GetPropertyNames();
	for (size_t i = 0; i < names.size(); i++)
	{
		if (names.at(i) == str_name)
			return true;
	}

	return false;
}

JSValueRef get_property_cb (JSContextRef js_context,
                            JSObjectRef  js_object,
                            JSStringRef  js_property,
                            JSValueRef*  js_exception)
{
	kroll::BoundObject* object = (kroll::BoundObject*) JSObjectGetPrivate (js_object);
	if (object == NULL)
		return JSValueMakeUndefined(js_context);

	JSValueRef js_val = NULL;
	char* name = JSStringToChars(js_property);
	try
	{
		kroll::Value* ti_val = object->Get(name);
		kroll::ScopedDereferencer s(ti_val);
		js_val = KrollValueToJSValue(js_context, ti_val);
	}
	catch (kroll::Value* exception)
	{
		kroll::ScopedDereferencer s(exception);
		*js_exception = KrollValueToJSValue(js_context, exception);
	}

	free(name);
	return js_val;
}

bool set_property_cb (JSContextRef js_context,
                      JSObjectRef  js_object,
                      JSStringRef  js_property,
                      JSValueRef   js_value,
                      JSValueRef*  js_exception)
{
	kroll::BoundObject* object = (kroll::BoundObject*) JSObjectGetPrivate (js_object);
	if (object == NULL)
		return false;

	char* prop_name = JSStringToChars(js_property);
	try
	{
		// we now own the reference returned from  JSValueTokroll::Value
		kroll::Value* ti_val = JSValueToKrollValue(js_context, js_value, js_object);
		kroll::ScopedDereferencer s(ti_val);
		object->Set(prop_name, ti_val);
	}
	catch (kroll::Value* exception)
	{
		kroll::ScopedDereferencer s(exception);
		*js_exception = KrollValueToJSValue(js_context, exception);
	}

	free(prop_name);
	return true;
}

JSValueRef call_as_function_cb (JSContextRef     js_context,
                                JSObjectRef      js_function,
                                JSObjectRef      js_this,
                                size_t           num_args,
                                const JSValueRef js_args[],
                                JSValueRef*      js_exception)
{
	kroll::BoundMethod* method = (kroll::BoundMethod*) JSObjectGetPrivate(js_function);
	if (method == NULL)
		return JSValueMakeUndefined(js_context);

	kroll::ValueList args;
	for (size_t i = 0; i < num_args; i++) {
		kroll::Value* arg_val = JSValueToKrollValue(js_context, js_args[i], js_this);
		args.push_back(arg_val);
	}

	JSValueRef js_val = NULL;
	try
	{
		kroll::Value *ti_val = method->Call(args);
		kroll::ScopedDereferencer s(ti_val);
		js_val = KrollValueToJSValue(js_context, ti_val);
	}
	catch (kroll::Value* exception)
	{
		kroll::ScopedDereferencer s(exception);
		*js_exception = KrollValueToJSValue(js_context, exception);
		js_val = NULL;
	}

	for (size_t i = 0; i < num_args; i++) {
		KR_DECREF(args[i]);
	}

	return js_val;

}

JSValueRef KrollValueToJSValue(JSContextRef ctx, kroll::Value* value)
{
	JSValueRef js_val;

	if (value->IsInt())
	{
		js_val = JSValueMakeNumber(ctx, value->ToInt());
	}
	else if (value->IsDouble())
	{
		js_val = JSValueMakeNumber(ctx, value->ToDouble());
	}
	else if (value->IsBool())
	{
		js_val = JSValueMakeBoolean(ctx, value->ToBool());
	}
	else if (value->IsString())
	{
		JSStringRef s = JSStringCreateWithUTF8CString(value->ToString().c_str());
		js_val = JSValueMakeString(ctx, s);
		JSStringRelease(s);
	}
	else if (value->IsObject())
	{
		kroll::BoundObject* obj = value->ToObject();
		KJSBoundObject* kobj = dynamic_cast<KJSBoundObject*>(obj);
		if (kobj != NULL)
		{
			// this object is actually a pure JS object
			js_val = kobj->GetJSObject();
		}
		else
		{
			// this is a kroll::BoundObject that needs to be proxied
			js_val = KrollBoundObjectToJSValue(ctx, obj);
		}
	}
	else if (value->IsMethod())
	{
		kroll::BoundMethod* meth = value->ToMethod();
		KJSBoundMethod* kmeth = dynamic_cast<KJSBoundMethod*>(meth);
		if (kmeth != NULL)
		{
			// this object is actually a pure JS callable object
			js_val = kmeth->GetJSObject();
		}
		else
		{
			// this is a TiBoundMethod that needs to be proxied
			js_val = KrollBoundMethodToJSValue(ctx, meth);
		}
	}
	else if (value->IsList())
	{
		printf("it's a list\n");
		kroll::BoundList* list = value->ToList();
		KJSBoundList* klist = dynamic_cast<KJSBoundList*>(list);
		if (klist != NULL)
		{
			// this object is actually a pure JS array
			js_val = klist->GetJSObject();
		}
		else
		{
			// this is a TiBoundMethod that needs to be proxied
			printf("proxying\n");
			js_val = KrollBoundListToJSValue(ctx, list);
		}
	}
	else if (value->IsNull())
	{
		js_val = JSValueMakeNull(ctx);
	}
	else if (value->IsUndefined())
	{
		js_val = JSValueMakeUndefined(ctx);
	}
	else
	{
		js_val = JSValueMakeUndefined(ctx);
	}

	return js_val;
}

kroll::Value* JSValueToKrollValue(JSContextRef ctx, JSValueRef value, JSObjectRef this_obj)
{
	kroll::Value *ti_val = NULL;
	JSValueRef exception = NULL;

	if (value == NULL)
	{
		fprintf(stderr, "Trying to convert NULL JSValueRef!\n");
		return kroll::Value::Undefined();
	}

	if (JSValueIsNumber(ctx, value))
	{
		ti_val = new kroll::Value(JSValueToNumber(ctx, value, &exception));
	}
	else if (JSValueIsBoolean(ctx, value))
	{
		ti_val = new kroll::Value(JSValueToBoolean(ctx, value));
	}
	else if (JSValueIsString(ctx, value))
	{

		JSStringRef string_ref = JSValueToStringCopy(ctx, value, &exception);
		if (string_ref)
		{
			char* chars = JSStringToChars(string_ref);
			std::string to_ret = std::string(chars);
			JSStringRelease(string_ref);
			free(chars);
			ti_val = new kroll::Value(to_ret);
		}

	}
	else if (JSValueIsObject(ctx, value))
	{

		JSObjectRef o = JSValueToObject(ctx, value, &exception);
		if (o != NULL)
		{
			void* data = (void*) JSObjectGetPrivate(o);
			if (JSObjectIsFunction(ctx, o) && data == NULL)
			{
				// this is a pure JS method: proxy it
				kroll::BoundMethod* tibm = new KJSBoundMethod(ctx, o, this_obj);
				ti_val = new kroll::Value(tibm);
				KR_DECREF(tibm);
			}
			else if (JSObjectIsFunction(ctx, o))
			{
				// this is a TiBoundMethod: unwrap it
				kroll::BoundMethod* tibm = (kroll::BoundMethod*) data;
				ti_val = new kroll::Value(tibm);
			}
			//else if (JSObjectIsArrayLike(ctx, o) && data == NULL)
			//{
			//	// this is a pure JS array: proxy it
			//	TiBoundList* tibl = new KJSBoundList(ctx, o);
			//	ti_val = new kroll::Value(tibl)
			//	KR_DECREF(tibl);
			//}
			//else if (JSObjectIsArrayLike(ctx, o))
			//{
			//	// this is a TiBoundList: unwrap it
			//	TiBoundList* tibl = (TiBoundList*) data;
			//	ti_val = new kroll::Value(tibl);
			//}
			else if (data == NULL)
			{
				// this is a pure JS object: proxy it
				kroll::BoundObject* tibo = new KJSBoundObject(ctx, o);
				ti_val = new kroll::Value(tibo);
				KR_DECREF(tibo);
			}
			else
			{
				// this is a kroll::BoundObject: unwrap it
				kroll::BoundObject* tibo = (kroll::BoundObject*) data;
				ti_val = new kroll::Value(tibo);
			}
		}

	}
	else if (JSValueIsNull(ctx, value))
	{
		ti_val = kroll::Value::Null();
	}
	else
	{
		ti_val = kroll::Value::Undefined();
	}

	if (ti_val != NULL && exception == NULL)
	{
		return ti_val;
	}
	else
	{
		throw JSValueToKrollValue(ctx, exception, NULL);
	}
}

char* JSStringToChars(JSStringRef js_string)
{
    size_t size = JSStringGetMaximumUTF8CStringSize (js_string);
    char* string = (char*) malloc(size);
    JSStringGetUTF8CString (js_string, string, size);
    return string;
}
