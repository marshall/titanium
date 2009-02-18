/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#include "../ui_module.h"
#include "osx_menu_item.h"
#include "osx_tray_item.h"

namespace ti
{
	OSXUIBinding::OSXUIBinding(Host *host) : UIBinding(host)
	{
		[TiProtocol registerSpecialProtocol];
		[AppProtocol registerSpecialProtocol];
		application = [[TiApplication alloc] initWithBinding:this];
		NSApplication *nsapp = [NSApplication sharedApplication];
		[nsapp setDelegate:application];
		[NSBundle loadNibNamed:@"MainMenu" owner:nsapp];
	}

	OSXUIBinding::~OSXUIBinding()
	{
		[application release];
		[appDockMenu release];
		[savedDockView release];
	}

	SharedPtr<MenuItem> OSXUIBinding::CreateMenu(bool trayMenu)
	{
		return new OSXMenuItem();
	}

	void OSXUIBinding::SetMenu(SharedPtr<MenuItem> menu)
	{
		this->menu = menu;
	}

	void OSXUIBinding::SetContextMenu(SharedPtr<MenuItem> menu)
	{
		this->contextMenu = menu;
	}

	void OSXUIBinding::SetDockIcon(SharedString badge_path)
	{
		NSDockTile *dockTile = [NSApp dockTile];
		std::string value = *badge_path;
		if (!value.empty())
		{
			// remember the old one
			if (!savedDockView)
			{
				savedDockView = [dockTile contentView];
				[savedDockView retain];
			}
		   	// setup our image view for the dock tile
		   	NSRect frame = NSMakeRect(0, 0, dockTile.size.width, dockTile.size.height);
		   	NSImageView *dockImageView = [[NSImageView alloc] initWithFrame: frame];

			NSImage *image = MakeImage(value);
		   	[dockImageView setImage:image];
			[image release];
			
		   	// by default, add it to the NSDockTile
		   	[dockTile setContentView: dockImageView];
		}
		else if (savedDockView)
		{
		   	[dockTile setContentView:savedDockView];
			[savedDockView release];
			savedDockView = nil;
		}
		else
		{
		   	[dockTile setContentView:nil];
		}
	   	[dockTile display];
	}
	
	NSImage* OSXUIBinding::MakeImage(std::string value)
	{
		if (ti::UIModule::IsResourceLocalFile(value) || FileUtils::IsFile(value))
		{
			SharedString file = ti::UIModule::GetResourcePath(value.c_str());
			NSString *path = [NSString stringWithCString:((*file).c_str())];
			return [[NSImage alloc] initWithContentsOfFile:path];
		}
		else
		{
			NSURL *url = [NSURL URLWithString:[NSString stringWithCString:value.c_str()]];
			return [[NSImage alloc] initWithContentsOfURL:url];
		}
	}
	
	NSMenu* OSXUIBinding::MakeMenu(SharedPtr<MenuItem> menu_item)
	{
		SharedPtr<OSXMenuItem> item = menu_item.cast<OSXMenuItem>();
		const char *label = item->GetLabel();
		NSString *title = label == NULL ? @"" : [NSString stringWithCString:label];
		NSMenu *menu = [[NSMenu alloc] initWithTitle:title];
		int count = item->GetChildCount();
		for (int c=0;c<count;c++)
		{
			OSXMenuItem *i = item->GetChild(c);
			if (i->IsEnabled())
			{
				NSMenuItem *mi = i->CreateNative();
				[menu addItem:mi];
			}
		}
		return menu;
	}

	void OSXUIBinding::SetDockMenu(SharedPtr<MenuItem> menu)
	{
		this->dockMenu = menu;
	}

	void OSXUIBinding::SetBadge(SharedString badge_label)
	{
		std::string value = *badge_label;
		NSString *label = @"";
		if (!value.empty())
		{
			label = [NSString stringWithCString:value.c_str()];
		}
		NSDockTile *tile = [[NSApplication sharedApplication] dockTile];
		[tile setBadgeLabel:label];
	}
	
	void OSXUIBinding::SetBadgeImage(SharedString badge_path)
	{
		//TODO: need to support allowing custom badge images
	}

	void OSXUIBinding::SetIcon(SharedString path)
	{
		std::string icon_path = *path;
		if (icon_path.empty())
		{
			[[NSApplication sharedApplication] setApplicationIconImage:nil];
		}
		else
		{
			NSImage *image = MakeImage(icon_path);
			[[NSApplication sharedApplication] setApplicationIconImage:image];
			[image release];
		}
	}
	
	SharedPtr<MenuItem> OSXUIBinding::GetDockMenu()
	{
		return this->dockMenu;
	}
	
	SharedPtr<MenuItem> OSXUIBinding::GetMenu()
	{
		return this->menu;
	}

	SharedPtr<MenuItem> OSXUIBinding::GetContextMenu()
	{
		return this->contextMenu;
	}
	
	SharedPtr<TrayItem> OSXUIBinding::AddTray(
		SharedString icon_path,
		SharedBoundMethod cb)
	{
		return new OSXTrayItem(icon_path,cb);
	}

	long OSXUIBinding::GetIdleTime()
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
