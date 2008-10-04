
#include "gears/base/common/base_class.h"
#include "gears/base/common/common.h"
#include "gears/base/common/js_runner.h"

extern "C" {
#include <ruby.h>
}

#ifndef RUBY_WRAPPER_H
#define RUBY_WRAPPER_H

#define RUBY_FUNCTION(x) ((VALUE(*)(...))x)
#define RUBY_BOOL(x) (x ? Qtrue : Qfalse)
#define STRING16_TO_VALUE(x) rb_str_new2(String16ToUTF8(x).c_str())

class RubyInitializer {
public:
	virtual JsObject* GetCallbackObject() = 0;
};


class RubyWrapper : public RubyInitializer {
public:
	RubyWrapper() { rubyBooted = false; }
	~RubyWrapper() { }
	
	void RunScript (const char* fullPath);
	virtual JsObject* GetCallbackObject();
	void DefineClasses();
	
private:
	bool rubyBooted;
};

JsParamType RubyTypeToJsParamType (int type);
void* RubyValueToJsValue (VALUE value);
VALUE JsRootedTokenToRubyValue (JsRootedToken *token, JsParamType type);
VALUE JsTokenToRubyValue (JsScopedToken *token, JsContextPtr context, JsParamType type, bool free);
bool InvokeJsCallback (JsObject *object, char* function, int argc, VALUE* argv, JsRootedToken **returnValue);

#endif
