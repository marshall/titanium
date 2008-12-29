#ifndef __JS_UTIL_H
#define __JS_UTIL_H
#include <vector>
#include <stdio.h>

class TiObject;
class AbstractCallback;
template <class T> class Callback;

class TiValue {

    public:
        TiValue() {}
        TiValue(JSContextRef, JSValueRef);
        JSValueRef GetValue();
        JSContextRef GetContext();

        TiValue NewValue(JSValueRef value);
        TiValue NewValue(std::string);
        TiValue NewValue(bool);
        TiValue NewValue(double);
        TiObject NewObject();
        TiValue Undefined();

        TiObject GetObject();
        std::string GetString();
        bool GetBool();
        double GetNumber();

     protected:
        JSContextRef context;
        JSValueRef value;
};


class TiObject : public TiValue {

    public:
        TiObject() {}
        TiObject(JSContextRef);
        TiObject(JSContextRef, JSObjectRef);
        JSObjectRef GetObject();

        void SetProperty(std::string, TiValue);
        TiValue GetProperty(std::string);

        template <class T> void BindMethod(
                const std::string,
                TiValue (T::*method)(size_t, TiValue[]));

        static JSValueRef CB(JSContextRef context,
                              JSObjectRef function_object,
                              JSObjectRef this_object,
                              size_t arg_count,
                              const JSValueRef arguments[],
                              JSValueRef* exception);

        static std::vector<AbstractCallback*> callbacks;
        static AbstractCallback* GetCallback(JSObjectRef ref);

    protected:
        JSObjectRef object;
};

class AbstractCallback : public TiObject {
    public:
        virtual TiValue Execute(size_t arg_count, TiValue args[]) = 0;

        JSObjectRef function;
};

template <class T>
class Callback : public AbstractCallback {
    public:
        Callback(JSObjectRef function,
                    T* owner,
                    TiValue (T::*method)(size_t, TiValue[])) {
            this->function = function;
            this->owner = owner;
            this->method = method;
        }


        TiValue Execute(size_t arg_count, TiValue args[]) {
            return (this->owner->*this->method)(arg_count, args);
        }

        TiValue (T::*method)(size_t, TiValue[]);
        T* owner;
};

template <class T>
void TiObject::BindMethod(std::string name,
                 TiValue (T::*method)(size_t, TiValue[])) {
    JSStringRef name_str = JSStringCreateWithUTF8CString(name.c_str());

    JSObjectRef function =
         JSObjectMakeFunctionWithCallback(this->GetContext(),
                                          name_str,
                                          TiObject::CB);

    /* map the method to the function object so that
       we can retrieve it later */
    AbstractCallback* c=
         new Callback<T>(function, static_cast<T*>(this), method);
    TiObject::callbacks.push_back(c);


    JSObjectSetProperty(this->GetContext(),
                        this->GetObject(),
                        name_str,
                        function,
                        kJSPropertyAttributeNone,
                        NULL);
    JSStringRelease(name_str);
}

#endif
