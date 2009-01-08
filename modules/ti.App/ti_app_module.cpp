#include "ti_app_module.h"

using namespace ti;

void AppConfig::Initialize()
{
	TiBoundObject *ti = host->GetGlobalObject()->GetProperty("ti");
	ti->BindProperty("App", this);
}

TiValue* AppModule::Get(std::string &name, TiBoundObject *context_local)
{
	return TiValue::Null();	
}

void AppModule::Set(std::string &name, TiValue *value, TiBoundObject *context_local)
{

}

TiValue* AppModule::Call(std::string &name, const kroll::ArgList &args, TiBoundObject *context_local)
{
	return TiValue::Null();
}
