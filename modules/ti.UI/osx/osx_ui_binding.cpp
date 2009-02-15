/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "../ui_module.h"

namespace ti
{

	/*
	* NOTES:
	* Dynamic Setting Dock Menu / Image on OSX
	* http://developer.apple.com/samplecode/DeskPictAppDockMenu/index.html
	*/

	OSXUIBinding::OSXUIBinding(Host *host) : UIBinding(host)
	{

	}

	OSXUIBinding::~OSXUIBinding()
	{

	}

	SharedPtr<MenuItem> OSXUIBinding::CreateMenu(bool trayMenu)
	{
		//SharedPtr<MenuItem> menu = new OSXMenuItem(true);
		SharedPtr<MenuItem> menu = NULL;
		return menu;
	}

	void OSXUIBinding::SetMenu(SharedPtr<MenuItem>)
	{
	}

	void OSXUIBinding::SetContextMenu(SharedPtr<MenuItem>)
	{
	}

	void OSXUIBinding::SetDockIcon(SharedString icon_path)
	{

	}

	void OSXUIBinding::SetDockMenu(SharedPtr<MenuItem>)
	{
	}

	void OSXUIBinding::SetBadge(SharedString badge_path)
	{
	}

	void OSXUIBinding::SetIcon(SharedString icon_path)
	{
	}

	SharedPtr<TrayItem> OSXUIBinding::AddTray(
		SharedString icon_path,
		SharedBoundMethod cb)
	{
		SharedPtr<TrayItem> item = NULL;
		return item;
	}

	void OSXUIBinding::RemoveTray()
	{
		// TODO
	}

	void OSXUIBinding::OpenFiles(
		SharedBoundMethod callback,
		bool multiple,
		bool files,
		bool directories,
		std::string& path,
		std::string& file,
		std::vector<std::string>& types)
	{
		SharedBoundList results = new StaticBoundList();

		NSOpenPanel* openDlg = [NSOpenPanel openPanel];
		[openDlg setCanChooseFiles:files];
		[openDlg setCanChooseDirectories:directories];
		[openDlg setAllowsMultipleSelection:multiple];
		[openDlg setResolvesAliases:YES];

		NSMutableArray *filetypes = nil;
		NSString *begin = nil, *filename = nil;

		if (file != "")
		{
			filename = [NSString stringWithCString:file.c_str()];
		}
		if (path != "")
		{
			begin = [NSString stringWithCString:path.c_str()];
		}
		if (types.size() > 0)
		{
			filetypes = [[NSMutableArray alloc] init];
			for (size_t t = 0; t < types.size(); t++)
			{
				const char *s = types.at(t).c_str();
				[filetypes addObject:[NSString stringWithCString:s]];
			}
		}

		if ( [openDlg runModalForDirectory:begin file:filename types:filetypes] == NSOKButton )
		{
			NSArray* selected = [openDlg filenames];
			for (int i = 0; i < (int)[selected count]; i++)
			{
				NSString* fileName = [selected objectAtIndex:i];
				std::string fn = [fileName UTF8String];
				results->Append(Value::NewString(fn));
			}
		}
		[filetypes release];

		ValueList args;
		args.push_back(Value::NewList(results));
		callback->Call(args);
	}

	long OSXUIBinding::GetSystemIdleTime()
	{
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
			return -1;
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
			return -1;
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

			tHandle /= 1000000; // return as milliseconds
		}
		else
		{
			tHandle = -1;
		}

		CFRelease((CFTypeRef)properties);
		IOObjectRelease(curObj);
		IOObjectRelease(iter);
		return (long)tHandle;
	}

}
