#include <stdlib.h>
#include <assert.h>
#include "ti_types.h"

std::vector<AbstractCallback*> TiObject::callbacks;

TiValue::TiValue(JSContextRef context, JSValueRef value) {
    this->context = context;
    this->value = value;
}

JSValueRef TiValue::GetValue() {
    return this->value;
}
JSContextRef TiValue::GetContext() {
    return this->context;
}

TiValue TiValue::NewValue(JSValueRef value) {
    return TiValue(this->context, value);
}

TiValue TiValue::NewValue(std::string str) {
    JSStringRef str_ref = JSStringCreateWithUTF8CString(str.c_str());
    JSValueRef value = JSValueMakeString(this->context, str_ref);
    JSStringRelease(str_ref);

    return TiValue(this->context, value);
}

TiValue TiValue::NewValue(bool boolean) {
    JSValueRef value =  JSValueMakeBoolean(this->context, boolean);
    return TiValue(this->context, value);
}

TiValue TiValue::NewValue(double number) {
    JSValueRef value =  JSValueMakeNumber(this->context, number);
    return TiValue(this->context, value);
}

TiValue TiValue::Undefined() {
    return this->NewValue(JSValueMakeUndefined(this->GetContext()));
}

TiObject TiValue::GetObject() {
    JSContextRef ctx = this->GetContext();
    JSValueRef val = this->GetValue();

    if (JSValueIsObject(ctx, val)) {
        TiObject(this->context, JSValueToObject(ctx, val, NULL));
    } else {
        return this->NewObject();
    }
}

std::string TiValue::GetString() {
    std::string to_ret = "";
    JSContextRef ctx = this->GetContext();
    JSValueRef val = this->GetValue();

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

bool TiValue::GetBool() {

    JSContextRef ctx = this->GetContext();
    JSValueRef val = this->GetValue();

    if (JSValueIsBoolean(ctx, val)) {
        return JSValueToBoolean(ctx, val);
    } else {
        return false;
    }
}

double TiValue::GetNumber() {

    JSContextRef ctx = this->GetContext();
    JSValueRef val = this->GetValue();

    if (JSValueIsNumber(ctx, val)) {
        return JSValueToNumber(ctx, val, NULL);
    } else {
        return 0;
    }
}

TiObject TiValue::NewObject() {
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

JSObjectRef TiObject::GetObject() {
    return this->object;
}

void TiObject::SetProperty(std::string name, TiValue value) {
    JSStringRef name_str = JSStringCreateWithUTF8CString(name.c_str());
    JSObjectSetProperty(this->GetContext(),
                        this->GetObject(),
                        name_str,
                        value.GetValue(),
                        kJSPropertyAttributeNone,
                        NULL);
    JSStringRelease(name_str);
}

TiValue TiObject::GetProperty(std::string name) {
    JSStringRef name_str = JSStringCreateWithUTF8CString(name.c_str());
    JSValueRef value = JSObjectGetProperty(this->GetContext(),
                                           this->GetObject(),
                                           name_str,
                                           NULL);
    JSStringRelease(name_str);

    if (JSValueIsObject(this->GetContext(), value)) {
        JSObjectRef object_ref = JSValueToObject(this->GetContext(),
                                                 value,
                                                 NULL);
        return TiObject(this->GetContext(), object_ref);
    } else {
        return TiValue(this->GetContext(), value);
    }
}

AbstractCallback* TiObject::GetCallback(JSObjectRef ref) {

    for (int i=0; i < TiObject::callbacks.size(); i++) {
        AbstractCallback* c = TiObject::callbacks.at(i);
        if (c->function == ref) {
            return c;
        }
    }

    return NULL;
}

JSValueRef TiObject::CB(JSContextRef context,
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
    AbstractCallback* c = TiObject::GetCallback(function_object);
    TiValue val = c->Execute(arg_count, val_args);

    return val.GetValue();
}

