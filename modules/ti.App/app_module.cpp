#include "app_module.h"

using namespace kroll;
using namespace ti;

KROLL_MODULE(AppModule);

void AppModule::Initialize()
{
	/*kroll::StaticBoundObject *ti = 
		(kroll::StaticBoundObject *) host->GetGlobalObject()->Get("ti");

	ti->Set("App", reinterpret_cast<kroll::Value*>(this));*/
}

void AppModule::Destroy()
{

}

/*
kroll::Value* AppModule::Get(const char *name, kroll::BoundObject *context)
{
	return kroll::Value::Null();	
}

void AppModule::Set(const char *name, kroll::Value *value, kroll::BoundObject *context)
{

}
*/

/*
kroll::Value* AppModule::Call(const char *name, const kroll::ValueList &args, kroll::BoundObject *context)
{
	return kroll::Value::Null();
}
*/

/*
std::vector<std::string>
AppModule::GetPropertyNames () {
		return std::vector<std::string>();
}*/
