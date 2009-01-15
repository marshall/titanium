/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include <kroll/kroll.h>
#include "growl_module.h"
#include "growl_test.h"
#include <Poco/Path.h>

#if defined(OS_OSX)
#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

using namespace kroll;
using namespace ti;

namespace ti
{
	KROLL_MODULE(GrowlModule);

	void GrowlModule::Initialize()
	{
		// load our variables
		this->variables = new GrowlBinding(host->GetGlobalObject());

		// set our ti.Growl
		Value *value = new Value(this->variables);
		host->GetGlobalObject()->Set("Growl",value);
		KR_DECREF(value);
	}

	void GrowlModule::CopyToApp()
	{
		/*std::string dir = host->GetApplicationHome();

#if defined(OS_OSX)
		dir += PATH_SEP;
		dir += "Contents";
		dir += PATH_SEP;
		dir += "Frameworks";

		NSFileManager *fm = [NSFileManager defaultManager];
		if (!FileUtils::IsDirectory(dir))
		{
			mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		}

		NSString *src = [NSString stringWithFormat:@"%@/Resources/Growl.framework", host->GetModuleHome(this).c_str()];
		NSString *dest = [NSString stringWithFormat:@"%@/Contents/Frameworks", host->GetApplicationHome().c_str()];
		[fm copyPath:src toPath:dest handler:nil];

		src = [NSString stringWithFormat:@"%@/Resources/Growl Registration Ticket.growlRegDict", host->GetModuleHome(this).c_str()];
		dest = [NSString stringWithFormat:@"%@/Contents/Resources", host->GetApplicationHome().c_str()];
		[fm copyPath:src toPath:dest handler:nil];
#endif*/
	}

	void GrowlModule::Destroy()
	{
		KR_DECREF(this->variables);
	}

	void GrowlModule::Test()
	{
	  GrowlUnitTestSuite test;
	  test.Run(host);
	}
}
