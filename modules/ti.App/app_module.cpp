#include "ti_app_module.h"

using namespace ti;

KROLL_MODULE(AppConfig);

void AppConfig::Initialize()
{
	kroll::StaticBoundObject *ti = 
		(kroll::StaticBoundObject *) host->GetGlobalObject()->GetProperty("ti");
	ti->BindProperty("App", this);
}

kroll::Value* AppModule::Get(std::string &name, kroll::BoundObject *context_local)
{
	return kroll::Value::Null();	
}

void AppModule::Set(std::string &name, kroll::Value *value, kroll::BoundObject *context_local)
{

}

kroll::Value* AppModule::Call(std::string &name, const kroll::ArgList &args, kroll::BoundObject *context_local)
{
	return kroll::Value::Null();
}
std::vector<std::string> AppModule::GetPropertyNames () {
		return std::vector<std::string>();
	}
}
