/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include <kroll/kroll.h>
#include "growl_module.h"
#include "growl_test.h"

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
		CopyToApp();

		// load our variables
		this->variables = new GrowlBinding(host->GetGlobalObject());

		// set our ti.Growl
		SharedPtr<Value> value = new Value(this->variables);
		host->GetGlobalObject()->Set("Growl",value);
	}

	void GrowlModule::CopyToApp()
	{
#if defined(OS_OSX)
		std::string dir = host->GetApplicationHome() + KR_PATH_SEP + "Contents" +
			KR_PATH_SEP + "Frameworks" + KR_PATH_SEP + "Growl.framework";

		if (!FileUtils::IsDirectory(dir))
		{

			NSFileManager *fm = [NSFileManager defaultManager];
			NSString *src = [NSString stringWithFormat:@"%@/Resources/Growl.framework", GetPath()];
			NSString *dest = [NSString stringWithFormat:@"%@/Contents/Frameworks", host->GetApplicationHome().c_str()];
			[fm copyPath:src toPath:dest handler:nil];

			src = [NSString stringWithFormat:@"%@/Resources/Growl Registration Ticket.growlRegDict", GetPath()];
			dest = [NSString stringWithFormat:@"%@/Contents/Resources", host->GetApplicationHome().c_str()];
			[fm copyPath:src toPath:dest handler:nil];
		}
#endif
	}

	void GrowlModule::Destroy()
	{
	}

	void GrowlModule::Test()
	{
	  GrowlUnitTestSuite test;
	  test.Run(host);
	}
}
