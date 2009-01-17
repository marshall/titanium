/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "desktop_binding.h"
#include <kroll/kroll.h>

#ifdef OS_OSX
#import <Cocoa/Cocoa.h>
#endif

namespace ti
{
	DesktopBinding::DesktopBinding(SharedBoundObject global) : global(global)
	{
		this->SetMethod("openApplication",&DesktopBinding::OpenApplication);
		this->SetMethod("openURL",&DesktopBinding::OpenURL);
		this->SetMethod("getSystemIdleTime",&DesktopBinding::GetSystemIdleTime);
	}
	DesktopBinding::~DesktopBinding()
	{
	}
	void DesktopBinding::CreateShortcut(const ValueList& args, SharedValue result)
	{
#ifdef OS_OSX
		// http://www.mail-archive.com/cocoa-dev@lists.apple.com/msg18273.html
#endif
	}
	void DesktopBinding::OpenFiles(const ValueList& args, SharedValue result)
	{
#ifdef OS_OSX
#endif
	}
	void DesktopBinding::OpenApplication(const ValueList& args, SharedValue result)
	{
#ifdef OS_OSX
		NSWorkspace * ws = [NSWorkspace sharedWorkspace];
		BOOL wasLaunched = [ws launchApplication:[NSString stringWithCString:args.at(0)->ToString()]];
		result->SetBool(wasLaunched);
#endif
	}
	void DesktopBinding::OpenURL(const ValueList& args, SharedValue result)
	{
#ifdef OS_OSX
		NSWorkspace * ws = [NSWorkspace sharedWorkspace];
		BOOL wasOpened = [ws openURL:[NSURL URLWithString:[NSString stringWithCString:args.at(0)->ToString()]]];
		result->SetBool(wasOpened);
#endif
	}
	void DesktopBinding::GetSystemIdleTime(const ValueList& args, SharedValue result)
	{
#ifdef OS_OSX
		// some of the code for this was from:
		// http://ryanhomer.com/blog/2007/05/31/detecting-when-your-cocoa-application-is-idle/
		CFMutableDictionaryRef properties = 0;
		CFTypeRef obj;
		mach_port_t masterPort;
		io_iterator_t iter;
		io_registry_entry_t curObj;

		IOMasterPort(MACH_PORT_NULL, &masterPort);

		/* Get IOHIDSystem */
		IOServiceGetMatchingServices(masterPort, IOServiceMatching("IOHIDSystem"), &iter);
		if (iter == 0)
		{
			result->SetInt(-1);
			return;
		}
		else
		{
			curObj = IOIteratorNext(iter);
		}
		if (IORegistryEntryCreateCFProperties(curObj, &properties, kCFAllocatorDefault, 0) == KERN_SUCCESS && properties != NULL)
		{
			obj = CFDictionaryGetValue(properties, CFSTR("HIDIdleTime"));
			CFRetain(obj);
		}
		else
		{
			result->SetInt(-1);
			return;
		}

		uint64_t tHandle = 0;
		if (obj)
		{
			CFTypeID type = CFGetTypeID(obj);

			if (type == CFDataGetTypeID())
			{
				CFDataGetBytes((CFDataRef) obj, CFRangeMake(0, sizeof(tHandle)), (UInt8*) &tHandle);
			}
			else if (type == CFNumberGetTypeID())
			{
				CFNumberGetValue((CFNumberRef)obj, kCFNumberSInt64Type, &tHandle);
			}
			else
			{
				// error
				tHandle = 0;
			}

			CFRelease(obj);

			tHandle >>= 30; // essentially divides by 10^9 (nanoseconds)
		}
		else
		{
			tHandle = -1;
		}

		CFRelease((CFTypeRef)properties);
		IOObjectRelease(curObj);
		IOObjectRelease(iter);
		result->SetInt((int)tHandle);
#endif
	}
}
