/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#import "ti_osx_user_window.h"

static NSUInteger toWindowMask(TiWindowConfig *config)
{
	NSUInteger mask = 0;
	if (config->IsUsingChrome())
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

TiOSXUserWindow::TiOSXUserWindow(TiHost *host, TiWindowConfig *config) : TiUserWindow(host,config)
{
	[NSApplication sharedApplication];
	
	NSRect frame = NSMakeRect(config->GetX(), config->GetY(), config->GetWidth(), config->GetHeight());
	
	NSUInteger mask = toWindowMask(config);
	
	window = [[NativeTiWindow alloc]
	        initWithContentRect:frame
	                  styleMask:mask
	                    backing:NSBackingStoreBuffered
	                      defer:false];
	[window setupDecorations:config host:host];
}
TiOSXUserWindow::~TiOSXUserWindow()
{
	[window release];
	window = nil;
}
void TiOSXUserWindow::Hide()
{
	this->config->SetVisible(false);
	[window orderOut:nil];
}
void TiOSXUserWindow::Show()
{
	this->config->SetVisible(true);
    [window makeKeyAndOrderFront:nil];	
}
bool TiOSXUserWindow::IsUsingChrome()
{
	return this->config->IsUsingChrome();
}
bool TiOSXUserWindow::IsUsingScrollbars()
{
	return this->config->IsUsingScrollbars();
}
bool TiOSXUserWindow::IsFullScreen()
{
	return this->config->IsFullscreen();
}
std::string TiOSXUserWindow::GetId()
{
	return this->config->GetID();
}
void TiOSXUserWindow::Open()
{
}
void TiOSXUserWindow::Close()
{
}
double TiOSXUserWindow::GetX()
{
	return 0;
}
void TiOSXUserWindow::SetX(double x)
{
}
double TiOSXUserWindow::GetY()
{
	return 0;
}
void TiOSXUserWindow::SetY(double y)
{
}
double TiOSXUserWindow::GetWidth()
{
	return 0;
}
void TiOSXUserWindow::SetWidth(double width)
{
}
double TiOSXUserWindow::GetHeight()
{
	return 0;
}
void TiOSXUserWindow::SetHeight(double height)
{
}
TiBounds TiOSXUserWindow::GetBounds()
{
	TiBounds b;
	return b;
}
void TiOSXUserWindow::SetBounds(TiBounds bounds)
{
}
std::string TiOSXUserWindow::GetTitle()
{
	return this->config->GetTitle();
}
void TiOSXUserWindow::SetTitle(std::string title)
{
	this->config->SetTitle(title);
	[window setTitle:[NSString stringWithCString:this->config->GetTitle().c_str()]];
}
std::string TiOSXUserWindow::GetUrl()
{
	return this->config->GetURL();
}
void TiOSXUserWindow::SetUrl(std::string url)
{
	this->config->SetURL(url);
	//TODO
}
bool TiOSXUserWindow::IsResizable()
{
	return this->config->IsResizable();
}
void TiOSXUserWindow::SetResizable(bool resizable)
{
	this->config->SetResizable(resizable);
}
bool TiOSXUserWindow::IsMaximizable()
{
	return this->config->IsMaximizable();
}
void TiOSXUserWindow::SetMaximizable(bool maximizable)
{
	this->config->SetMaximizable(maximizable);
	[[window standardWindowButton:NSWindowZoomButton] setHidden:!maximizable];
}
bool TiOSXUserWindow::IsMinimizable()
{
	return this->config->IsMinimizable();
}
void TiOSXUserWindow::SetMinimizable(bool minimizable)
{
	this->config->SetMinimizable(minimizable);
	[[window standardWindowButton:NSWindowMiniaturizeButton] setHidden:!minimizable];
}
bool TiOSXUserWindow::IsCloseable()
{
	return this->config->IsCloseable();
}
void TiOSXUserWindow::SetCloseable(bool closeable)
{
	this->config->SetCloseable(closeable);
	[[window standardWindowButton:NSWindowCloseButton] setHidden:!closeable];
}
bool TiOSXUserWindow::IsVisible()
{
	return this->config->IsVisible();
}
void TiOSXUserWindow::SetVisible(bool visible)
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
double TiOSXUserWindow::GetTransparency()
{
	return this->config->GetTransparency();
}
void TiOSXUserWindow::SetTransparency(double transparency)
{
	if (transparency < 0.0)
	{
		transparency = 1.0;
	}
	this->config->SetTransparency(transparency);
	[window setTransparency:transparency];
}
