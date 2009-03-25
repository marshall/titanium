/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "osx_desktop.h"

namespace ti
{
	OSXDesktop::OSXDesktop()
	{
	}

	OSXDesktop::~OSXDesktop()
	{
	}

	bool OSXDesktop::OpenApplication(std::string& app)
	{
		NSDictionary *dictionary = [[NSProcessInfo processInfo] environment];
		NSMutableDictionary *remember = [[NSMutableDictionary alloc] init];
		// unset any KR_ environment variables and the library path
		// so it doesn't interfere with process we're launching 
		// (reset below)
		NSEnumerator * ourDictionaryKeyEnumerator = [dictionary keyEnumerator];
		while (id key = [ourDictionaryKeyEnumerator nextObject])
		{
			if ([key hasPrefix:@"KR_"])
			{
				[remember setObject:[dictionary objectForKey:key] forKey:key];
			}
		}
		[remember setObject:[dictionary objectForKey:@"DYLD_LIBRARY_PATH"] forKey:@"DYLD_LIBRARY_PATH"];
		
		NSWorkspace* ws = [NSWorkspace sharedWorkspace];
		NSString *name = [NSString stringWithCString:app.c_str()];
#ifdef DEBUG
		NSLog(@"launching external app: %@",name);
#endif
		BOOL result = [ws launchApplication:name];

		NSEnumerator * ourRememberKeyEnumerator = [remember keyEnumerator];
		while (id key = [ourRememberKeyEnumerator nextObject])
		{
			id value = [remember objectForKey:key];
			setenv([key UTF8String],[value UTF8String],1);
		}
#ifdef DEBUG
		NSLog(@"launched external app, returned = %d",result);
#endif
		[remember release];
		return result;
	}

	bool OSXDesktop::OpenURL(std::string& url)
	{
		NSWorkspace * ws = [NSWorkspace sharedWorkspace];
		BOOL wasOpened = [ws openURL:[NSURL URLWithString:[NSString stringWithCString:url.c_str()]]];
		return wasOpened;
	}
}
