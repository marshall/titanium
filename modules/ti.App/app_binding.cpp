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
	AppBinding::AppBinding(BoundObject *global) : global(global)
	{
		KR_ADDREF(global);
		this->SetMethod("getID",&AppBinding::GetID);
		this->SetMethod("getName",&AppBinding::GetName);
		this->SetMethod("getVersion",&AppBinding::GetVersion);
		this->SetMethod("getUpdateURL",&AppBinding::GetUpdateURL);
		this->SetMethod("getGUID",&AppBinding::GetGUID);
	}
	AppBinding::~AppBinding()
	{
		KR_DECREF(global);
	}
	void AppBinding::GetID(const ValueList& args, Value *result)
	{
		result->Set(AppConfig::Instance()->GetAppID());
	}
	void AppBinding::GetName(const ValueList& args, Value *result)
	{
		result->Set(AppConfig::Instance()->GetAppName());
	}
	void AppBinding::GetVersion(const ValueList& args, Value *result)
	{
		result->Set(AppConfig::Instance()->GetVersion());
	}
	void AppBinding::GetUpdateURL(const ValueList& args, Value *result)
	{
		result->Set(AppConfig::Instance()->GetUpdateSite());
	}
	void AppBinding::GetGUID(const ValueList& args, Value *result)
	{
		//FIXME: implement this
	}
}
