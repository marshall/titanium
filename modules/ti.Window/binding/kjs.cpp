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

std::map <JSContextRef, TiStaticBoundObject*> context_locals;
TiStaticBoundObject* GetContextLocal(JSContextRef ref)
{
	std::map<JSContextRef, TiStaticBoundObject*>::iterator i;
	i = context_locals.find(ref);

	TiStaticBoundObject *context_local;
	if (i == context_locals.end())
	{
		/*
		 * By default we don't ADDREF the context object here,
		 * because only the caller knows if the reference will
		 * be retained. If it will be, the caller can ADDREF, signalling
		 * to the pointer counter that they are forking the reference.
		*/
		context_local = new TiStaticBoundObject();
	}
	else
	{
		context_local = i->second;
	}
	return context_local;
}

JSObjectRef TiBoundObjectToJSValue(
            JSContextRef js_context,
            TiBoundObject* instance)
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

	TI_ADDREF(instance);
	return JSObjectMake (js_context, tibo_class, instance);
}

JSObjectRef TiBoundMethodToJSValue(
            JSContextRef js_context,
            TiBoundMethod *method)
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

	TI_ADDREF(method);
	return JSObjectMake (js_context, tibm_class, method);
}

JSObjectRef TiBoundListToJSValue(
            JSContextRef js_context,
            TiBoundList *list)
{

	if (tibl_class == NULL)
	{
		JSClassDefinition js_class_def = kJSClassDefinitionEmpty;
		js_class_def.className = strdup("TitaniumJSList");
		js_class_def.attributes = kJSClassAttributeNoAutomaticPrototype;
		js_class_def.getPropertyNames = get_property_names_cb;
		js_class_def.finalize = finalize_cb;
		js_class_def.hasProperty = has_property_cb;
		js_class_def.getProperty = get_property_cb;
		js_class_def.setProperty = set_property_cb;
		tibl_class = JSClassCreate (&js_class_def);
	}


	TI_ADDREF(list);

	JSValueRef args[0];
	JSObjectRef array = JSObjectMakeArray(js_context, 0, args, NULL);

	JSObjectRef object = JSObjectMake (js_context, tibl_class, list);
	JSValueRef array_prototype = JSObjectGetPrototype(js_context, array);
	JSObjectSetPrototype(js_context, object, array_prototype);

	return object;
}

void finalize_cb(JSObjectRef js_object)
{
	TiBoundObject* object = (TiBoundObject*) JSObjectGetPrivate (js_object);
	TI_DECREF(object);
}

void get_property_names_cb (JSContextRef js_context,
                            JSObjectRef js_object,
                            JSPropertyNameAccumulatorRef js_properties)
{
	TiBoundObject* object = (TiBoundObject*) JSObjectGetPrivate (js_object);

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
	TiBoundObject* object = (TiBoundObject*) JSObjectGetPrivate (js_object);
	if (object == NULL)
		return false;

	bool has_value = false;

	char *name = JSStringToChars(js_property);
	std::string str_name(name);
	free(name);

	std::vector<std::string> names = object->GetPropertyNames();
	for (size_t i = 0; i < names.size(); i++)
	{
		if (names.at(i) == str_name)
			has_value = true;
	}

	return has_value;
}

JSValueRef get_property_cb (JSContextRef js_context,
                            JSObjectRef  js_object,
                            JSStringRef  js_property,
                            JSValueRef*  js_exception)
{
	TiBoundObject* context_local = GetContextLocal(js_context);
	TiBoundObject* object = (TiBoundObject*) JSObjectGetPrivate (js_object);
	if (object == NULL)
		return JSValueMakeUndefined(js_context);

	JSValueRef js_val = NULL;
	char* name = JSStringToChars(js_property);
	try
	{
		TiValue* ti_val = object->Get(name, context_local);
		TiScopedDereferencer s(ti_val);
		js_val = TiValueToJSValue(js_context, ti_val);
	}
	catch (TiValue* exception)
	{
		TiScopedDereferencer s(exception);
		*js_exception = TiValueToJSValue(js_context, exception);
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
	TiBoundObject* context_local = GetContextLocal(js_context);
	TiBoundObject* object = (TiBoundObject*) JSObjectGetPrivate (js_object);
	if (object == NULL)
		return false;

	char* prop_name = JSStringToChars(js_property);
	try
	{
		// we now own the reference returned from  JSValueToTiValue
		TiValue* ti_val = JSValueToTiValue(js_context, js_value, js_object);
		TiScopedDereferencer s(ti_val);
		object->Set(prop_name, ti_val, context_local);
	}
	catch (TiValue* exception)
	{
		TiScopedDereferencer s(exception);
		*js_exception = TiValueToJSValue(js_context, exception);
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
	TiBoundMethod* method = (TiBoundMethod*) JSObjectGetPrivate(js_function);
	TiBoundObject* context_local = GetContextLocal(js_context);
	if (method == NULL)
		return JSValueMakeUndefined(js_context);

	TiValueList args;
	for (size_t i = 0; i < num_args; i++) {
		TiValue* arg_val = JSValueToTiValue(js_context, js_args[i], js_this);
		args.push_back(arg_val);
	}

	JSValueRef js_val = NULL;
	try
	{
		TiValue *ti_val = method->Call(args, context_local);
		TiScopedDereferencer s(ti_val);
		js_val = TiValueToJSValue(js_context, ti_val);
	}
	catch (TiValue* exception)
	{
		TiScopedDereferencer s(exception);
		*js_exception = TiValueToJSValue(js_context, exception);
		js_val = NULL;
	}

	for (size_t i = 0; i < num_args; i++) {
		TI_DECREF(args[i]);
	}

	return js_val;

}

JSValueRef TiValueToJSValue(JSContextRef ctx, TiValue* value)
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
		TiBoundObject* obj = value->ToObject();
		KJSBoundObject* kobj = dynamic_cast<KJSBoundObject*>(obj);
		if (kobj != NULL)
		{
			// this object is actually a pure JS object
			js_val = kobj->GetJSObject();
		}
		else
		{
			// this is a TiBoundObject that needs to be proxied
			js_val = TiBoundObjectToJSValue(ctx, obj);
		}
	}
	else if (value->IsMethod())
	{
		TiBoundMethod* meth = value->ToMethod();
		KJSBoundMethod* kmeth = dynamic_cast<KJSBoundMethod*>(meth);
		if (kmeth != NULL)
		{
			// this object is actually a pure JS callable object
			js_val = kmeth->GetJSObject();
		}
		else
		{
			// this is a TiBoundMethod that needs to be proxied
			js_val = TiBoundMethodToJSValue(ctx, meth);
		}
	}
	else if (value->IsList())
	{
		TiBoundList* list = value->ToList();
		KJSBoundList* klist = dynamic_cast<KJSBoundList*>(list);
		if (klist != NULL)
		{
			// this object is actually a pure JS array
			js_val = klist->GetJSObject();
		}
		else
		{
			// this is a TiBoundMethod that needs to be proxied
			js_val = TiBoundListToJSValue(ctx, list);
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

TiValue* JSValueToTiValue(JSContextRef ctx, JSValueRef value, JSObjectRef this_obj)
{
	TiValue *ti_val = NULL;
	JSValueRef exception = NULL;

	if (value == NULL)
	{
		fprintf(stderr, "Trying to convert NULL JSValueRef!\n");
		return TiValue::Undefined();
	}

	if (JSValueIsNumber(ctx, value))
	{
		ti_val = new TiValue(JSValueToNumber(ctx, value, &exception));
	}
	else if (JSValueIsBoolean(ctx, value))
	{
		ti_val = new TiValue(JSValueToBoolean(ctx, value));
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
			ti_val = new TiValue(to_ret);
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
				TiBoundMethod* tibm = new KJSBoundMethod(ctx, o, this_obj);
				ti_val = new TiValue(tibm);
				TI_DECREF(tibm);
			}
			else if (JSObjectIsFunction(ctx, o))
			{
				// this is a TiBoundMethod: unwrap it
				TiBoundMethod* tibm = (TiBoundMethod*) data;
				ti_val = new TiValue(tibm);
			}
			//else if (JSObjectIsArrayLike(ctx, o) && data == NULL)
			//{
			//	// this is a pure JS array: proxy it
			//	TiBoundList* tibl = new KJSBoundList(ctx, o);
			//	ti_val = new TiValue(tibl)
			//	TI_DECREF(tibl);
			//}
			//else if (JSObjectIsArrayLike(ctx, o))
			//{
			//	// this is a TiBoundList: unwrap it
			//	TiBoundList* tibl = (TiBoundList*) data;
			//	ti_val = new TiValue(tibl);
			//}
			else if (data == NULL)
			{
				// this is a pure JS object: proxy it
				TiBoundObject* tibo = new KJSBoundObject(ctx, o);
				ti_val = new TiValue(tibo);
				TI_DECREF(tibo);
			}
			else
			{
				// this is a TiBoundObject: unwrap it
				TiBoundObject* tibo = (TiBoundObject*) data;
				ti_val = new TiValue(tibo);
			}
		}

	}
	else if (JSValueIsNull(ctx, value))
	{
		ti_val = TiValue::Null();
	}
	else
	{
		ti_val = TiValue::Undefined();
	}

	if (ti_val != NULL && exception == NULL)
	{
		return ti_val;
	}
	else
	{
		throw JSValueToTiValue(ctx, exception, NULL);
	}
}

char* JSStringToChars(JSStringRef js_string)
{
    size_t size = JSStringGetMaximumUTF8CStringSize (js_string);
    char* string = (char*) malloc(size);
    JSStringGetUTF8CString (js_string, string, size);
    return string;
}
