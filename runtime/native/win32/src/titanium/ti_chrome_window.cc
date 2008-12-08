/*
* Copyright 2006-2008 Appcelerator, Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "ti_chrome_window.h"
#include "ti_user_window.h"
#include "ti_utils.h"
#include "ti_menu.h"
#include "ti_app_arguments.h"
#include "Resource.h"

#include <fstream>
#include <shellapi.h>

TCHAR TiChromeWindow::defaultWindowTitle[128];
TCHAR TiChromeWindow::windowClassName[128];
std::vector<TiChromeWindow*> TiChromeWindow::openWindows = std::vector<TiChromeWindow*>();
bool TiChromeWindow::isInspectorOpen = false;

// slightly off white, there's probably a better way to do this
COLORREF transparencyColor = RGB(0xF9, 0xF9, 0xF9);


/*static*/
TiChromeWindow* TiChromeWindow::fromWindow(HWND hWnd) {
  return reinterpret_cast<TiChromeWindow*>(win_util::GetWindowUserData(hWnd));
}

/*static*/
TiChromeWindow* TiChromeWindow::getMainWindow() {
	TiChromeWindow *main = getWindow("main");
	if (main == NULL) {
		if (openWindows.size() > 0) {
			main = openWindows[0];
		}
	}

	return main;
}

/*static*/
TiChromeWindow* TiChromeWindow::getWindow(const char *id)
{
	for (size_t i = 0; i < openWindows.size(); i++) {
		if (openWindows[i]->getTiWindowConfig()->getId() == id) {
			return openWindows[i];
		}
	}
	return NULL;
}

/*static*/
void TiChromeWindow::initWindowClass (HINSTANCE hInstance)
{
	static bool initialized = false;
	if (! initialized) {
		LoadString(hInstance, IDS_APP_TITLE, defaultWindowTitle, 128);
		LoadString(hInstance, IDC_CHROME_SHELL3, windowClassName, 128);
		WNDCLASSEX wcex;

		wcex.cbSize = sizeof(WNDCLASSEX);

		wcex.style			= CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc	= TiChromeWindow::WndProc;
		wcex.cbClsExtra		= 0;
		wcex.cbWndExtra		= 0;
		wcex.hInstance		= hInstance;
		wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CHROME_SHELL3));
		wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
		//wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
		wcex.hbrBackground	= CreateSolidBrush(transparencyColor);
		wcex.lpszMenuName	= 0;
		wcex.lpszClassName	= windowClassName;
		wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
		
		RegisterClassEx(&wcex);
		
		initialized = true;
	}
}

/*static*/
void TiChromeWindow::removeWindowClass (HINSTANCE hInstance)
{
	UnregisterClass(windowClassName, hInstance);
}

/*static*/
void TiChromeWindow::DestroyWindow (TiChromeWindow *window)
{
	std::vector<TiChromeWindow*>::iterator iter;
	for (iter = openWindows.begin(); iter != openWindows.end(); iter++) {
		if ((*iter) == window) {
			break;
		}
	}
	bool isMain = (window == TiChromeWindow::getMainWindow());

	if (iter != openWindows.end()) {
		openWindows.erase(iter);
	}

	if (isMain) {

		if (TiMenu::trayMenu != NULL) {
			TiMenu::removeTrayMenu();
		}
		PostQuitMessage(0);
	}
}

void TiChromeWindow::openInspectorWindow()
{
	isInspectorOpen = true;
	host->webview()->ShowJavaScriptConsole();
}

/*static*/
LRESULT CALLBACK TiChromeWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	int wmId, wmEvent;

	TiChromeWindow *window = TiChromeWindow::fromWindow(hWnd);

	switch (message)
	{
		case WM_COMMAND:
			wmId    = LOWORD(wParam);
			wmEvent = HIWORD(wParam);
			// Parse the menu selections:
			switch (wmId)
			{
				case IDM_ABOUT:
					//DialogBox(tiWebShell->getInstance(), MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
					break;
				case IDM_EXIT:
					::DestroyWindow(hWnd);
					break;
				default:
				{
					LRESULT handled = TiMenu::handleMenuClick(hWnd, message, wParam, lParam);

					if(! handled)
					{
						return DefWindowProc(hWnd, message, wParam, lParam);
					}
				}
			}
			break;

		case WM_PAINT: {
			//if (window->getTiWindowConfig()->getTransparency() == 1.0) {
			/*	PAINTSTRUCT ps; 
				HBRUSH brush;
				RECT rect;
				GetWindowRect(hWnd, &rect);
				HDC hdc = BeginPaint(hWnd, &ps);
				brush = CreateSolidBrush(transparencyColor);
				SelectObject(hdc, brush);
				Rectangle(hdc, 0, 0, rect.right - rect.top, rect.bottom - rect.top);
				DeleteObject(brush);
				EndPaint(hWnd, &ps);*/
			//}
			window->getHost()->Paint();
		} break;

		// forward some things onto the webview host
		case WM_MOUSEMOVE:
		case WM_MOUSELEAVE:
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
		case WM_LBUTTONDBLCLK:
		case WM_MBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
		case WM_CAPTURECHANGED:
		case WM_CANCELMODE:
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_IME_CHAR:
		case WM_SETFOCUS:
		case WM_KILLFOCUS:
		//case WM_MOUSEWHEEL:
			if (!PostMessage(window->getHost()->window_handle(), message, wParam, lParam)) {
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;
		case WM_DESTROY:
			DestroyWindow(window);
			break;
		case WM_SIZE:
			if (window != NULL)
				window->resizeHost();
			return 0;

		case TI_TRAY_CLICKED:
			{
				UINT uMouseMsg = (UINT) lParam;
				if(uMouseMsg == WM_LBUTTONDOWN)
				{
					TiMenu::invokeLeftClickCallback();
				}
				else if (uMouseMsg == WM_RBUTTONDOWN)
				{
					TiMenu::showTrayMenu();
				}
			}
			break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

TiChromeWindow::TiChromeWindow(HINSTANCE hInstance_, TiWindowConfig *windowConfig) : hWnd(NULL), hInstance(hInstance_), host(NULL), openOnLoad(true)
{
	TiChromeWindow::openWindows.push_back(this);	
	this->tiWindowConfig = windowConfig;

	if (windowConfig != NULL) {
		this->url = tiWindowConfig->getURL();
	}

	tiUserWindow = new TiUserWindow(this);
	createWindow();
}

TiChromeWindow::TiChromeWindow(HINSTANCE hInstance_, const char *url) : hWnd(NULL), hInstance(hInstance_), host(NULL), openOnLoad(true)
{
	TiChromeWindow::openWindows.push_back(this);	
	this->url = url;
	this->tiWindowConfig = new TiWindowConfig();
	
	tiUserWindow = new TiUserWindow(this);
	createWindow();
}

TiChromeWindow::~TiChromeWindow(void) {
	// TODO  remove tray icon if one exists .. mmm.. 
	//this->removeTrayIcon();
}

void TiChromeWindow::createWindow()
{
	hWnd = CreateWindowEx(WS_EX_LAYERED, windowClassName, defaultWindowTitle,
                           WS_CLIPCHILDREN,
                           0, 0, 0, 0,
                           NULL, NULL, hInstance, NULL);

	win_util::SetWindowUserData(hWnd, this);
	
	static WebPreferences webPrefs = ti_initWebPrefs();
	webViewDelegate = new TiWebViewDelegate(this);
	host = WebViewHost::Create(hWnd, webViewDelegate, webPrefs);
	webViewDelegate->setWebViewHost(host);
	
	reloadTiWindowConfig();
		
	if (tiWindowConfig->getURL().length() > 0) {
		maybeLoadURL(tiWindowConfig->getURL().c_str());
	}

}

WebWidget* TiChromeWindow::createPopupWidget()
{
	popupHost = WebWidgetHost::Create(NULL, webViewDelegate);
	ShowWindow(getPopupWindowHandle(), SW_SHOW);

	return popupHost->webwidget();
}

void TiChromeWindow::closePopup()
{
	PostMessage(getPopupWindowHandle(), WM_CLOSE, 0, 0);
	getHost()->Paint();

	popupHost = NULL;
}

#define SetFlag(x,flag,b) ((b) ? x |= flag : x &= ~flag)
#define UnsetFlag(x,flag) (x &= ~flag)=

void TiChromeWindow::setTiWindowConfig(TiWindowConfig *tiWindowConfig)
{
	this->tiWindowConfig = tiWindowConfig;
	this->tiUserWindow->setTiWindowConfig(tiWindowConfig);

	reloadTiWindowConfig();
}

void TiChromeWindow::reloadTiWindowConfig()
{
	host->webview()->GetMainFrame()->SetAllowsScrolling(tiWindowConfig->isUsingScrollbars());
	SetWindowText(hWnd, UTF8ToWide(tiWindowConfig->getTitle()).c_str());

	long windowStyle = GetWindowLong(hWnd, GWL_STYLE);

	SetFlag(windowStyle, WS_MINIMIZEBOX, tiWindowConfig->isMinimizable());
	SetFlag(windowStyle, WS_MAXIMIZEBOX, tiWindowConfig->isMaximizable());

	SetFlag(windowStyle, WS_OVERLAPPEDWINDOW, !tiWindowConfig->isUsingChrome() && tiWindowConfig->isResizable());
	SetFlag(windowStyle, WS_CAPTION, !tiWindowConfig->isUsingChrome());

	SetWindowLong(hWnd, GWL_STYLE, windowStyle);

	UINT flags = SWP_NOZORDER | SWP_FRAMECHANGED;

	sizeTo(tiWindowConfig->getX(), tiWindowConfig->getY(), tiWindowConfig->getWidth(), tiWindowConfig->getHeight(), flags);
	
	//SetLayeredWindowAttributes(hWnd, 0, (BYTE)0, LWA_ALPHA);
	if (tiWindowConfig->getTransparency() < 1.0) {
		SetLayeredWindowAttributes(hWnd, 0, (BYTE)floor(tiWindowConfig->getTransparency()*255), LWA_ALPHA);
	}
	SetLayeredWindowAttributes(hWnd, transparencyColor, 0, LWA_COLORKEY);
}


void TiChromeWindow::open() {
	ShowWindow(hWnd, SW_SHOW);
	ShowWindow(host->window_handle(), SW_SHOW);
}

void TiChromeWindow::maybeLoadURL(const char* url) {
	if (currentURL != url) {
		currentURL = url;

		WebRequest *request = WebRequest::Create(GURL(url));
		WebFrame *frame = host->webview()->GetMainFrame();
		frame->LoadRequest(request);

		host->webview()->SetFocusedFrame(frame);

		SetFocus(host->window_handle());
		ShowWindow(host->window_handle(), SW_SHOW);
	}
}


void TiChromeWindow::sizeTo(int x, int y, int width, int height, UINT flags) {
	RECT rc, rw;
	GetClientRect(hWnd, &rc);
	GetWindowRect(hWnd, &rw);

	int client_width = rc.right - rc.left;
	int window_width = rw.right - rw.left;
	window_width = (window_width - client_width) + width;

	int client_height = rc.bottom - rc.top;
	int window_height = rw.bottom - rw.top;
	window_height = (window_height - client_height) + height;

	HWND desktop = GetDesktopWindow();
	RECT desktopRect;
	GetWindowRect(desktop, &desktopRect);

	if (x < 0) {
		x = (desktopRect.right - window_width) / 2;
	}
	if (y < 0) {
		y = (desktopRect.bottom - window_height) / 2;
	}

	SetWindowPos(hWnd, NULL, x, y, window_width, window_height, flags);
}

void TiChromeWindow::resizeHost() {
	RECT rc;
	GetClientRect(hWnd, &rc);

	MoveWindow(host->window_handle(), 0, 0, rc.right, rc.bottom, TRUE);
}

void TiChromeWindow::include(WebFrame *frame, std::string& relativePath)
{
	std::string absolutePath;

	if (relativePath.find_first_of("://") != std::string::npos) {
		absolutePath = WideToUTF8(TiURL::getPathForURL(GURL(relativePath)));
	}
	else {
		absolutePath = WideToUTF8(TiAppConfig::instance()->getResourcePath());
		absolutePath += "\\";
		absolutePath += relativePath;
	}
	std::ifstream in(absolutePath.c_str());
	std::string s, line;
	while(getline(in, line)) {
		s += line + "\n";
	}

	frame->ExecuteJavaScript(s, absolutePath);
}

void TiChromeWindow::showWindow(int nCmdShow) {
	ShowWindow(hWnd, nCmdShow);
}

std::string TiChromeWindow::getTitle() {
	wchar_t buffer[2049];
	GetWindowText(this->hWnd, buffer, 2048);

	std::string result;
	result.assign(WideToUTF8(buffer));

	return result;
}

void TiChromeWindow::setTitle(std::string title) {
	SetWindowText(this->hWnd, UTF8ToWide(title).c_str());
}

void TiChromeWindow::close() {
	::DestroyWindow(hWnd);
}

