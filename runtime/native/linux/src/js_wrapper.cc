#include <stdlib.h>
#include <assert.h>
#include <webkit/webkit.h>
#include <JavaScriptCore/JSObjectRef.h>
#include <JavaScriptCore/JSStringRef.h>
#include "js_wrapper.h"

std::vector<MethodMapping*> TiObject::method_mapping;

TiValue::TiValue(JSContextRef context, JSValueRef value) {
    this->context = context;
    this->value = value;
}

JSValueRef TiValue::get_value() {
    return this->value;
}
JSContextRef TiValue::get_context() {
    return this->context;
}

TiValue TiValue::new_value(JSValueRef value) {
    return TiValue(this->context, value);
}

TiValue TiValue::new_value(char* chars) {
    JSStringRef str_ref = JSStringCreateWithUTF8CString(chars);
    JSValueRef value = JSValueMakeString(this->context, str_ref);
    JSStringRelease(str_ref);

    return TiValue(this->context, value);
}

TiValue TiValue::undefined() {
    return this->new_value(JSValueMakeUndefined(this->get_context()));
}

char* TiValue::get_chars() {

    JSContextRef ctx = this->get_context();
    JSValueRef val = this->get_value();

    if (JSValueIsString(ctx, val)) {
        JSStringRef string_ref = JSValueToStringCopy(ctx, val, NULL);
        size_t length = JSStringGetMaximumUTF8CStringSize(string_ref);
        char* chars = (char*) malloc(length);
        JSStringGetUTF8CString(string_ref, chars, length);
        JSStringRelease(string_ref);

        return chars;
    } else {
        return NULL;
    }
}

TiObject TiValue::new_object() {
    JSObjectRef object = JSObjectMake(this->context, NULL, NULL);
    return TiObject(this->context, object);
}


TiObject::TiObject(JSContextRef context, JSObjectRef object)
        : TiValue(context, object) {
    this->object = object;
}

JSObjectRef TiObject::get_object() {
    return this->object;
}

void TiObject::set_property(const char *name, TiValue value) {
    JSStringRef name_str = JSStringCreateWithUTF8CString(name);
    JSObjectSetProperty(this->get_context(),
                        this->get_object(),
                        name_str,
                        value.get_value(),
                        kJSPropertyAttributeNone,
                        NULL);
    JSStringRelease(name_str);
}

TiValue TiObject::get_property(const char *name) {
    JSStringRef name_str = JSStringCreateWithUTF8CString(name);
    JSValueRef value = JSObjectGetProperty(this->get_context(),
                                           this->get_object(),
                                           name_str,
                                           NULL);
    JSStringRelease(name_str);

    if (JSValueIsObject(this->get_context(), value)) {
        JSObjectRef object_ref = JSValueToObject(this->get_context(),
                                                 value,
                                                 NULL);
        return TiObject(this->get_context(), object_ref);
    } else {
        return TiValue(this->get_context(), value);
    }
}

void TiObject::bind_method(const char *name, TiMethod method) {

    JSStringRef name_str = JSStringCreateWithUTF8CString(name);

    JSObjectRef function =
         JSObjectMakeFunctionWithCallback(this->get_context(),
                                          name_str,
                                          TiObject::cb);

    /* map the method to the function object so that
 *     we can retrieve it later */
    MethodMapping* mapping = new MethodMapping;
    mapping->function = function;
    mapping->ti_method = method;
    mapping->ti_object = this;
    TiObject::method_mapping.push_back(mapping);

    JSObjectSetProperty(this->get_context(),
                        this->get_object(),
                        name_str,
                        function,
                        kJSPropertyAttributeNone,
                        NULL);
    JSStringRelease(name_str);
}

MethodMapping* TiObject::get_method_mapping(JSObjectRef ref) {
    for (int i=0; i < TiObject::method_mapping.size(); i++) {
        MethodMapping* mapping = TiObject::method_mapping.at(i);
        if (mapping->function == ref) {
            return mapping;
        }
    }
    return NULL;
}

JSValueRef TiObject::cb(JSContextRef context,
                        JSObjectRef function_object,
                        JSObjectRef this_object,
                        size_t arg_count,
                        const JSValueRef arguments[],
                        JSValueRef* exception) {


    /* fetch the "this" jsc object and the method pointer */
    MethodMapping* mapping = TiObject::get_method_mapping(function_object);
    TiObject* this_tiobject = mapping->ti_object;
    TiMethod method = mapping->ti_method;

    /* convert all arguments to jsc values */
    TiValue *val_args = new TiValue[arg_count];
    for (int i = 0; i < arg_count; i++) {
        val_args[i] = TiValue(context, arguments[i]);
    }

    TiValue val = (this_tiobject->*method)(arg_count, val_args);

    return val.get_value();
}

/* example of a callback method */
TiValue TiObject::printer(size_t num_args, TiValue args[]) {

    if (num_args > 0) {
        TiValue arg = args[0];

        char* string = arg.get_chars();
        if (string != NULL) {
            printf("%s\n", string);
            free(string);
        }
    }

    return this->undefined();
}

