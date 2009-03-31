/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include <kroll/kroll.h>
#include <Poco/Environment.h>
#include "app_binding.h"
#include "app_config.h"
#include "Properties/properties_binding.h"

namespace ti
{
	AppBinding::AppBinding(Host *host,SharedBoundObject global) : host(host),global(global)
	{
		/**
		 * @tiapi(method=True,immutable=True,returns=string,name=App.getID) get the application id
		 */
		this->SetMethod("getID", &AppBinding::GetID);
		/**
		 * @tiapi(method=True,immutable=True,returns=string,name=App.getName) get the application name
		 */
		this->SetMethod("getName", &AppBinding::GetName);
		/**
		 * @tiapi(method=True,immutable=True,returns=string,name=App.getVersion) get the application version
		 */
		this->SetMethod("getVersion", &AppBinding::GetVersion);
		/**
		 * @tiapi(method=True,immutable=True,returns=string,name=App.getUpdateURL) get the application update URL
		 */
		this->SetMethod("getUpdateURL", &AppBinding::GetUpdateURL);
		/**
		 * @tiapi(method=True,immutable=True,returns=string,name=App.getGUID) get the application globally unique id
		 */
		this->SetMethod("getGUID", &AppBinding::GetGUID);
		/**
		 * @tiapi(method=True,immutable=True,returns=string,name=App.appURLToPath) get a full path from an application using app: URL
		 */
		this->SetMethod("appURLToPath", &AppBinding::AppURLToPath);
		
		/**
		 * @tiapi(property=True,immutable=True,type=string,name=App.path) get a full path to the application
		 */
#ifdef OS_OSX
		NSString *path = [[NSBundle mainBundle] bundlePath];
		this->Set("path",Value::NewString([path UTF8String]));
#else
		this->Set("path",Value::NewString(host->GetCommandLineArg(0)));
#endif

		/**
		 * @tiapi(property=True,immutable=True,type=double,name=App.version) returns the Titanium product version
		 */
		SharedValue version = Value::NewDouble(PRODUCT_VERSION);
		global->Set("version", version);

		/**
		 * @tiapi(property=True,immutable=True,type=string,name=App.platform) returns the Titanium platform
		 */
		SharedValue platform = Value::NewString(host->GetPlatform());
		global->Set("platform",platform);

		// skip the first argument which is the filepath to the
		// executable
		SharedBoundList argList = new StaticBoundList();
		for (int i = 1; i < Host::GetInstance()->GetCommandLineArgCount(); i++) {
			argList->Append(Value::NewString(Host::GetInstance()->GetCommandLineArg(i)));
		}
		SharedValue arguments = Value::NewList(argList);
		/**
		 * @tiapi(property=True,immutable=True,type=list,name=App.arguments) returns the arguments from the command line
		 */
		Set("arguments", arguments);

		/**
		 * @tiapi(method=True,immutable=True,returns=void,name=App.exit) causes the application to exit
		 */
		this->SetMethod("exit",&AppBinding::Exit);

		/**
		 * @tiapi(method=True,immutable=True,returns=list,name=App.loadProperties) load a properties list from a file path
		 */
		this->SetMethod("loadProperties", &AppBinding::LoadProperties);
	}

	AppBinding::~AppBinding()
	{
	}
	void AppBinding::GetID(const ValueList& args, SharedValue result)
	{
		result->SetString(AppConfig::Instance()->GetAppID().c_str());
	}
	void AppBinding::GetName(const ValueList& args, SharedValue result)
	{
		result->SetString(AppConfig::Instance()->GetAppName().c_str());
	}
	void AppBinding::GetVersion(const ValueList& args, SharedValue result)
	{
		result->SetString(AppConfig::Instance()->GetVersion().c_str());
	}
	void AppBinding::GetUpdateURL(const ValueList& args, SharedValue result)
	{
		result->SetString(AppConfig::Instance()->GetUpdateSite().c_str());
	}
	void AppBinding::GetGUID(const ValueList& args, SharedValue result)
	{
		std::string name = "KR_APP_GUID";
		
		if (Poco::Environment::has(name))
		{
			std::string value = Poco::Environment::get(name);
			result->SetString(value);
		}
		else
		{
			result->SetNull();
		}
	}
	void AppBinding::Exit(const ValueList& args, SharedValue result)
	{
		host->Exit(args.size()==0 ? 0 : args.at(0)->ToInt());
	}

	static const char *kAppURLPrefix = "Resources";
	void AppBinding::AppURLToPath(const ValueList& args, SharedValue result)
	{
		//FIXME - use FileUtils for this... so we can a common implementation
		
		result->SetString("");

		if (args.size() < 0 || !args.at(0)->IsString())
			return;

//FIXME: take into consider the appid which is in the host position of the URL
		std::string url = std::string(args.at(0)->ToString());
		if (url.find("app://") == 0)
		{
			url = url.substr(6, url.length() - 6);
		}
		std::string path = Poco::Environment::get("KR_HOME", "");

		result->SetString(std::string(path + KR_PATH_SEP + kAppURLPrefix + KR_PATH_SEP + url).c_str());
	}

	void AppBinding::LoadProperties(const ValueList& args, SharedValue result)
	{
		if (args.size() >= 1 && args.at(0)->IsString()) {
			std::string file_path = args.at(1)->ToString();
			SharedBoundObject properties = new PropertiesBinding(file_path);
			result->SetObject(properties);
		}
	}

}
