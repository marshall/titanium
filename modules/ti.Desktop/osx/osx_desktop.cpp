/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
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
	SharedBoundList OSXDesktop::OpenFiles(SharedBoundObject props)
	{
		NSOpenPanel* openDlg = [NSOpenPanel openPanel];
		[openDlg setCanChooseFiles:YES];
		[openDlg setCanChooseDirectories:NO]; 
		[openDlg setAllowsMultipleSelection:NO];
		[openDlg setResolvesAliases:YES];
		
		// pass in a set of properties with each key being
		// the name of the property and a boolean for its setting
		// example:
		//
		// var selected = Titanium.Desktop.openFiles({
		//    multiple:true,
		//    files:false,
		//    directories:true,
		//    types:['js','html']	
		// });
		//
		NSMutableArray *filetypes = nil;
		NSString *begin = nil, *filename = nil;
		SharedValue multiple = props->Get("multiple");
		if (!multiple->IsNull())
		{
			[openDlg setAllowsMultipleSelection:multiple->ToBool()];
		}
		SharedValue path = props->Get("path");
		if (!path->IsNull() && path->IsString())
		{
			begin = [NSString stringWithCString:path->ToString()];
		}
		SharedValue file = props->Get("filename");
		if (!file->IsNull() && file->IsString())
		{
			filename = [NSString stringWithCString:file->ToString()];
		}
		SharedValue files = props->Get("files");
		if (!files->IsNull())
		{
			[openDlg setCanChooseFiles:files->ToBool()];
		}
		SharedValue dirs = props->Get("directories");
		if (!dirs->IsNull())
		{
			[openDlg setCanChooseDirectories:dirs->ToBool()];
		}
		SharedValue types = props->Get("types");
		if (!types->IsNull() && types->IsList())
		{
			SharedBoundList list = types->ToList();
			if (list->Size()>0)
			{
				filetypes = [[NSMutableArray alloc] init]; 
				for (int c=0;c<list->Size();c++)
				{
					SharedValue v = list->At(c);
					if (v->IsString())
					{
						const char *s = v->ToString();
						[filetypes addObject:[NSString stringWithCString:s]];
					}
				}
			}
		}
		
		SharedBoundList results = new StaticBoundList();
		if ( [openDlg runModalForDirectory:begin file:filename types:filetypes] == NSOKButton )
		{
		    NSArray* selected = [openDlg filenames];
		    for( int i = 0; i < (int)[selected count]; i++ )
		    {
		        NSString* fileName = [selected objectAtIndex:i];
				std::string fn = [fileName UTF8String];
				SharedValue f = Value::NewString(fn.c_str());
				results->Append(f);
		    }
		}
		[filetypes release];
		return results;
	}
	bool OSXDesktop::OpenApplication(std::string& app)
	{
		NSWorkspace* ws = [NSWorkspace sharedWorkspace];
		BOOL wasLaunched = [ws launchApplication:[NSString stringWithCString:app.c_str()]];
		return wasLaunched;
	}
	bool OSXDesktop::OpenURL(std::string& url)
	{
		NSWorkspace * ws = [NSWorkspace sharedWorkspace];
		BOOL wasOpened = [ws openURL:[NSURL URLWithString:[NSString stringWithCString:url.c_str()]]];
		return wasOpened;
	}
	int OSXDesktop::GetSystemIdleTime()
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
		return (int)tHandle;
	}
}
