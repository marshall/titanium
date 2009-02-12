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


	}

	GrowlOSX::~GrowlOSX() {
		// TODO Auto-generated destructor stub
	}

	void GrowlOSX::ShowNotification(std::string& title, std::string& description, std::string& iconURL, int notification_delay, SharedBoundMethod callback)
	{
		[GrowlApplicationBridge setGrowlDelegate:@""];
		[GrowlApplicationBridge
			 notifyWithTitle:[NSString stringWithCString:title.c_str()]
			 description:[NSString stringWithCString:description.c_str()]
			 notificationName:@"tiNotification"
			 iconData:nil
			 priority:0
			 isSticky:NO
			 clickContext:nil];
	}

	bool GrowlOSX::IsRunning()
	{
		//TODO: implement me http://growl.info/documentation/developer/implementing-growl.php?lang=cocoa
		return false;
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
