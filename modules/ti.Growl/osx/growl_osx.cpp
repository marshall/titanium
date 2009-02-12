/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "growl_osx.h"
#import "GrowlApplicationBridge.h"
#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#include <sys/types.h>
#include <sys/stat.h>

using namespace kroll;
using namespace ti;

namespace ti {
	GrowlOSX::GrowlOSX(SharedBoundObject global) : GrowlBinding(global) {
		delegate = [[TiGrowlDelegate alloc] init];
		[delegate retain];
	}

	GrowlOSX::~GrowlOSX() {
		[delegate release];
	}

	void GrowlOSX::ShowNotification(std::string& title, std::string& description, std::string& iconURL, int notification_delay, SharedBoundMethod callback)
	{
		NSData *iconData = [NSData data];

		if (iconURL.size() > 0) {
			SharedValue iconPathValue = global->CallNS("App.appURLToPath", Value::NewString(iconURL));
			if (iconPathValue->IsString()) {
				std::string iconPath = iconPathValue->ToString();
				iconData = [NSData dataWithContentsOfFile:[NSString stringWithCString:iconPath.c_str()]];
			}
		}

		id clickContext = nil;
		if (!callback.isNull()) {
			clickContext = [[MethodWrapper alloc] initWithMethod:new SharedBoundMethod(callback)];
		}

		[GrowlApplicationBridge
			 notifyWithTitle:[NSString stringWithCString:title.c_str()]
			 description:[NSString stringWithCString:description.c_str()]
			 notificationName:@"tiNotification"
			 iconData:iconData
			 priority:0
			 isSticky:NO
			 clickContext:clickContext];
	}

	bool GrowlOSX::IsRunning()
	{
		return [delegate growlReady];
	}

	void GrowlOSX::CopyToApp(kroll::Host *host, kroll::Module *module)
	{
		std::string dir = host->GetApplicationHome() + KR_PATH_SEP + "Contents" +
			KR_PATH_SEP + "Frameworks" + KR_PATH_SEP + "Growl.framework";

		if (!FileUtils::IsDirectory(dir))
		{
			NSFileManager *fm = [NSFileManager defaultManager];
			NSString *src = [NSString stringWithFormat:@"%s/Resources/Growl.framework", module->GetPath()];
			NSString *dest = [NSString stringWithFormat:@"%s/Contents/Frameworks", host->GetApplicationHome().c_str()];
			[fm copyPath:src toPath:dest handler:nil];

			src = [NSString stringWithFormat:@"%s/Resources/Growl Registration Ticket.growlRegDict", module->GetPath()];
			dest = [NSString stringWithFormat:@"%s/Contents/Resources", host->GetApplicationHome().c_str()];
			[fm copyPath:src toPath:dest handler:nil];
		}
	}
}
