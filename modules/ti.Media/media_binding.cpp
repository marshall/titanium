/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "media_binding.h"
#include <kroll/kroll.h>
#include <cstring>
#include <Poco/URI.h>

namespace ti
{
	MediaBinding::MediaBinding(SharedBoundObject global) : global(global)
	{
		this->SetMethod("createSound", &MediaBinding::_CreateSound);
		this->SetMethod("beep", &MediaBinding::_Beep);
	}

	MediaBinding::~MediaBinding()
	{
	}

	void MediaBinding::_CreateSound(const ValueList& args, SharedValue result)
	{
		if (args.size()!=1)
			throw ValueException::FromString("createSound takes 1 parameter");

		std::string path(args.at(0)->ToString());
		result->SetObject(this->CreateSound(path));
	}

	void MediaBinding::_Beep(const ValueList& args, SharedValue result)
	{
		this->Beep();
	}

	std::string MediaBinding::GetResourcePath(const char *URL)
	{
		if (URL == NULL || !strcmp(URL, ""))
			return std::string();
		
		Poco::URI uri(URL);
		std::string scheme = uri.getScheme();

		if (scheme == "app" || scheme == "ti")
		{
			SharedValue meth_val = this->global->GetNS("App.appURLToPath");
			if (!meth_val->IsMethod())
				return std::string();

			SharedBoundMethod meth = meth_val->ToMethod();
			ValueList args;
			args.push_back(Value::NewString(URL));
			SharedValue out_val = meth->Call(args);

			if (out_val->IsString())
			{
				return std::string(out_val->ToString());
			}
			else
			{
				return std::string();
			}
		}
		else
		{
			return std::string(URL);
		}
	}
}
