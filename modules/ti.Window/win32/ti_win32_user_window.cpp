/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "ti_win32_user_window.h"
#include "ti_win32_frame_load_delegate.h"
#include "string_util.h"
#include <math.h>

bool TiWin32UserWindow::ole_initialized = false;

static void* SetWindowUserData(HWND hwnd, void* user_data) {
  return
      reinterpret_cast<void*>(SetWindowLongPtr(hwnd, GWLP_USERDATA,
          reinterpret_cast<LONG_PTR>(user_data)));
}

static void* GetWindowUserData(HWND hwnd) {
  return reinterpret_cast<void*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
}

/*static*/
TiWin32UserWindow* TiWin32UserWindow::FromWindow(HWND hWnd)
{
	return reinterpret_cast<TiWin32UserWindow*>(GetWindowUserData(hWnd));
}


const TCHAR *windowClassName = "TiWin32UserWindow";

/*static*/
void TiWin32UserWindow::RegisterWindowClass (HINSTANCE hInstance)
{
	static bool class_initialized = false;
	if (!class_initialized) {
		//LoadString(hInstance, IDC_TIUSERWINDOW, windowClassName, 100);

		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);

		wcex.style			= CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc	= TiWin32UserWindow::WndProc;
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

/*static*/
LRESULT CALLBACK
TiWin32UserWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	TiWin32UserWindow *window = TiWin32UserWindow::FromWindow(hWnd);

	switch (message)
	{
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case WM_SIZE:
			if (!window->web_view) break;
			window->ResizeSubViews();
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

TiWin32UserWindow::TiWin32UserWindow(TiHost *host, TiWindowConfig *config)
	: TiUserWindow(host, config)
{
	static bool initialized = false;
	win32_host = static_cast<Win32Host*>(host);
	if (!initialized) {
		INITCOMMONCONTROLSEX InitCtrlEx;

		InitCtrlEx.dwSize = sizeof(INITCOMMONCONTROLSEX);
		InitCtrlEx.dwICC  = 0x00004000; //ICC_STANDARD_CLASSES;
		InitCommonControlsEx(&InitCtrlEx);
	}

	std::cout << "HINSTANCE = " << (int)win32_host->GetInstanceHandle() << std::endl;

	TiWin32UserWindow::RegisterWindowClass(win32_host->GetInstanceHandle());
	window_handle = CreateWindow(windowClassName, "Titanium Application",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
            NULL, NULL, win32_host->GetInstanceHandle(), NULL);

	if (window_handle == NULL) {
		std::cout << "Error Creating Window: " << GetLastError() << std::endl;
	}

	if (!initialized) {
		HRESULT r = OleInitialize(NULL);
		std::cout << "OleInitialize Result: " << r << std::endl;
		initialized = true;
	}

	SetWindowUserData(window_handle, this);

	ShowWindow(window_handle, SW_SHOW);

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

	std::cout << "create delegate " << std::endl;
	delegate = new TiWin32FrameLoadDelegate(this);

	TiBounds b;
	b.x = config->GetX();
	b.y = config->GetY();
	b.width = config->GetWidth();
	b.height = config->GetHeight();
	SetBounds(b);

	std::cout << "set frame load delegate, set host window, webview=" << (int)web_view  << std::endl;
	hr = web_view->setFrameLoadDelegate(delegate);
	hr = web_view->setHostWindow((OLE_HANDLE)window_handle);

	std::cout << "init with frame" << std::endl;
	RECT client_rect;
	GetClientRect(window_handle, &client_rect);
	hr = web_view->initWithFrame(client_rect, 0, 0);

	IWebViewPrivate *web_view_private;
	hr = web_view->QueryInterface(IID_IWebViewPrivate, (void**)&web_view_private);
	hr = web_view_private->viewWindow((OLE_HANDLE*) &view_window_handle);
	web_view_private->Release();

	hr = web_view->mainFrame(&main_frame);

	std::cout << "resize subviews" << std::endl;
	ResizeSubViews();
}

TiWin32UserWindow::~TiWin32UserWindow()
{
	if (web_view)
		web_view->Release();

	if (main_frame)
		main_frame->Release();
}

void TiWin32UserWindow::ResizeSubViews()
{
    RECT rcClient;
    GetClientRect(window_handle, &rcClient);
    MoveWindow(view_window_handle, 0, 0, rcClient.right, rcClient.bottom, TRUE);
}

void TiWin32UserWindow::Hide() {
	ShowWindow(window_handle, SW_HIDE);
}

void TiWin32UserWindow::Show() {
	ShowWindow(window_handle, SW_SHOW);
}

void TiWin32UserWindow::Open() {
	std::cout << "Opening window_handle=" << (int)window_handle << ", view_window_handle="<<(int)view_window_handle<<std::endl;
	ShowWindow(window_handle, SW_SHOW);
	ShowWindow(view_window_handle, SW_SHOW);
	UpdateWindow(window_handle);
	UpdateWindow(view_window_handle);

	ResizeSubViews();

	TiUserWindow::Open(this);
	SetUrl(this->config->GetURL());

}

void TiWin32UserWindow::Close() {
	CloseWindow(window_handle);

	TiUserWindow::Close(this);
}

double TiWin32UserWindow::GetX() {
	return GetBounds().x;
}

void TiWin32UserWindow::SetX(double x) {
	TiBounds b = GetBounds();
	b.x = x;
	SetBounds(b);
}

double TiWin32UserWindow::GetY() {
	return GetBounds().y;
}

void TiWin32UserWindow::SetY(double y) {
	TiBounds b = GetBounds();
	b.y = y;
	SetBounds(b);
}

double TiWin32UserWindow::GetWidth() {
	return GetBounds().width;
}

void TiWin32UserWindow::SetWidth(double width) {
	TiBounds b = GetBounds();
	b.width = width;
	SetBounds(b);
}

double TiWin32UserWindow::GetHeight() {
	return GetBounds().height;
}

void TiWin32UserWindow::SetHeight(double height) {
	TiBounds b = GetBounds();
	b.height = height;
	SetBounds(b);
}

TiBounds TiWin32UserWindow::GetBounds() {
	TiBounds bounds;

	RECT rect;
	GetWindowRect(window_handle, &rect);

	bounds.x = rect.left;
	bounds.y = rect.top;
	bounds.width = rect.right - rect.left;
	bounds.height = rect.bottom - rect.top;

	return bounds;
}

void TiWin32UserWindow::SetBounds(TiBounds bounds) {
	SetWindowPos(window_handle, NULL, bounds.x, bounds.y, bounds.width, bounds.height, SWP_SHOWWINDOW | SWP_NOZORDER);
}

void TiWin32UserWindow::SetTitle(std::string title) {
	this->title = title;
	SetWindowText(window_handle, title.c_str());
}

void TiWin32UserWindow::SetUrl(std::string url) {
	this->config->SetURL(url);

	std::cout << "SetUrl: " << url << std::endl;

	IWebMutableURLRequest* request = 0;
	std::wstring method = L"GET";

	if (url.length() > 0 && (PathFileExists(url.c_str()) || PathIsUNC(url.c_str()))) {
		TCHAR fileURL[INTERNET_MAX_URL_LENGTH];
		DWORD fileURLLength = sizeof(fileURL)/sizeof(fileURL[0]);
		if (SUCCEEDED(UrlCreateFromPath(url.c_str(), fileURL, &fileURLLength, 0)))
			url = fileURL;
	}
	std::wstring wurl = UTF8ToWide(url);

	HRESULT hr = CoCreateInstance(CLSID_WebMutableURLRequest, 0, CLSCTX_ALL, IID_IWebMutableURLRequest, (void**)&request);
	if (FAILED(hr))
		goto exit;

	hr = request->initWithURL(SysAllocString(wurl.c_str()), WebURLRequestUseProtocolCachePolicy, 60);
	if (FAILED(hr))
		goto exit;

	hr = request->setHTTPMethod(SysAllocString(method.c_str()));
	if (FAILED(hr))
		goto exit;

	hr = main_frame->loadRequest(request);
	if (FAILED(hr))
		goto exit;

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

void TiWin32UserWindow::SetResizable(bool resizable) {
	this->resizable = resizable;
	SetGWLFlag(window_handle, WS_OVERLAPPEDWINDOW, using_chrome && !resizable);
}

void TiWin32UserWindow::SetMaximizable(bool maximizable) {
	this->maximizable = maximizable;
	SetGWLFlag(window_handle, WS_MAXIMIZEBOX, maximizable);
}

void TiWin32UserWindow::SetMinimizable(bool minimizable) {
	this->minimizable = minimizable;
	SetGWLFlag(window_handle, WS_MINIMIZEBOX, minimizable);
}

void TiWin32UserWindow::SetCloseable(bool closeable) {
	this->closeable = closeable;
}

bool TiWin32UserWindow::IsVisible() {
	WINDOWPLACEMENT placement;
	placement.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(window_handle, &placement);

	return placement.showCmd == SW_SHOW;
}

void TiWin32UserWindow::SetVisible(bool visible) {
	this->showing = visible;
	ShowWindow(window_handle, visible ? SW_SHOW : SW_HIDE);
}

void TiWin32UserWindow::SetTransparency(double transparency) {
	this->transparency = transparency;
	SetLayeredWindowAttributes(window_handle, 0, (BYTE)floor(transparency*255), LWA_ALPHA);
}

