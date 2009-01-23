/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */	
#include <kroll/kroll.h>
#include "app_binding.h"
#include "app_config.h"

namespace ti
{
	AppBinding::AppBinding(SharedBoundObject global) : global(global)
	{
		this->SetMethod("getID",&AppBinding::GetID);
		this->SetMethod("getName",&AppBinding::GetName);
		this->SetMethod("getVersion",&AppBinding::GetVersion);
		this->SetMethod("getUpdateURL",&AppBinding::GetUpdateURL);
		this->SetMethod("getGUID",&AppBinding::GetGUID);
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
}
