#include <stdlib.h>
#include <assert.h>
#include "ti_types.h"

std::vector<AbstractCallback*> TiObject::callbacks;

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
    return TiObject(this->context);
}


TiObject::TiObject(JSContextRef context) {
    JSObjectRef object = JSObjectMake(this->context, NULL, NULL);
    this->context = context;
    this->object = object;
    this->value = object;
}

TiObject::TiObject(JSContextRef context, JSObjectRef object) {
    this->context = context;
    this->object = object;
    this->value = object;
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

AbstractCallback* TiObject::get_callback(JSObjectRef ref) {

    for (int i=0; i < TiObject::callbacks.size(); i++) {
        AbstractCallback* c = TiObject::callbacks.at(i);
        if (c->function == ref) {
            return c;
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

    /* convert all arguments to jsc values */
    TiValue *val_args = new TiValue[arg_count];
    for (int i = 0; i < arg_count; i++) {
        val_args[i] = TiValue(context, arguments[i]);
    }

    /* fetch the callback object for this function */
    AbstractCallback* c = TiObject::get_callback(function_object);
    TiValue val = c->execute(arg_count, val_args);

    return val.get_value();
}

