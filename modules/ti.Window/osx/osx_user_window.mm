/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#import "osx_user_window.h"

namespace ti
{
	static NSUInteger toWindowMask(WindowConfig *config)
	{
		NSUInteger mask = 0;
		if (config->IsUsingChrome() || config->IsFullscreen())
		{
			mask = NSBorderlessWindowMask;
		}
		else
		{
			mask |= NSTitledWindowMask;
			if (config->IsResizable())
			{
				mask |= NSResizableWindowMask;
			}
			if (config->IsCloseable())
			{
				mask |= NSClosableWindowMask;
			}
			if (config->IsMaximizable())
			{
				mask |= NSMiniaturizableWindowMask;
			}
			if (config->IsMaximizable())
			{
				// handled in the window code
			}
		}
		return mask;
	}

	OSXUserWindow::OSXUserWindow(Host *host, WindowConfig *config) : UserWindow(host,config)
	{
		[NSApplication sharedApplication];

		NSRect frame = NSMakeRect(config->GetX(), config->GetY(), config->GetWidth(), config->GetHeight());

		NSUInteger mask = toWindowMask(config);

		if (config->IsFullscreen())
		{
			frame = [[NSScreen mainScreen] frame];
		}

		window = [[NativeWindow alloc]
		        initWithContentRect:frame
		                  styleMask:mask
		                    backing:NSBackingStoreBuffered
		                      defer:false];
		[window setupDecorations:config host:host];
	}
	OSXUserWindow::~OSXUserWindow()
	{
		[window release];
		window = nil;
	}
	void OSXUserWindow::Hide()
	{
		this->config->SetVisible(false);
		[window orderOut:nil];
	}
	void OSXUserWindow::Show()
	{
		this->config->SetVisible(true);
	    [window makeKeyAndOrderFront:nil];	
	}
	bool OSXUserWindow::IsUsingChrome()
	{
		return this->config->IsUsingChrome();
	}
	bool OSXUserWindow::IsUsingScrollbars()
	{
		return this->config->IsUsingScrollbars();
	}
	bool OSXUserWindow::IsFullScreen()
	{
		return this->config->IsFullscreen();
	}
	std::string OSXUserWindow::GetId()
	{
		return this->config->GetID();
	}
	void OSXUserWindow::Open()
	{
		//TODO: validate properties like URL
		[window open];
	}
	void OSXUserWindow::Close()
	{
		[window close];
	}
	double OSXUserWindow::GetX()
	{
		return 0;
	}
	void OSXUserWindow::SetX(double x)
	{
	}
	double OSXUserWindow::GetY()
	{
		return 0;
	}
	void OSXUserWindow::SetY(double y)
	{
	}
	double OSXUserWindow::GetWidth()
	{
		return [window frame].size.width;
	}
	void OSXUserWindow::SetWidth(double width)
	{
	}
	double OSXUserWindow::GetHeight()
	{
		return [window frame].size.height;
	}
	void OSXUserWindow::SetHeight(double height)
	{
	}
	Bounds OSXUserWindow::GetBounds()
	{
		Bounds b;
		b.width = this->GetWidth();
		b.height = this->GetHeight();
		b.x = this->GetX();
		b.y = this->GetY();
		return b;
	}
	void OSXUserWindow::SetBounds(Bounds bounds)
	{
		this->SetX(bounds.x);
		this->SetY(bounds.y);
		this->SetWidth(bounds.width);
		this->SetHeight(bounds.height);
	}
	std::string OSXUserWindow::GetTitle()
	{
		return this->config->GetTitle();
	}
	void OSXUserWindow::SetTitle(std::string& title)
	{
		this->config->SetTitle(title);
		[window setTitle:[NSString stringWithCString:this->config->GetTitle().c_str()]];
	}
	std::string OSXUserWindow::GetUrl()
	{
		return this->config->GetURL();
	}
	void OSXUserWindow::SetUrl(std::string& url)
	{
		this->config->SetURL(url);
		//TODO
	}
	bool OSXUserWindow::IsResizable()
	{
		return this->config->IsResizable();
	}
	void OSXUserWindow::SetResizable(bool resizable)
	{
		this->config->SetResizable(resizable);
	}
	bool OSXUserWindow::IsMaximizable()
	{
		return this->config->IsMaximizable();
	}
	void OSXUserWindow::SetMaximizable(bool maximizable)
	{
		this->config->SetMaximizable(maximizable);
		[[window standardWindowButton:NSWindowZoomButton] setHidden:!maximizable];
	}
	bool OSXUserWindow::IsMinimizable()
	{
		return this->config->IsMinimizable();
	}
	void OSXUserWindow::SetMinimizable(bool minimizable)
	{
		this->config->SetMinimizable(minimizable);
		[[window standardWindowButton:NSWindowMiniaturizeButton] setHidden:!minimizable];
	}
	bool OSXUserWindow::IsCloseable()
	{
		return this->config->IsCloseable();
	}
	void OSXUserWindow::SetCloseable(bool closeable)
	{
		this->config->SetCloseable(closeable);
		[[window standardWindowButton:NSWindowCloseButton] setHidden:!closeable];
	}
	bool OSXUserWindow::IsVisible()
	{
		return this->config->IsVisible();
	}
	void OSXUserWindow::SetVisible(bool visible)
	{
		this->config->SetVisible(visible);
		if (visible)
		{
			this->Show();
		}
		else
		{
			this->Hide();
		}
	}
	double OSXUserWindow::GetTransparency()
	{
		return this->config->GetTransparency();
	}
	void OSXUserWindow::SetTransparency(double transparency)
	{
		if (transparency < 0.0)
		{
			transparency = 1.0;
		}
		this->config->SetTransparency(transparency);
		[window setTransparency:transparency];
	}
	void OSXUserWindow::SetFullScreen(bool fullscreen)
	{
		[window setFullScreen:fullscreen];
	}
}
