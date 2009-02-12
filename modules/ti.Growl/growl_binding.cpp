/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include <kroll/kroll.h>
#include "growl_binding.h"

namespace ti
{
	GrowlBinding::GrowlBinding(SharedBoundObject global) : global(global)
	{
		SetMethod("showNotification", &GrowlBinding::ShowNotification);
		SetMethod("isRunning", &GrowlBinding::IsRunning);
	}

	void GrowlBinding::ShowNotification(const ValueList& args, SharedValue result)
	{
		if (args.size() >= 2) {
			std::string title = args.at(0)->ToString();
			std::string description = args.at(1)->ToString();
			std::string iconURL = "";
			int notification_timeout = 3;

			if (args.size() >= 3 && args.at(2)->IsString()) {
				iconURL = args.at(2)->ToString();
			}
			if (args.size() >= 4 && args.at(3)->IsNumber()) {
				notification_timeout = args.at(3)->ToInt();
			}

			SharedBoundMethod callback;
			ShowNotification(title, description, iconURL, notification_timeout, callback);
		}
	}

	void GrowlBinding::IsRunning(const ValueList& args, SharedValue result)
	{
		result->SetBool(IsRunning());
	}

	GrowlBinding::~GrowlBinding()
	{
	}
}
