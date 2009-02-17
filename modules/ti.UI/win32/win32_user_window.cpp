/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "win32_user_window.h"
#include "webkit_frame_load_delegate.h"
#include "webkit_ui_delegate.h"
#include "webkit_policy_delegate.h"
#include "win32_tray_item.h"
#include "string_util.h"
#include "../url/app_url.h"
#include <math.h>
#include <shellapi.h>
#include <comutil.h>

#define STUB() printf("Method is still a stub, %s:%i\n", __FILE__, __LINE__)
#define SetFlag(x,flag,b) ((b) ? x |= flag : x &= ~flag)
#define UnsetFlag(x,flag) (x &= ~flag)=

using namespace ti;

static void* SetWindowUserData(HWND hwnd, void* user_data) {
	return
		reinterpret_cast<void*>(SetWindowLongPtr(hwnd, GWLP_USERDATA,
			reinterpret_cast<LONG_PTR>(user_data)));
}

static void* GetWindowUserData(HWND hwnd) {
	return reinterpret_cast<void*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
}

/*static*/
Win32UserWindow* Win32UserWindow::FromWindow(HWND hWnd)
{
	return reinterpret_cast<Win32UserWindow*>(GetWindowUserData(hWnd));
}


const TCHAR *windowClassName = "Win32UserWindow";

/*static*/
void Win32UserWindow::RegisterWindowClass (HINSTANCE hInstance)
{
	static bool class_initialized = false;
	if (!class_initialized) {
		//LoadString(hInstance, IDC_TIUSERWINDOW, windowClassName, 100);

		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);

		wcex.style			= CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc	= Win32UserWindow::WndProc;
		wcex.cbClsExtra		= 0;
		wcex.cbWndExtra		= 0;
		wcex.hInstance		= hInstance;
		wcex.hIcon			= 0;
		wcex.hIconSm		= 0;
		wcex.hCursor		= LoadCursor(hInstance, IDC_ARROW);
		wcex.hbrBackground	= (HBRUSH)(COLOR_BACKGROUND+1);
		wcex.lpszMenuName	= "";
		wcex.lpszClassName	= windowClassName;

		ATOM result = RegisterClassEx(&wcex);
		if (result == NULL) {
			std::cout << "Error Registering Window Class: " << GetLastError() << std::endl;
		}

		class_initialized = true;
	}
}

void Win32UserWindow::AddMessageHandler(const ValueList& args, SharedValue result)
{
	if (args.size() < 2 || !args.at(0)->IsNumber() || !args.at(1)->IsMethod())
		return;

	long messageCode = (long)args.at(0)->ToDouble();
	SharedBoundMethod callback = args.at(1)->ToMethod();

	messageHandlers[messageCode] = callback;
}

/*static*/
LRESULT CALLBACK
Win32UserWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	Win32UserWindow *window = Win32UserWindow::FromWindow(hWnd);

	if (window && (window->messageHandlers.size() > 0) && (window->messageHandlers.find(message) != window->messageHandlers.end()))
	{
		SharedBoundMethod handler = window->messageHandlers[message];
		ValueList args;
		handler->Call(args);

		return 0;
	}

	switch (message)
	{
		case WM_DESTROY:
			PostQuitMessage(0);
			window->Close();
			break;
		case WM_CLOSE:
			window->FireEvent(CLOSED);
			return DefWindowProc(hWnd, message, wParam, lParam);
		case WM_SIZE:
			if (!window->web_view) break;
			window->ResizeSubViews();
			if (wParam == SIZE_MAXIMIZED) {
				window->FireEvent(MAXIMIZED);
			} else if (wParam == SIZE_MINIMIZED) {
				window->FireEvent(MINIMIZED);
			} else if (wParam == SIZE_RESTORED) {
				window->FireEvent(RESIZED);
			}
			break;
		case WM_SETFOCUS:
			window->FireEvent(FOCUSED);
			return DefWindowProc(hWnd, message, wParam, lParam);
		case WM_KILLFOCUS:
			window->FireEvent(UNFOCUSED);
			return DefWindowProc(hWnd, message, wParam, lParam);
		case WM_MOVE:
			window->FireEvent(MOVED);
			return DefWindowProc(hWnd, message, wParam, lParam);
		case WM_SHOWWINDOW:
			window->FireEvent(((BOOL)wParam) ? SHOWN : HIDDEN);
			return DefWindowProc(hWnd, message, wParam, lParam);


		case TI_TRAY_CLICKED:
			{
				UINT uMouseMsg = (UINT) lParam;
				if(uMouseMsg == WM_LBUTTONDOWN)
				{
					Win32TrayItem::InvokeLeftClickCallback(hWnd, message, wParam, lParam);
				}
				else if (uMouseMsg == WM_RBUTTONDOWN)
				{
					Win32TrayItem::ShowTrayMenu(hWnd, message, wParam, lParam);
				}
			}
			break;
		default:
			LRESULT handled = Win32MenuItemImpl::handleMenuClick(hWnd, message, wParam, lParam);

			if(! handled)
			{
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
	}

	return 0;
}

Win32UserWindow::Win32UserWindow(kroll::Host *host, WindowConfig *config)
	: UserWindow(host, config), script_evaluator(host), menuBarHandle(NULL), menuInUse(NULL), menu(NULL), contextMenuHandle(NULL), initial_icon(NULL), topmost(false)
{
	static bool initialized = false;
	win32_host = static_cast<kroll::Win32Host*>(host);
	if (!initialized) {
		INITCOMMONCONTROLSEX InitCtrlEx;

		InitCtrlEx.dwSize = sizeof(INITCOMMONCONTROLSEX);
		InitCtrlEx.dwICC  = 0x00004000; //ICC_STANDARD_CLASSES;
		InitCommonControlsEx(&InitCtrlEx);

		curl_register_local_handler(&Titanium_app_url_handler);
		addScriptEvaluator(&script_evaluator);
	}

	Win32UserWindow::RegisterWindowClass(win32_host->GetInstanceHandle());
	window_handle = CreateWindowA(windowClassName, config->GetTitle().c_str(),
			WS_CLIPCHILDREN,
			CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
			NULL, NULL, win32_host->GetInstanceHandle(), NULL);

	if (window_handle == NULL) {
		std::cout << "Error Creating Window: " << GetLastError() << std::endl;
	}
	std::cout << "window_handle = " << (int)window_handle << std::endl;

	// make our HWND available to 3rd party devs without needing our headers
	SharedValue windowHandle = Value::NewVoidPtr((void*)window_handle);
	this->Set("windowHandle", windowHandle);
	this->SetMethod("addMessageHandler", &Win32UserWindow::AddMessageHandler);

	this->ReloadTiWindowConfig();

	SetWindowUserData(window_handle, this);

	Bounds b;
	b.x = config->GetX();
	b.y = config->GetY();
	b.width = config->GetWidth();
	b.height = config->GetHeight();
	SetBounds(b);

	//web_view = WebView::createInstance();
	HRESULT hr = CoCreateInstance(CLSID_WebView, 0, CLSCTX_ALL, IID_IWebView, (void**)&web_view);
	if (FAILED(hr)) {
		std::cerr << "Error Creating WebView: ";
		if (hr == REGDB_E_CLASSNOTREG) std::cerr << "REGDB_E_CLASSNOTREG" << std::endl;
		else if (hr == CLASS_E_NOAGGREGATION) std::cerr << "CLASS_E_NOAGGREGATION" << std::endl;
		else if (hr == E_NOINTERFACE) std::cerr << "E_NOINTERFACE" << std::endl;
		else if (hr == E_UNEXPECTED) std::cerr << "E_UNEXPECTED" << std::endl;
		else if (hr == E_OUTOFMEMORY) std::cerr << "E_OUTOFMEMORY" << std::endl;
		else if (hr == E_INVALIDARG) std::cerr << "E_INVALIDARG" << std::endl;
		else fprintf(stderr, "Unknown Error? %x\n", hr);
	}

	// set the custom user agent for Titanium
	double version = host->GetGlobalObject()->Get("version")->ToDouble();
	char userAgent[128];
	sprintf(userAgent,"%s/%0.2f",PRODUCT_NAME,version);
	_bstr_t ua(userAgent);
	web_view->setApplicationNameForUserAgent(ua.copy());

	std::cout << "create frame load delegate " << std::endl;
	frameLoadDelegate = new Win32WebKitFrameLoadDelegate(this);
	uiDelegate = new Win32WebKitUIDelegate(this);
	policyDelegate = new Win32WebKitPolicyDelegate(this);

	std::cout << "set delegates, set host window, webview=" << (int)web_view  << std::endl;
	hr = web_view->setFrameLoadDelegate(frameLoadDelegate);
	hr = web_view->setUIDelegate(uiDelegate);
	hr = web_view->setPolicyDelegate(policyDelegate);
	hr = web_view->setHostWindow((OLE_HANDLE)window_handle);


	std::cout << "init with frame" << std::endl;
	RECT client_rect;
	GetClientRect(window_handle, &client_rect);
	hr = web_view->initWithFrame(client_rect, 0, 0);

	AppConfig *appConfig = AppConfig::Instance();
	std::string appid = appConfig->GetAppID();

	IWebPreferences *prefs = NULL;
	hr = CoCreateInstance(CLSID_WebPreferences, 0, CLSCTX_ALL, IID_IWebPreferences, (void**)&prefs);
	if (FAILED(hr) || prefs == NULL)
	{
		std::cerr << "Couldn't create the preferences object" << std::endl;
	}
	else
	{
		_bstr_t pi(appid.c_str());
		prefs->initWithIdentifier(pi.copy(),&prefs);

		prefs->setCacheModel(WebCacheModelDocumentBrowser);
		prefs->setPlugInsEnabled(true);
		prefs->setJavaEnabled(false);
		prefs->setJavaScriptEnabled(true);
		prefs->setDOMPasteAllowed(true);

		IWebPreferencesPrivate* privatePrefs = NULL;
		hr = prefs->QueryInterface(IID_IWebPreferencesPrivate, (void**)&privatePrefs);
		if (FAILED(hr))
		{
			std::cerr << "Failed to get private preferences" << std::endl;
		}
		else
		{
			privatePrefs->setDeveloperExtrasEnabled(host->IsDebugMode());
			privatePrefs->setDatabasesEnabled(true);
			privatePrefs->setLocalStorageEnabled(true);
			privatePrefs->setOfflineWebApplicationCacheEnabled(true);

			_bstr_t db_path(FileUtils::GetApplicationDataDirectory(appid).c_str());
			privatePrefs->setLocalStorageDatabasePath(db_path.copy());
			privatePrefs->Release();
		}

		web_view->setPreferences(prefs);
		prefs->Release();
	}

	// allow app:// and ti:// to run with local permissions (cross-domain ajax,etc)
	_bstr_t app_proto("app");
	web_view->registerURLSchemeAsLocal(app_proto.copy());

	_bstr_t ti_proto("ti");
	web_view->registerURLSchemeAsLocal(ti_proto.copy());

	IWebViewPrivate *web_view_private;
	hr = web_view->QueryInterface(IID_IWebViewPrivate, (void**)&web_view_private);
	hr = web_view_private->viewWindow((OLE_HANDLE*) &view_window_handle);
	web_view_private->Release();

	hr = web_view->mainFrame(&main_frame);

	std::cout << "resize subviews" << std::endl;
	ResizeSubViews();

	// ensure we have valid restore values
	restore_bounds = GetBounds();
	restore_styles = GetWindowLong(window_handle, GWL_STYLE);

	// set this flag to indicate that when the frame is loaded
	// we want to show the window - we do this to prevent white screen
	// while the URL is being fetched
	this->requires_display = true;

	// set initial window icon to icon associated with exe file
	char exePath[MAX_PATH];
	GetModuleFileNameA(GetModuleHandle(NULL), exePath, MAX_PATH);
	initial_icon = ExtractIcon(win32_host->GetInstanceHandle(), exePath, 0);
	if(initial_icon)
	{
		SendMessageA(window_handle, (UINT)WM_SETICON, ICON_BIG, (LPARAM)initial_icon);
	}
}

Win32UserWindow::~Win32UserWindow()
{
	if (web_view)
		web_view->Release();

	if (main_frame)
		main_frame->Release();
}

UserWindow* Win32UserWindow::WindowFactory(Host *host, WindowConfig* config)
{
	return new Win32UserWindow(host, config);
}

void Win32UserWindow::ResizeSubViews()
{
	RECT rcClient;
	GetClientRect(window_handle, &rcClient);
	MoveWindow(view_window_handle, 0, 0, rcClient.right, rcClient.bottom, TRUE);
}

HWND Win32UserWindow::GetWindowHandle() {
	return this->window_handle;
}

void Win32UserWindow::Hide() {
	ShowWindow(window_handle, SW_HIDE);
}

void Win32UserWindow::Show() {
	ShowWindow(window_handle, SW_SHOW);
}

void Win32UserWindow::Open() {
	std::cout << "Opening window_handle=" << (int)window_handle << ", view_window_handle="<<(int)view_window_handle<<std::endl;

	UpdateWindow(window_handle);
	UpdateWindow(view_window_handle);

	ResizeSubViews();

	UserWindow::Open(this);
	SetURL(this->config->GetURL());
	if (!this->requires_display)
	{
		ShowWindow(window_handle, SW_SHOW);
		ShowWindow(view_window_handle, SW_SHOW);
	}

	FireEvent(OPENED);
}

void Win32UserWindow::Close() {
	DestroyWindow(window_handle);

	UserWindow::Close();
}

double Win32UserWindow::GetX() {
	return GetBounds().x;
}

void Win32UserWindow::SetX(double x) {
	Bounds b = GetBounds();
	b.x = x;
	SetBounds(b);
}

double Win32UserWindow::GetY() {
	return GetBounds().y;
}

void Win32UserWindow::SetY(double y) {
	Bounds b = GetBounds();
	b.y = y;
	SetBounds(b);
}

double Win32UserWindow::GetWidth() {
	return GetBounds().width;
}

void Win32UserWindow::SetWidth(double width) {
	Bounds b = GetBounds();
	b.width = width;
	SetBounds(b);
}

double Win32UserWindow::GetHeight() {
	return GetBounds().height;
}

void Win32UserWindow::SetHeight(double height) {
	Bounds b = GetBounds();
	b.height = height;
	SetBounds(b);
}

double Win32UserWindow::GetMaxWidth() {
	return this->config->GetMaxWidth();
}

void Win32UserWindow::SetMaxWidth(double width) {
	this->config->SetMaxWidth(width);
	//STUB();
}

double Win32UserWindow::GetMinWidth() {
	return this->config->GetMinWidth();
}

void Win32UserWindow::SetMinWidth(double width) {
	this->config->SetMinWidth(width);
	//STUB();
}

double Win32UserWindow::GetMaxHeight() {
	return this->config->GetMaxHeight();
}

void Win32UserWindow::SetMaxHeight(double height) {
	this->config->SetMaxHeight(height);
	//STUB();
}

double Win32UserWindow::GetMinHeight() {
	return this->config->GetMinHeight();
}

void Win32UserWindow::SetMinHeight(double height) {
	this->config->SetMinHeight(height);
	//STUB();
}

Bounds Win32UserWindow::GetBounds() {
	Bounds bounds;

	RECT rect;
	GetWindowRect(window_handle, &rect);

	bounds.x = rect.left;
	bounds.y = rect.top;
	bounds.width = rect.right - rect.left;
	bounds.height = rect.bottom - rect.top;

	return bounds;
}

void Win32UserWindow::SetBounds(Bounds bounds) {
	HWND desktop = GetDesktopWindow();
	RECT desktopRect;
	GetWindowRect(desktop, &desktopRect);

	if (bounds.x == WindowConfig::DEFAULT_POSITION) {
		bounds.x = (desktopRect.right - bounds.width) / 2;
	}
	if (bounds.y == WindowConfig::DEFAULT_POSITION) {
		bounds.y = (desktopRect.bottom - bounds.height) / 2;
	}

	SetWindowPos(window_handle, NULL, bounds.x, bounds.y, bounds.width, bounds.height, SWP_SHOWWINDOW | SWP_NOZORDER);
}

void Win32UserWindow::SetTitle(std::string& title) {
	this->title = std::string(title);
	SetWindowText(window_handle, title.c_str());
}

void Win32UserWindow::SetURL(std::string& url_) {
	std::string url = url_;

	this->config->SetURL(url);

	std::cout << "SetURL: " << url << std::endl;

	IWebMutableURLRequest* request = 0;
	std::wstring method = L"GET";

	if (url.length() > 0 && (PathFileExists(url.c_str()) || PathIsUNC(url.c_str()))) {
		TCHAR fileURL[INTERNET_MAX_URL_LENGTH];
		DWORD fileURLLength = sizeof(fileURL)/sizeof(fileURL[0]);
		if (SUCCEEDED(UrlCreateFromPath(url.c_str(), fileURL, &fileURLLength, 0)))
			url = fileURL;
	}
	std::wstring wurl = UTF8ToWide(url);

	std::cout << "CoCreateInstance " << std::endl;
	HRESULT hr = CoCreateInstance(CLSID_WebMutableURLRequest, 0, CLSCTX_ALL, IID_IWebMutableURLRequest, (void**)&request);
	if (FAILED(hr))
		goto exit;

	std::cout << "initWithURL: " << url << std::endl;
	hr = request->initWithURL(SysAllocString(wurl.c_str()), WebURLRequestUseProtocolCachePolicy, 60);
	if (FAILED(hr))
		goto exit;

	std::cout << "set HTTP method" << std::endl;
	hr = request->setHTTPMethod(SysAllocString(method.c_str()));
	if (FAILED(hr))
		goto exit;

	std::cout << "load request" << std::endl;
	hr = main_frame->loadRequest(request);
	if (FAILED(hr))
		goto exit;

	std::cout << "set focus" << std::endl;
	SetFocus(view_window_handle);

exit:
	if (request)
		request->Release();
}


#define SetFlag(x,flag,b) ((b) ? x |= flag : x &= ~flag)
#define UnsetFlag(x,flag) (x &= ~flag)=

#define SetGWLFlag(wnd,flag,b) long window_style = GetWindowLong(wnd, GWL_STYLE);\
SetFlag(window_style, flag, b);\
SetWindowLong(wnd, GWL_STYLE, window_style);

void Win32UserWindow::SetResizable(bool resizable) {
	this->resizable = resizable;
	SetGWLFlag(window_handle, WS_OVERLAPPEDWINDOW, using_chrome && !resizable);
}

void Win32UserWindow::SetMaximizable(bool maximizable) {
	this->maximizable = maximizable;
	SetGWLFlag(window_handle, WS_MAXIMIZEBOX, maximizable);
}

void Win32UserWindow::SetMinimizable(bool minimizable) {
	this->minimizable = minimizable;
	SetGWLFlag(window_handle, WS_MINIMIZEBOX, minimizable);
}

void Win32UserWindow::SetCloseable(bool closeable) {
	this->closeable = closeable;
}

bool Win32UserWindow::IsVisible() {
	WINDOWPLACEMENT placement;
	placement.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(window_handle, &placement);

	return placement.showCmd == SW_SHOW;
}

void Win32UserWindow::SetVisible(bool visible) {
	this->showing = visible;
	ShowWindow(window_handle, visible ? SW_SHOW : SW_HIDE);
}

void Win32UserWindow::SetTransparency(double transparency) {
	this->transparency = transparency;
	SetLayeredWindowAttributes(window_handle, 0, (BYTE)floor(transparency*255), LWA_ALPHA);
}

void Win32UserWindow::SetFullScreen(bool fullscreen) {
	config->SetFullScreen(fullscreen);

	if (fullscreen) {
		restore_bounds = GetBounds();
		restore_styles = GetWindowLong(window_handle, GWL_STYLE);

		RECT desktopRect;
		if (SystemParametersInfoA(SPI_GETWORKAREA, 0, &desktopRect, NULL) == 1) {
			SetWindowLong(window_handle, GWL_STYLE, 0);
			SetWindowPos(window_handle, NULL, 0, 0, desktopRect.right - desktopRect.left, desktopRect.bottom - desktopRect.top, SWP_SHOWWINDOW);
		}
		FireEvent(FULLSCREENED);
	} else {
		SetWindowLong(window_handle, GWL_STYLE, restore_styles);
		SetBounds(restore_bounds);
		FireEvent(UNFULLSCREENED);
	}
}

void Win32UserWindow::SetMenu(SharedPtr<MenuItem> value)
{
	SharedPtr<Win32MenuItemImpl> menu = value.cast<Win32MenuItemImpl>();
	this->menu = menu;
	this->SetupMenu();
}

SharedPtr<MenuItem> Win32UserWindow::GetMenu()
{
	return this->menu;
}

void Win32UserWindow::SetContextMenu(SharedPtr<MenuItem> menu)
{
	SharedPtr<Win32MenuItemImpl> menu_new = menu.cast<Win32MenuItemImpl>();

	// if it's the same menu, don't do anything
	if((menu_new.isNull() && this->contextMenu.isNull()) || (menu_new == this->contextMenu))
	{
		return;
	}

	// remove old menu if needed
	if(! this->contextMenu.isNull())
	{
		this->contextMenu->ClearRealization(contextMenuHandle);
		this->contextMenuHandle = NULL;
	}

	this->contextMenu = menu_new;
	if(! this->contextMenu.isNull())
	{
		this->contextMenuHandle = this->contextMenu->GetMenu();
	}
}

SharedPtr<MenuItem> Win32UserWindow::GetContextMenu()
{
	return this->contextMenu;
}

void Win32UserWindow::SetIcon(SharedString icon_path)
{
	this->icon_path = icon_path;
	this->SetupIcon();
}

void Win32UserWindow::SetupIcon()
{
	SharedString icon_path = this->icon_path;

	if (icon_path.isNull() && !UIModule::GetIcon().isNull())
		icon_path = UIModule::GetIcon();

	if (icon_path.isNull())
	{
		// need to remove the icon
		SendMessageA(window_handle, (UINT)WM_SETICON, ICON_BIG, (LPARAM)initial_icon);
	}
	else
	{
		std::string ext = icon_path->substr(icon_path->length()-4,4);
		if (ext == ".ico")
		{
			HANDLE icon = LoadImageA(win32_host->GetInstanceHandle(),
				icon_path->c_str(), IMAGE_ICON,
				32, 32,
				LR_LOADFROMFILE);
			SendMessageA(window_handle, (UINT)WM_SETICON, ICON_BIG, (LPARAM)icon);
		}
	}
}

SharedString Win32UserWindow::GetIcon()
{
	return icon_path;
}

void Win32UserWindow::SetUsingChrome(bool chrome) {
	STUB();
	//TODO: implement
}

void Win32UserWindow::AppMenuChanged()
{
	if (this->menu.isNull())
	{
		this->SetupMenu();
	}
}

void Win32UserWindow::AppIconChanged()
{
	if (this->icon_path.isNull())
	{
		this->SetupIcon();
	}
}

void Win32UserWindow::RemoveMenu()
{
	// Check if we are already using a menu
	// and the window is initialized.
	if (this->window_handle != NULL && !this->menuInUse.isNull())
	{
		this->menuInUse->ClearRealization(this->menuBarHandle);
		::SetMenu(this->window_handle, NULL);
	}

	this->menuInUse = NULL;
}

void Win32UserWindow::SetupMenu()
{
	SharedPtr<Win32MenuItemImpl> menu = this->menu;
	SharedPtr<MenuItem> appMenu = UIModule::GetMenu();

	// No window menu, try to use the application menu.
	if (menu.isNull() && !appMenu.isNull())
	{
		menu = appMenu.cast<Win32MenuItemImpl>();
	}

	// Only do this if the menu is actually changing.
	if (menu == this->menuInUse)
		return;

	this->RemoveMenu();

	if (!menu.isNull() && this->window_handle)
	{
		this->menuBarHandle = menu->GetMenuBar();
		::SetMenu(this->window_handle, menuBarHandle);
		DrawMenuBar(this->window_handle);
	}

	this->menuInUse = menu;
}

void Win32UserWindow::ReloadTiWindowConfig()
{
	//host->webview()->GetMainFrame()->SetAllowsScrolling(tiWindowConfig->isUsingScrollbars());
	//SetWindowText(hWnd, UTF8ToWide(tiWindowConfig->getTitle()).c_str());

	long windowStyle = GetWindowLong(this->window_handle, GWL_STYLE);

	SetFlag(windowStyle, WS_MINIMIZEBOX, config->IsMinimizable());
	SetFlag(windowStyle, WS_MAXIMIZEBOX, config->IsMaximizable());

	SetFlag(windowStyle, WS_OVERLAPPEDWINDOW, config->IsUsingChrome() && config->IsResizable());
	SetFlag(windowStyle, WS_CAPTION, config->IsUsingChrome());

	SetWindowLong(this->window_handle, GWL_STYLE, windowStyle);

	//UINT flags = SWP_NOZORDER | SWP_FRAMECHANGED;

	//sizeTo(tiWindowConfig->getX(), tiWindowConfig->getY(), tiWindowConfig->getWidth(), tiWindowConfig->getHeight(), flags);

	//SetLayeredWindowAttributes(hWnd, 0, (BYTE)0, LWA_ALPHA);
	/*
	if (tiWindowConfig->getTransparency() < 1.0) {
		SetLayeredWindowAttributes(hWnd, 0, (BYTE)floor(tiWindowConfig->getTransparency()*255), LWA_ALPHA);
	}
	SetLayeredWindowAttributes(hWnd, transparencyColor, 0, LWA_COLORKEY);
	*/
}

	// called by frame load delegate to let the window know it's loaded
void Win32UserWindow::FrameLoaded()
{
	if (this->requires_display)
	{
		this->requires_display = false;
		ShowWindow(window_handle, SW_SHOW);
	}
}

bool Win32UserWindow::IsTopMost()
{
	return topmost;
}

void Win32UserWindow::SetTopMost(bool topmost)
{
	if (topmost)
	{
		SetWindowPos(window_handle,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
		this->topmost = true;
	}
	else
	{
		SetWindowPos(window_handle,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
		this->topmost = false;
	}
}
