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
#include <stdio.h>
#include <io.h>
#include <fcntl.h>

TCHAR TiChromeWindow::defaultWindowTitle[128];
TCHAR TiChromeWindow::windowClassName[128];
TiAppConfig* TiChromeWindow::tiAppConfig = NULL;
std::vector<TiChromeWindow*> TiChromeWindow::openWindows = std::vector<TiChromeWindow*>();

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
		wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
		wcex.lpszMenuName	= 0;
		wcex.lpszClassName	= windowClassName;
		wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
		
		RegisterClassEx(&wcex);
		
		initialized = true;
	}

#ifdef TITANIUM_DEBUGGING
	createDebugConsole();
#else
	if (TiAppArguments::isDevMode) {
		createDebugConsole();
	}
#endif
}

/*static*/
void TiChromeWindow::removeWindowClass (HINSTANCE hInstance)
{
	UnregisterClass(windowClassName, hInstance);
}

/*static*/
void TiChromeWindow::createDebugConsole ()
{
	AllocConsole();

    HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
    int hCrt = _open_osfhandle((long)handle_out, _O_TEXT);
    FILE* hf_out = _fdopen(hCrt, "w");
    setvbuf(hf_out, NULL, _IONBF, 1);
    *stdout = *hf_out;

    HANDLE handle_in = GetStdHandle(STD_INPUT_HANDLE);
    hCrt = _open_osfhandle((long)handle_in, _O_TEXT);
    FILE* hf_in = _fdopen(hCrt, "r");
    setvbuf(hf_in, NULL, _IONBF, 128);
    *stdin = *hf_in;
}

void TiChromeWindow::openInspectorWindow()
{
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
					DestroyWindow(hWnd);
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

		// forward some things onto the webview host
		//case WM_MOUSEMOVE:
		//case WM_MOUSELEAVE:
		//case WM_LBUTTONDOWN:
		//case WM_MBUTTONDOWN:
		//case WM_RBUTTONDOWN:
		//case WM_LBUTTONUP:
		//case WM_MBUTTONUP:
		//case WM_RBUTTONUP:
		//case WM_LBUTTONDBLCLK:
		//case WM_MBUTTONDBLCLK:
		//case WM_RBUTTONDBLCLK:
		//case WM_CAPTURECHANGED:
		//case WM_CANCELMODE:
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_IME_CHAR:
		//case WM_SETFOCUS:
		//case WM_KILLFOCUS:
		//case WM_MOUSEWHEEL:
			if (!PostMessage(window->getHost()->window_handle(), message, wParam, lParam)) {
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;
		case WM_DESTROY:
			if (window == TiChromeWindow::getMainWindow()) {
				PostQuitMessage(0);
			}
			break;
		case WM_SIZE:
			window->resizeHost();
			break;
		case TI_TRAY_CLICKED:
			{
				UINT uMouseMsg = (UINT) lParam;
				if(uMouseMsg == WM_LBUTTONDOWN)
				{
					// handle the click callback for the tray
				}
				else if (uMouseMsg == WM_RBUTTONDOWN)
				{
					TiMenu::showSystemMenu();
				}
			}
			break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

TiChromeWindow::TiChromeWindow(HINSTANCE hInstance_, TiWindowConfig *windowConfig) : hWnd(NULL), hInstance(hInstance_), host(NULL)
{
	TiChromeWindow::openWindows.push_back(this);	
	this->tiWindowConfig = windowConfig;

	if (windowConfig != NULL) {
		this->url = tiWindowConfig->getURL();
	}

	tiUserWindow = new TiUserWindow(this);
	createWindow();
}

TiChromeWindow::TiChromeWindow(HINSTANCE hInstance_, const char *url) : hWnd(NULL), hInstance(hInstance_), host(NULL)
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
	popupHost = NULL;
}

#define SetFlag(x,flag,b) ((b) ? x |= flag : x &= ~flag)
#define UnsetFlag(x,flag) (x &= ~flag)=

void TiChromeWindow::setTiWindowConfig(TiWindowConfig *tiWindowConfig)
{
	this->tiWindowConfig = tiWindowConfig;
	reloadTiWindowConfig();
}

void TiChromeWindow::reloadTiWindowConfig()
{

	SetWindowText(hWnd, UTF8ToWide(tiWindowConfig->getTitle()).c_str());

	long windowStyle = GetWindowLong(hWnd, GWL_STYLE);

	SetFlag(windowStyle, WS_MINIMIZEBOX, tiWindowConfig->isMinimizable());
	SetFlag(windowStyle, WS_MAXIMIZEBOX, tiWindowConfig->isMaximizable());

	SetFlag(windowStyle, WS_OVERLAPPEDWINDOW, tiWindowConfig->isUsingChrome() && tiWindowConfig->isResizable());
	SetFlag(windowStyle, WS_CAPTION, tiWindowConfig->isUsingChrome());

	SetWindowLong(hWnd, GWL_STYLE, windowStyle);

	UINT flags = SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED;

	sizeTo(tiWindowConfig->getX(), tiWindowConfig->getY(), tiWindowConfig->getWidth(), tiWindowConfig->getHeight(), flags);
	SetLayeredWindowAttributes(hWnd, 0, (BYTE)floor(tiWindowConfig->getTransparency()*255), LWA_ALPHA);
}


void TiChromeWindow::open() {
	if (hWnd == NULL) {
		createWindow();
	}
	
	if (tiWindowConfig->getURL().length() > 0) {
		maybeLoadURL(tiWindowConfig->getURL().c_str());
	}

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
		absolutePath = WideToUTF8(tiAppConfig->getResourcePath());
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
	CloseWindow(hWnd);
}

