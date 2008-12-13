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

TiValue TiValue::new_value(std::string str) {
    JSStringRef str_ref = JSStringCreateWithUTF8CString(str.c_str());
    JSValueRef value = JSValueMakeString(this->context, str_ref);
    JSStringRelease(str_ref);

    return TiValue(this->context, value);
}

TiValue TiValue::new_value(bool boolean) {
    JSValueRef value =  JSValueMakeBoolean(this->context, boolean);
    return TiValue(this->context, value);
}

TiValue TiValue::new_value(double number) {
    JSValueRef value =  JSValueMakeNumber(this->context, number);
    return TiValue(this->context, value);
}

TiValue TiValue::undefined() {
    return this->new_value(JSValueMakeUndefined(this->get_context()));
}

TiObject TiValue::get_object() {
    JSContextRef ctx = this->get_context();
    JSValueRef val = this->get_value();

    if (JSValueIsObject(ctx, val)) {
        TiObject(this->context, JSValueToObject(ctx, val, NULL));
    } else {
        return this->new_object();
    }
}

std::string TiValue::get_string() {
    std::string to_ret = "";
    JSContextRef ctx = this->get_context();
    JSValueRef val = this->get_value();

    if (JSValueIsString(ctx, val)) {
        JSStringRef string_ref = JSValueToStringCopy(ctx, val, NULL);
        size_t length = JSStringGetMaximumUTF8CStringSize(string_ref);
        char* chars = (char*) malloc(length);
        JSStringGetUTF8CString(string_ref, chars, length);
        JSStringRelease(string_ref);

        to_ret = std::string(chars);
        free(chars);
    }

    return to_ret;
}

bool TiValue::get_bool() {

    JSContextRef ctx = this->get_context();
    JSValueRef val = this->get_value();

    if (JSValueIsBoolean(ctx, val)) {
        return JSValueToBoolean(ctx, val);
    } else {
        return false;
    }
}

double TiValue::get_number() {

    JSContextRef ctx = this->get_context();
    JSValueRef val = this->get_value();

    if (JSValueIsNumber(ctx, val)) {
        return JSValueToNumber(ctx, val, NULL);
    } else {
        return 0;
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

void TiObject::set_property(std::string name, TiValue value) {
    JSStringRef name_str = JSStringCreateWithUTF8CString(name.c_str());
    JSObjectSetProperty(this->get_context(),
                        this->get_object(),
                        name_str,
                        value.get_value(),
                        kJSPropertyAttributeNone,
                        NULL);
    JSStringRelease(name_str);
}

TiValue TiObject::get_property(std::string name) {
    JSStringRef name_str = JSStringCreateWithUTF8CString(name.c_str());
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

