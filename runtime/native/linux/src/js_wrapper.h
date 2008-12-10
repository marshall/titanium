#ifndef __JS_UTIL_H
#define __JS_UTIL_H

#include <vector>
class TiObject;

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

     private:
        JSValueRef value;
};

typedef TiValue (TiObject::*TiMethod)(size_t, TiValue[]);
typedef struct {
    JSObjectRef function;
    TiMethod ti_method;
    TiObject* ti_object;
} MethodMapping;

class TiObject : public TiValue {

    public:
        TiObject(JSContextRef, JSObjectRef);
        JSObjectRef get_object();

        void set_property(const char*, TiValue);
        TiValue get_property(const char*);

        void bind_method(const char*, TiMethod);
        static JSValueRef cb(JSContextRef context,
                              JSObjectRef function_object,
                              JSObjectRef this_object,
                              size_t arg_count,
                              const JSValueRef arguments[],
                              JSValueRef* exception);

        static std::vector<MethodMapping*> method_mapping;
        static MethodMapping* get_method_mapping(JSObjectRef ref);

        TiValue printer(size_t, TiValue[]);
    private:
        JSObjectRef object;
};

#endif
