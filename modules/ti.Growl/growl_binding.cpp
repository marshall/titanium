/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include <kroll/kroll.h>
#include "growl_binding.h"

#ifdef OS_OSX
#import "GrowlApplicationBridge.h"
#endif

namespace ti
{
	GrowlBinding::GrowlBinding(SharedBoundObject global) : global(global)
	{
		SetMethod("showNotification", &GrowlBinding::ShowNotification);
	}

	void GrowlBinding::ShowNotification(const ValueList& args, SharedValue result)
	{
		std::string title = args.at(0)->ToString();
		std::string description = args.at(1)->ToString();

#ifdef OS_OSX
		[GrowlApplicationBridge setGrowlDelegate:@""];
		[GrowlApplicationBridge
			 notifyWithTitle:[NSString stringWithCString:title.c_str()]
			 description:[NSString stringWithCString:description.c_str()]
			 notificationName:@"tiNotification"
			 iconData:nil
			 priority:0
			 isSticky:NO
			 clickContext:nil];
#endif
	}

	GrowlBinding::~GrowlBinding()
	{
	}
}
