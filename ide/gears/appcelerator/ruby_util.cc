#include "gears/base/common/js_types.h"
#include "gears/base/common/basictypes.h"
#include "gears/base/common/js_dom_element.h"
#include "gears/base/common/js_runner.h"
#include "third_party/scoped_ptr/scoped_ptr.h"
#include "gears/appcelerator/app_command.h"
#include "gears/appcelerator/ruby_util.h"
#include "gears/appcelerator/appcelerator.h"

static char* RubyTypeToString (int type)
{
	switch (type) {
		case T_NIL: return "T_NIL";
		case T_STRING: return "T_STRING";
		case T_TRUE: return "T_TRUE";
		case T_FALSE: return "T_FALSE";
		case T_FIXNUM: return "T_FIXNUM";
	}
	return "UNKNOWN??";
}

static char* JsParamTypeToString (JsParamType type)
{
	switch (type) {
		case JSPARAM_NULL: return "JSPARAM_NULL";
		case JSPARAM_STRING16: return "JSPARAM_STRING16";
		case JSPARAM_BOOL: return "JSPARAM_BOOL";
		case JSPARAM_INT: return "JSPARAM_INT";
	}
	return "UNKNOWN";
}

JsParamType RubyTypeToJsParamType (int type)
{
	switch(type) {
		case T_NIL: return JSPARAM_NULL;
		case T_STRING: return JSPARAM_STRING16;
		case T_TRUE: return JSPARAM_BOOL;
		case T_FALSE: return JSPARAM_BOOL;
		case T_FIXNUM: return JSPARAM_INT;
	}
}

bool _true = true, _false = false;
int val;
void* RubyValueToJsValue (VALUE value)
{
	switch(TYPE(value)) {
		case T_NIL: return NULL;
		case T_STRING: {
			std::string16 *val = new std::string16(UTF8ToString16(StringValueCStr(value)));
			return val;
		}
		case T_TRUE: {
			return &_true;
		}
		case T_FALSE: {
			return &_false;
		}
		case T_FIXNUM: {
			val = (int)NUM2INT(value);
			return &val;
		}
	}	
}

VALUE JsObjectToRubyObject (JsObject &object)
{
	VALUE rubyObject = rb_hash_new();
	
	std::vector<std::string16> propertyNames;
	if (object.GetPropertyNames(&propertyNames)) {
		for (int i = 0; i < propertyNames.size(); i++) {
			JsScopedToken token;
			if (object.GetProperty(propertyNames[i], &token)) {
				
				JsParamType type = object.GetPropertyType(propertyNames[i]);
				
				rb_hash_aset(rubyObject, STRING16_TO_VALUE(propertyNames[i]),
					JsTokenToRubyValue(&token, object.context(), type, false));
			}
		}
	}
	
	return rubyObject;
}

VALUE JsRootedTokenToRubyValue (JsRootedToken *token, JsParamType type)
{
	return JsTokenToRubyValue((JsScopedToken *)(&(token->token())), token->context(), type, true);	
}

VALUE JsTokenToRubyValue (JsScopedToken *token, JsContextPtr context, JsParamType type, bool free)
{
	if (JsTokenIsNullOrUndefined(*token)) {
		return Qnil;
	}
	
	if (type == JSPARAM_BOOL) {
		bool value;
		
		if (JsTokenToBool_NoCoerce(*token, context, &value))
		{
			if (free) delete token;
			return RUBY_BOOL(value);
		}
		return Qfalse;
	}
	else if (type == JSPARAM_STRING16) {
		std::string16 value;
		if (JsTokenToString_NoCoerce(*token, context, &value))
		{
			if (free) delete token;
			return STRING16_TO_VALUE(value);
		}
		return Qnil;
	}
	else if (type == JSPARAM_OBJECT) {
		JsObject value;
		value.SetObject(*token, context);
		//if (free) delete token;
			
		VALUE val = JsObjectToRubyObject(value);
		if (free) delete token;
		
		return val;
	}
	else if (type == JSPARAM_INT) {
		int value;
		
		if (JsTokenToInt_NoCoerce(*token, context, &value))
		{
			if (free) delete token;
			return INT2NUM(value);
		}
	}
	return Qnil;
}

bool InvokeJsCallback (AppCommand *appC, JsObject *object, char* function, int argc, VALUE* argv, JsRootedToken **returnValue)
{
	if (appC != NULL) {
		JsRootedCallback *callback;
		if (!object->GetPropertyAsFunction(UTF8ToString16(function).c_str(), &callback))
		{
			return false;
		}
		
		JsParamToSend param_argv[argc];
		for (int i = 0; i < argc; i++) {
			JsParamToSend param;
			
			param.type = RubyTypeToJsParamType(TYPE(argv[i]));
			param.value_ptr = RubyValueToJsValue(argv[i]);

			param_argv[i] = param;
		}
		
	  	return appC->GetJsRunner()->InvokeCallback(callback, argc, param_argv, returnValue);
	}
	
	return false;
}

static bool rubyBooted = false;
void RunScript (AppCommand *appC, const char* fullPath)
{	
	if (!rubyBooted) {
		ruby_init();
		ruby_init_loadpath();
		appC->DefineClasses();
		
		rubyBooted = true;
	}
	
	VALUE options = JsObjectToRubyObject(*(appC->appOptions));
	rb_define_variable("$TitaniumOptions", &options);
	
	ruby_script(fullPath);
	rb_load_file(fullPath);
	
	int status = ruby_exec();
	
	// don't cleanup yet.. we need to keep references to the function objects for callbacks ?
	//ruby_cleanup(status);
}