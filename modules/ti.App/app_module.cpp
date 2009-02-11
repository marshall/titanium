/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "app_module.h"
#include "app_config.h"
#include "app_binding.h"
#include "Properties/properties_binding.h"

using namespace kroll;
using namespace ti;

KROLL_MODULE(AppModule);

namespace ti
{
	void AppModule::Initialize()
	{
		const char *home = getenv("KR_HOME");
		
		
		std::string config(home);
		config+="/"CONFIG_FILENAME;

		std::cout << "+++home = " << home << std::endl;
		std::cout << "+++config = " << config << std::endl;

		if (!FileUtils::IsFile(config))
		{
			//FIXME: in this scenario we need a cleaner way of stopping the
			//boot
			std::cerr << "can't load " CONFIG_FILENAME " from: " << config << std::endl;
			return;
		}

		// initialize our application config
		AppConfig::Init(config);

		// load our variables
		this->app_binding = new AppBinding(host,host->GetGlobalObject());

		// add our command line array
		SharedBoundList args = new StaticBoundList();
		// skip the first argument which is the filepath to the
		// executable
		for (int c=1;c<host->GetCommandLineArgCount();c++)
		{
			const char *v = host->GetCommandLineArg(c);
			Value *value = Value::NewString(v);
			args->Append(value);
		}
		SharedValue argsvalue = Value::NewList(args);
		this->app_binding->Set("commandline",argsvalue);

		// set our ti.App
		SharedValue value = Value::NewObject(this->app_binding);
		host->GetGlobalObject()->Set("App",value);

		this->properties_binding = new PropertiesBinding(host);
		SharedValue properties_value = Value::NewObject(this->properties_binding);
		this->app_binding->Set("Properties", properties_value);
	}

	void AppModule::Stop()
	{
	}
}
