/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "desktop_binding.h"
#include <kroll/kroll.h>

#ifdef OS_OSX
#import <Cocoa/Cocoa.h>
#elif defined(OS_WIN32)
#include <windows.h>
#include <shellapi.h>
#endif

namespace ti
{
	DesktopBinding::DesktopBinding(SharedBoundObject global) : global(global)
	{
		this->SetMethod("openApplication",&DesktopBinding::OpenApplication);
		this->SetMethod("openURL",&DesktopBinding::OpenURL);
		this->SetMethod("openFiles",&DesktopBinding::OpenFiles);
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
		if (args.size() > 0)
		{
			SharedValue properties = args.at(0);
			if (properties->IsObject())
			{
				SharedBoundObject props = properties->ToObject();
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
			}
		}
		
		SharedBoundList files = new StaticBoundList();
		if ( [openDlg runModalForDirectory:begin file:filename types:filetypes] == NSOKButton )
		{
		    NSArray* selected = [openDlg filenames];
		    for( int i = 0; i < (int)[selected count]; i++ )
		    {
		        NSString* fileName = [selected objectAtIndex:i];
				std::string fn = [fileName UTF8String];
				SharedValue f = Value::NewString(fn.c_str());
				files->Append(f);
		    }
		}
		[filetypes release];
		result->SetList(files);
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
#elif defined(OS_WIN32)
		long response = (long)ShellExecuteA(NULL, "open", args.at(0)->ToString(), NULL, NULL, SW_SHOWNORMAL);
		result->SetBool(response > 0);
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

			tHandle /= 1000000; // return as milliseconds
		}
		else
		{
			tHandle = -1;
		}

		CFRelease((CFTypeRef)properties);
		IOObjectRelease(curObj);
		IOObjectRelease(iter);
		result->SetInt((int)tHandle);
#elif defined(OS_WIN32)
		LASTINPUTINFO lii;
		memset(&lii, 0, sizeof(lii));

		lii.cbSize = sizeof(lii);
		::GetLastInputInfo(&lii);

		DWORD currentTickCount = GetTickCount();
		long idleTicks = currentTickCount - lii.dwTime;

		result->SetInt((int)idleTicks);
#endif
	}
}
