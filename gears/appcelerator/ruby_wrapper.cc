
#include "gears/base/common/js_types.h"

#include "gears/appcelerator/ruby_wrapper.h"
#include "gears/appcelerator/appcelerator.h"
#include "gears/appcelerator/TitaniumBoot.h"

extern Appcelerator *app;

JsParamType RubyTypeToJsParamType (int type)
{
	switch(type) {
		case T_NIL: return JSPARAM_NULL;
		case T_STRING: return JSPARAM_STRING16;
		case T_TRUE:
		case T_FALSE: return JSPARAM_BOOL;
	}
}

void* RubyValueToJsValue (VALUE value)
{
	switch(TYPE(value)) {
		case T_NIL: return NULL;
		case T_STRING: {
			std::string16 val = UTF8ToString16(StringValueCStr(value));
			return &val;
		}
		case T_TRUE: {
			bool val = true;
			return &val;
		}
		case T_FALSE: {
			bool val = false;
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
		if (free) delete token;
			
		return JsObjectToRubyObject(value);
	}
	return Qnil;
}

bool InvokeJsCallback (JsObject *object, char* function, int argc, VALUE* argv, JsRootedToken **returnValue)
{
	if (app != NULL) {
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
		
	  	return app->GetJsRunner()->InvokeCallback(callback, argc, param_argv, returnValue);
	}
	
	return false;
}

JsObject* RubyWrapper::GetCallbackObject ()
{
	return &(app->GetBootCallback());
}

void RubyWrapper::RunScript (const char* fullPath)
{	
	if (!rubyBooted) {
		ruby_init();
		ruby_init_loadpath();
		DefineClasses();
		
		rubyBooted = true;
	}
	
	ruby_script(fullPath);
	rb_load_file(fullPath);
	
	int status = ruby_exec();
	ruby_cleanup(status);
}

void RubyWrapper::DefineClasses()
{
	TitaniumBoot::DefineClass(this);
}