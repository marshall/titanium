/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "app_module.h"
#include "app_config.h"
#include "app_binding.h"
#include "Properties/properties_binding.h"
#include <Poco/File.h>

using namespace kroll;
using namespace ti;

KROLL_MODULE(AppModule);

namespace ti
{
	void AppModule::Initialize()
	{
		std::string home = host->GetApplicationHome();
		std::string config = FileUtils::Join(home.c_str(),CONFIG_FILENAME,NULL);

		PRINTD("+++home = " << home);
		PRINTD("+++config = " << config);

		if (!FileUtils::IsFile(config))
		{
			//FIXME: in this scenario we need a cleaner way of stopping the
			//boot
			std::cerr << "can't load " CONFIG_FILENAME " from: " << config << std::endl;
			//return;
		}

		// initialize our application config
		AppConfig::Init(config);

		// load our variables
		this->app_binding = new AppBinding(host,host->GetGlobalObject());

		// set our ti.App
		SharedValue value = Value::NewObject(this->app_binding);
		host->GetGlobalObject()->Set("App",value);

		std::string appid = AppConfig::Instance()->GetAppID();
		std::string app_properties = kroll::FileUtils::GetApplicationDataDirectory(appid);

		Poco::File app_properties_dir(app_properties);
		if (!app_properties_dir.exists()) {
			app_properties_dir.createDirectories();
		}

		app_properties += KR_PATH_SEP;
		app_properties += "application.properties";

		this->properties_binding = new PropertiesBinding(app_properties);
		SharedValue properties_value = Value::NewObject(this->properties_binding);
		/**
		 * @tiapi(property=True,type=object,name=App.Properties) returns the application's private properties
		 */
		this->app_binding->Set("Properties", properties_value);
	}

	void AppModule::Stop()
	{
	}
}
