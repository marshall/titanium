/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#import "osx_user_window.h"
#define STUB() printf("Method is still a stub, %s:%i\n", __FILE__, __LINE__)

namespace ti
{
	static NSUInteger toWindowMask(WindowConfig *config)
	{
		NSUInteger mask = 0;
		if (config->IsUsingChrome() || config->IsFullScreen())
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

	OSXUserWindow::OSXUserWindow(Host *host, WindowConfig *config) : UserWindow(host,config), window(NULL), opened(false), closed(false), menu_wrapper(NULL)
	{
		[NSApplication sharedApplication];

		NSRect frame = NSMakeRect(config->GetX(), config->GetY(), config->GetWidth(), config->GetHeight());

		NSUInteger mask = toWindowMask(config);

		if (config->IsFullScreen())
		{
			frame = [[NSScreen mainScreen] frame];
		}

		window = [[NativeWindow alloc]
		        initWithContentRect:frame
		                  styleMask:mask
		                    backing:NSBackingStoreBuffered
		                      defer:false];
		[window setupDecorations:config host:host userwindow:this];
	}
	OSXUserWindow::~OSXUserWindow()
	{
		KR_DUMP_LOCATION
		
		window = nil; // don't release
		
		if (this->menu_wrapper)
		{
			delete this->menu_wrapper;
			this->menu_wrapper = NULL;
		}
		
		if (!closed)
		{
			UserWindow::Close(this);
		}
	}
	UserWindow* OSXUserWindow::WindowFactory(Host* host, WindowConfig* config)
	{
		return new OSXUserWindow(host, config);
	}
	void OSXUserWindow::Hide()
	{
		this->config->SetVisible(false);
		if (opened)
		{
			[window orderOut:nil];
		}
	}
	void OSXUserWindow::Show()
	{
		this->config->SetVisible(true);
		if (opened)
		{
		    [window makeKeyAndOrderFront:nil];	
		}
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
		return this->config->IsFullScreen();
	}
	std::string OSXUserWindow::GetId()
	{
		return this->config->GetID();
	}
	void OSXUserWindow::Open()
	{
		opened = true;
		[window open];
		UserWindow::Open(this);
	}
	void OSXUserWindow::Close()
	{
		opened = false;
		closed = true;
		[window close];
		UserWindow::Close(this);
	}
	double OSXUserWindow::GetX()
	{
		//FIXME
		return 0;
	}
	void OSXUserWindow::SetX(double x)
	{
		NSPoint p;
		p.x = x;
		p.y = this->GetY();
		config->SetX(x);
		[window setFrameTopLeftPoint:p];
	}
	double OSXUserWindow::GetY()
	{
		//FIXME
		return 0;
	}
	void OSXUserWindow::SetY(double y)
	{
		NSPoint p;
		p.x = this->GetX();
		p.y = y;
		config->SetY(y);
		[window setFrameTopLeftPoint:p];
	}
	double OSXUserWindow::GetWidth()
	{
		return [window frame].size.width;
	}
	void OSXUserWindow::SetWidth(double width)
	{
		NSRect frame = [window frame];
		BOOL display = config->IsVisible();
		frame.size.width = width;
		config->SetWidth(width);
		[window setFrame:frame display:display animate:display];
	}
	double OSXUserWindow::GetHeight()
	{
		return [window frame].size.height;
	}
	void OSXUserWindow::SetHeight(double height)
	{
		NSRect frame = [window frame];
		BOOL display = config->IsVisible();
		frame.size.height = height;
		config->SetHeight(height);
		[window setFrame:frame display:display animate:display];
	}
	double OSXUserWindow::GetMaxWidth() {
		return this->config->GetMaxWidth();
	}
	
	void OSXUserWindow::SetMaxWidth(double width) {
		this->config->SetMaxWidth(width);
		STUB();
	}
	
	double OSXUserWindow::GetMinWidth() {
		return this->config->GetMinWidth();
	}
	
	void OSXUserWindow::SetMinWidth(double width) {
		this->config->SetMinWidth(width);
		STUB();
	}
	
	double OSXUserWindow::GetMaxHeight() {
		return this->config->GetMaxHeight();
	}
	
	void OSXUserWindow::SetMaxHeight(double height) {
		this->config->SetMaxHeight(height);
		STUB();
	}
	
	double OSXUserWindow::GetMinHeight() {
		return this->config->GetMinHeight();
	}
	
	void OSXUserWindow::SetMinHeight(double height) {
		this->config->SetMinHeight(height);
		STUB();
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
	std::string OSXUserWindow::GetURL()
	{
		return this->config->GetURL();
	}
	void OSXUserWindow::SetURL(std::string& url)
	{
		this->config->SetURL(url);
		if (opened)
		{
			NSURL *nsurl = [NSURL URLWithString:[NSString stringWithCString:url.c_str()]];
			[[[window webView] mainFrame] loadRequest:[NSURLRequest requestWithURL:nsurl]];
		}
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
		config->SetFullScreen(fullscreen);
		[window setFullScreen:fullscreen];
	}

	void OSXUserWindow::SetUsingChrome(bool chrome)
	{
		this->config->SetUsingChrome(chrome);
	}

	void OSXUserWindow::SetMenu(SharedBoundList menu)
	{	
		if (this->menu_wrapper != NULL)
		{
			delete this->menu_wrapper;
		}
		
		// NOTE: we probably have to toggle this based on when
		// the window is focused or not

		this->menu_wrapper = new OSXMenuWrapper(menu,this->GetHost()->GetGlobalObject());
		NSMenu *nsmenu = this->menu_wrapper->getNSMenu();
		[NSApp setMainMenu:nsmenu];
	}

	SharedBoundList OSXUserWindow::GetMenu()
	{
		STUB();
		return NULL;
	}
	
	void OSXUserWindow::SetIcon(SharedString icon_path)
	{
		STUB();
	}
	
	SharedString OSXUserWindow::GetIcon()
	{
		STUB();
		return NULL;
	}

}
