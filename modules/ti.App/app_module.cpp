/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "app_module.h"
#include "app_config.h"
#include "app_binding.h"

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
		config+="/"CONFIG_FILENAME;
		
		if (!FileUtils::IsFile(config))
		{
			std::cerr << "can't load " CONFIG_FILENAME " from: " << config << std::endl;
			return;
		}
		
		// initialize our application config
		AppConfig::Init(config);
		
		// load our variables
		this->variables = new AppBinding(host->GetGlobalObject());
		
		// add our command line array
		StaticBoundList *args = new StaticBoundList();
		// skip the first argument which is the filepath to the
		// executable
		for (int c=1;c<host->GetCommandLineArgCount();c++)
		{
			const char *v = host->GetCommandLineArg(c);
			Value *value = new Value(v);
			args->Append(value);
			KR_DECREF(value);
		}
		Value *argsvalue = new Value(args);
		this->variables->Set("commandline",argsvalue);
		KR_DECREF(args);
		KR_DECREF(argsvalue);
		
		// set our ti.App
		Value *value = new Value(this->variables);
		host->GetGlobalObject()->Set("App",value);
		KR_DECREF(value);
	}

	void AppModule::Destroy()
	{
		KR_DECREF(this->variables);
	}
}
