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
		this->SetMethod("getID", &AppBinding::GetID);
		this->SetMethod("getName", &AppBinding::GetName);
		this->SetMethod("getVersion", &AppBinding::GetVersion);
		this->SetMethod("getUpdateURL", &AppBinding::GetUpdateURL);
		this->SetMethod("getGUID", &AppBinding::GetGUID);
		this->SetMethod("appURLToPath", &AppBinding::AppURLToPath);

		// FIXME: for now this version is hardcoded
		SharedValue version = Value::NewDouble(0.2);
		global->Set("version", version);

		// platform
		SharedValue platform = Value::NewString(Host::Platform);
		global->Set("platform",platform);

		SharedBoundList argList = new StaticBoundList();
		for (int i = 0; i < Host::GetInstance()->GetCommandLineArgCount(); i++) {
			argList->Append(Value::NewString(Host::GetInstance()->GetCommandLineArg(i)));
		}
		SharedValue arguments = Value::NewList(argList);
		Set("arguments", arguments);

		this->SetMethod("exit",&AppBinding::Exit);
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
		//FIXME: implement this
	}
	void AppBinding::Exit(const ValueList& args, SharedValue result)
	{
		host->Exit(args.size()==0 ? 0 : args.at(0)->ToInt());
	}

	static const char *kAppURLPrefix = "Resources";
	void AppBinding::AppURLToPath(const ValueList& args, SharedValue result)
	{
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
