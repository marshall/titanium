/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#import "osx_user_window.h"
#import "osx_ui_binding.h"
#import "osx_menu_item.h"

#define STUB() printf("Method is still a stub, %s:%i\n", __FILE__, __LINE__)

namespace ti
{
	static unsigned int toWindowMask(WindowConfig *config)
	{
		unsigned int mask = 0;
		if (!config->IsUsingChrome() || config->IsFullScreen())
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

	OSXUserWindow::OSXUserWindow(Host *host, WindowConfig *config, SharedPtr<OSXUIBinding> binding) : UserWindow(host,config), window(NULL), opened(false), closed(false), binding(binding)
	{
		unsigned int mask = toWindowMask(config);

		NSRect mainFrame = [[NSScreen mainScreen] frame];
		NSRect frame = mainFrame;

		// Set up the size and position of the
		// window using our Set<...> methods so
		// we avoid duplicating the logic here.
		if (!config->IsFullScreen())
			frame = NSMakeRect(0, 0, 10, 10);

		window = [[NativeWindow alloc]
		           initWithContentRect: frame
		           styleMask: mask
		           backing: NSBackingStoreBuffered
		           defer: false];

		if (!config->IsFullScreen())
		{
			this->SetX(config->GetX());
			this->SetY(config->GetY());
			this->SetHeight(config->GetHeight());
			this->SetWidth(config->GetWidth());
		}

		[window setupDecorations:config host:host userwindow:this];
	}
	OSXUserWindow::~OSXUserWindow()
	{
		window = nil; // don't release
		
		if (!closed)
		{
			UserWindow::Close();
		}
	}
	UserWindow* OSXUserWindow::WindowFactory(Host* host, WindowConfig* config, SharedPtr<OSXUIBinding> binding)
	{
		return new OSXUserWindow(host, config, binding);
	}
	void OSXUserWindow::Hide()
	{
		this->config->SetVisible(false);
		if (opened)
		{
			this->Unfocus();
			[window fireWindowEvent:HIDDEN];
		}
	}
	void OSXUserWindow::Focus()
	{
		if (!focused)
		{
		    [window makeKeyAndOrderFront:nil];
			this->Focused();	
		}
	}
	void OSXUserWindow::Unfocus()
	{
		if (focused)
		{
			[window orderOut:nil];
			this->Unfocused();
		}
	}
	void OSXUserWindow::Show()
	{
		this->config->SetVisible(true);
		if (opened)
		{
			this->Focus();
			[window fireWindowEvent:SHOWN];
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
		KR_DUMP_LOCATION
		if (!closed)
		{
			opened = false;
			closed = true;
			[window close];
			UserWindow::Close();
		}
	}
	double OSXUserWindow::GetX()
	{
		return this->config->GetX();
	}
	
	void OSXUserWindow::SetX(double x)
	{
		NSRect winFrame = [[window screen] frame];
		if (x == UserWindow::CENTERED)
		{
			double width = config->GetWidth();
			x = (winFrame.size.width - width) / 2;
		}
		config->SetX(x);

		// Flip for cartesian coordinates and adjust for origin
		x = x + winFrame.origin.x;
		double y = winFrame.origin.y + winFrame.size.height - config->GetY();
		NSPoint p = NSMakePoint(x, y);
		[window setFrameTopLeftPoint:p];
	}
	double OSXUserWindow::GetY()
	{
		return this->config->GetY();
	}
	void OSXUserWindow::SetY(double y)
	{
		NSRect winFrame = [[window screen] frame];
		if (y == UserWindow::CENTERED)
		{
			double height = config->GetHeight();
			y = (winFrame.size.height - height) / 2;
		}
		config->SetY(y);

		// Flip for cartesian coordinates
		double x = config->GetX() + winFrame.origin.x;
		y = winFrame.origin.y + winFrame.size.height - y;
		NSPoint p = NSMakePoint(x, y);
		[window setFrameTopLeftPoint:p];

	}
	double OSXUserWindow::GetWidth()
	{
		return [[window contentView] frame].size.width;
	}
	void OSXUserWindow::SetWidth(double width)
	{
		config->SetWidth(width);
		// Compensate for frame size
		NSRect frame = [window frame];
		int diff = frame.size.width - [[window contentView] frame].size.width;
		printf("width diff %i\n", diff);
		width += frame.size.width - [[window contentView] frame].size.width;
		BOOL display = config->IsVisible();
		frame.size.width = width;
		[window setFrame:frame display:display animate:display];
	}
	double OSXUserWindow::GetHeight()
	{
		return [[window contentView] frame].size.height;
	}
	void OSXUserWindow::SetHeight(double height)
	{
		config->SetHeight(height);

		// Compensate for frame size
		NSRect frame = [window frame];
		int diff = frame.size.height - [[window contentView] frame].size.height;
		printf("height diff %i\n", diff);
		height += frame.size.height - [[window contentView] frame].size.height;
		BOOL display = config->IsVisible();
		double originalHeight = NSHeight(frame);
		frame.size.height = height;
		NSPoint origin = frame.origin;
		origin.y += (originalHeight - height);
		[window setFrame: NSMakeRect(origin.x, origin.y, frame.size.width, height) display:display animate:YES];
	}
	double OSXUserWindow::GetMaxWidth() {
		return this->config->GetMaxWidth();
	}
	
	void OSXUserWindow::SetMaxWidth(double width) {
		this->config->SetMaxWidth(width);
		this->ReconfigureWindowConstraints();
	}
	
	double OSXUserWindow::GetMinWidth() {
		return this->config->GetMinWidth();
	}
	
	void OSXUserWindow::ReconfigureWindowConstraints()
	{
		NSSize min_size, max_size;
		min_size.width = this->GetMinWidth();
		min_size.height = this->GetMinHeight();
		max_size.width = this->GetMaxWidth();
		max_size.height = this->GetMaxHeight();

		[window setContentMinSize: min_size];
		[window setContentMaxSize: max_size];
	}

	void OSXUserWindow::SetMinWidth(double width) {
		this->config->SetMinWidth(width);
		this->ReconfigureWindowConstraints();
	}
	
	double OSXUserWindow::GetMaxHeight() {
		return this->config->GetMaxHeight();
	}
	
	void OSXUserWindow::SetMaxHeight(double height) {
		this->config->SetMaxHeight(height);
		this->ReconfigureWindowConstraints();
	}
	
	double OSXUserWindow::GetMinHeight() {
		return this->config->GetMinHeight();
	}
	
	void OSXUserWindow::SetMinHeight(double height) {
		this->config->SetMinHeight(height);
		this->ReconfigureWindowConstraints();
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
			std::string url_str = AppConfig::Instance()->InsertAppIDIntoURL(config->GetURL());
			NSURL* url = [NSURL URLWithString: [NSString stringWithCString:url_str.c_str()]];
			[[[window webView] mainFrame] loadRequest:[NSURLRequest requestWithURL:url]];
		}
	}
	bool OSXUserWindow::IsResizable()
	{
		return this->config->IsResizable();
	}
	void OSXUserWindow::SetResizable(bool resizable)
	{
		this->config->SetResizable(resizable);
		[window setShowsResizeIndicator:resizable];
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

	void OSXUserWindow::SetMenu(SharedPtr<MenuItem> menu)
	{	
		if (menu == this->menu)
		{
			return;
		}
		this->menu = menu;
		if (focused)
		{
			SharedPtr<OSXMenuItem> m = menu.cast<OSXMenuItem>();
			this->binding->WindowFocused(this,m.get());
		}
	}
	

	SharedPtr<MenuItem> OSXUserWindow::GetMenu()
	{
		return this->menu;
	}
	
	void OSXUserWindow::Focused()
	{
		this->focused = true;
		if (!menu.isNull())
		{
			SharedPtr<OSXMenuItem> m = menu.cast<OSXMenuItem>();
			this->binding->WindowFocused(this,m.get());
		}
	}

	void OSXUserWindow::Unfocused()
	{
		this->focused = false;
		if (!menu.isNull())
		{
			SharedPtr<OSXMenuItem> m = menu.cast<OSXMenuItem>();
			this->binding->WindowUnfocused(this,m.get());
		}
	}
	
	void OSXUserWindow::SetContextMenu(SharedPtr<MenuItem> value)
	{
		this->context_menu = value;
	}

	SharedPtr<MenuItem> OSXUserWindow::GetContextMenu()
	{
		return this->context_menu;
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

	bool OSXUserWindow::IsTopMost()
	{
		return this->topmost;
	}
	
	void OSXUserWindow::SetTopMost(bool topmost)
	{
		if (topmost)
		{
			[window setLevel:NSPopUpMenuWindowLevel];
			this->topmost = true;
		}
		else
		{
			[window setLevel:NSNormalWindowLevel];
			this->topmost = false;
		}
	}
	
	void OSXUserWindow::OpenFiles(
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
		this->Show();
	}

}
    
