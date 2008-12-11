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
        JSValueRef get_value();
        JSContextRef get_context();

        TiValue new_value(JSValueRef value);
        TiValue new_value(char *);
        TiObject new_object();
        TiValue undefined();

        char* get_chars();

     protected:
        JSContextRef context;
        JSValueRef value;
};


class TiObject : public TiValue {

    public:
        TiObject() {}
        TiObject(JSContextRef);
        TiObject(JSContextRef, JSObjectRef);
        JSObjectRef get_object();

        void set_property(const char*, TiValue);
        TiValue get_property(const char*);

        template <class T> void bind_method(
                const char* name,
                TiValue (T::*method)(size_t, TiValue[]));

        static JSValueRef cb(JSContextRef context,
                              JSObjectRef function_object,
                              JSObjectRef this_object,
                              size_t arg_count,
                              const JSValueRef arguments[],
                              JSValueRef* exception);

        static std::vector<AbstractCallback*> callbacks;
        static AbstractCallback* get_callback(JSObjectRef ref);

    protected:
        JSObjectRef object;
};

class AbstractCallback : public TiObject {
    public:
        virtual TiValue execute(size_t arg_count, TiValue args[]) = 0;

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


        TiValue execute(size_t arg_count, TiValue args[]) {
            return (this->owner->*this->method)(arg_count, args);
        }

        TiValue (T::*method)(size_t, TiValue[]);
        T* owner;
};

template <class T>
void TiObject::bind_method(const char* name,
                 TiValue (T::*method)(size_t, TiValue[])) {
    JSStringRef name_str = JSStringCreateWithUTF8CString(name);

    JSObjectRef function =
         JSObjectMakeFunctionWithCallback(this->get_context(),
                                          name_str,
                                          TiObject::cb);

    /* map the method to the function object so that
       we can retrieve it later */
    AbstractCallback* c=
         new Callback<T>(function, static_cast<T*>(this), method);
    TiObject::callbacks.push_back(c);


    JSObjectSetProperty(this->get_context(),
                        this->get_object(),
                        name_str,
                        function,
                        kJSPropertyAttributeNone,
                        NULL);
    JSStringRelease(name_str);
}

#endif
