/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "app_module.h"
#include "app_config.h"

using namespace kroll;
using namespace ti;

KROLL_MODULE(AppModule);

namespace ti
{
	void AppModule::Initialize()
	{
		const char *home = getenv("KR_HOME");
		std::string config(home);
#ifdef OS_OSX
		config+="/Contents";
#endif		
		config+="/tiapp.xml";
		
		if (!FileUtils::IsFile(config))
		{
			std::cerr << "can't load tiapp.xml from: " << config << std::endl;
			return;
		}
		
		AppConfig::Init(config);
		
		/*kroll::StaticBoundObject *ti = 
			(kroll::StaticBoundObject *) host->GetGlobalObject()->Get("ti");

		ti->Set("App", reinterpret_cast<kroll::Value*>(this));*/
	}

	void AppModule::Destroy()
	{

	}

	/*
	kroll::Value* AppModule::Get(const char *name)
	{
		return kroll::Value::Null();	
	}

	void AppModule::Set(const char *name, kroll::Value *value)
	{

	}
	*/

	/*
	kroll::Value* AppModule::Call(const char *name, const kroll::ValueList &args)
	{
		return kroll::Value::Null();
	}
	*/

	/*
	std::vector<std::string>
	AppModule::GetPropertyNames () {
			return std::vector<std::string>();
	}*/
}
