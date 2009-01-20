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
	void OSXDesktop::CreateShortcut(const ValueList& args, SharedValue result)
	{
		// http://www.mail-archive.com/cocoa-dev@lists.apple.com/msg18273.html
		
		NSString* originalPath = [NSString stringWithCString:args.at(0)->ToString()];
		NSString* destPath = [NSString stringWithCString:args.at(1)->ToString()];
		
		NSMutableString *source = [NSMutableString stringWithString:@"tell application \"Finder\"\n"];

		[source appendFormat:@"set theAlias to make alias at POSIX file \"%@\" to POSIX file \"%@\"\n", NSTemporaryDirectory(), [originalPath stringByExpandingTildeInPath]];
		[source appendFormat:@"get POSIX path of (theAlias as string)\n"];
		[source appendFormat:@"end tell"];
		
		NSAppleScript *script = [[[NSAppleScript alloc] initWithSource:source] autorelease];

		NSDictionary *error = nil;
		NSAppleEventDescriptor *desc = [script executeAndReturnError:&error];
		
		if (desc==nil)
		{
			//TODO: throw exception?
			result->SetBool(false);
		}
		else
		{
			BOOL worked = [[NSFileManager defaultManager] movePath:[desc stringValue] toPath:[destPath stringByExpandingTildeInPath] handler:nil];
			result->SetBool(worked);
		}
	}
	void OSXDesktop::OpenFiles(const ValueList& args, SharedValue result)
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
	}
	void OSXDesktop::OpenApplication(const ValueList& args, SharedValue result)
	{
		NSWorkspace * ws = [NSWorkspace sharedWorkspace];
		BOOL wasLaunched = [ws launchApplication:[NSString stringWithCString:args.at(0)->ToString()]];
		result->SetBool(wasLaunched);
	}
	void OSXDesktop::OpenURL(const ValueList& args, SharedValue result)
	{
		NSWorkspace * ws = [NSWorkspace sharedWorkspace];
		BOOL wasOpened = [ws openURL:[NSURL URLWithString:[NSString stringWithCString:args.at(0)->ToString()]]];
		result->SetBool(wasOpened);
	}
	void OSXDesktop::GetSystemIdleTime(const ValueList& args, SharedValue result)
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
	}
}
