#ifndef RUBY_UTIL_H
#define RUBY_UTIL_H

#include <ruby.h>

#include "gears/base/common/common.h"
#include "gears/base/common/js_types.h"
#include "gears/base/common/js_runner.h"

#define RUBY_FUNCTION(x) ((VALUE(*)(...))x)
#define RUBY_BOOL(x) (x ? Qtrue : Qfalse)
#define STRING16_TO_VALUE(x) rb_str_new2(String16ToUTF8(x).c_str())

JsParamType RubyTypeToJsParamType (int type);
void* RubyValueToJsValue (VALUE value);
VALUE JsRootedTokenToRubyValue (JsRootedToken *token, JsParamType type);
VALUE JsTokenToRubyValue (JsScopedToken *token, JsContextPtr context, JsParamType type, bool free_);
VALUE JsObjectToRubyObject (JsObject &object);
bool InvokeJsCallback (AppCommand *appC, JsObject *object, char* function, int argc, VALUE* argv, JsRootedToken **returnValue);
void RunScript (AppCommand *appC, const char* fullPath);

#endif
