/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _KJS_H_
#define _KJS_H_

#include <kroll/kroll.h>
#include <JavaScriptCore/JSObjectRef.h>
#include <JavaScriptCore/JSStringRef.h>
#include <JavaScriptCore/JSContextRef.h>
#include <cstring>
#include <map>

#include "kjs_bound_object.h"
#include "kjs_bound_method.h"
#include "kjs_bound_list.h"

void BindPropertyToJSObject(JSContextRef ctx,
                            JSObjectRef o,
                            std::string name,
                            JSValueRef property);
JSObjectRef KrollBoundObjectToJSValue(JSContextRef, kroll::BoundObject*);
JSObjectRef KrollBoundMethodToJSValue(JSContextRef, kroll::BoundMethod*);
JSObjectRef KrollBoundListToJSValue(JSContextRef, kroll::BoundList*);
JSValueRef KrollValueToJSValue(JSContextRef, kroll::Value*);
kroll::Value* JSValueToKrollValue(JSContextRef, JSValueRef, JSObjectRef);
char* JSStringToChars(JSStringRef);

// callbacks
void get_property_names_cb (JSContextRef, JSObjectRef, JSPropertyNameAccumulatorRef);
bool has_property_cb (JSContextRef, JSObjectRef, JSStringRef);
JSValueRef get_property_cb (JSContextRef, JSObjectRef, JSStringRef, JSValueRef*);
bool set_property_cb (JSContextRef, JSObjectRef, JSStringRef, JSValueRef, JSValueRef*);
JSValueRef call_as_function_cb (JSContextRef, JSObjectRef, JSObjectRef, size_t, const JSValueRef[], JSValueRef*);
void finalize_cb(JSObjectRef);

#endif
