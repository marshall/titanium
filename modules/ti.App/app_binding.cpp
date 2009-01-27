/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */	
#include <kroll/kroll.h>
#include <Poco/Environment.h>
#include "app_binding.h"
#include "app_config.h"

namespace ti
{
	AppBinding::AppBinding(SharedBoundObject global) : global(global)
	{
		this->SetMethod("getID", &AppBinding::GetID);
		this->SetMethod("getName", &AppBinding::GetName);
		this->SetMethod("getVersion", &AppBinding::GetVersion);
		this->SetMethod("getUpdateURL", &AppBinding::GetUpdateURL);
		this->SetMethod("getGUID", &AppBinding::GetGUID);
		this->SetMethod("appURLToPath", &AppBinding::AppURLToPath);
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

	static const char *kAppURLPrefix = "/Resources";
	void AppBinding::AppURLToPath(const ValueList& args, SharedValue result)
	{
		result->SetString("");

		if (args.size() < 0 || !args.at(0)->IsString())
			return;

		std::string url = std::string(args.at(0)->ToString());
		if (url.find("app://") == 0)
		{
			url = url.substr(6, url.length() - 6);
		}
		std::string path = Poco::Environment::get("KR_HOME", "");

		result->SetString(std::string(path + kAppURLPrefix + "/" + url).c_str());
	}

}
