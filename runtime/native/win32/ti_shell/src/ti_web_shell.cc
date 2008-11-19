//
// Copyright 2006-2008 Appcelerator, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "ti_web_shell.h"
#include "ti_utils.h"
#include "Resource.h"

#include <fstream>
#include <shellapi.h>

std::string TIGetDataResource(HMODULE module, int resource_id) {
	void *data_ptr;
	size_t data_size;
	return base::GetDataResourceFromModule(module, resource_id,
		&data_ptr, &data_size) ?
		std::string(static_cast<char*>(data_ptr), data_size) : std::string();

}

std::string TINetResourceProvider(int key) {
	return TIGetDataResource(::GetModuleHandle(NULL), key);
}

TIWebShell::TIWebShell(HINSTANCE hInstance, HWND hWnd) : delegate(this) {
	ti_debug("creating TIWebShell...");

	this->hInstance = hInstance;
	this->hWnd = hWnd;
}

TIWebShell::~TIWebShell(void) {
	ti_debug("destroying TIWebSehll...");

	// TODO  remove tray icon if one exists .. mmm.. 
	//this->removeTrayIcon();
}

void TIWebShell::init(TiApp *tiApp) {
	ti_debug("initializing TIWebShell...");
	this->tiApp = tiApp;

	std::wstring cache_path;
	PathService::Get(base::DIR_EXE, &cache_path);
    file_util::AppendToPath(&cache_path, L"cache");

	// Initializing with a default context, which means no on-disk cookie DB,
	// and no support for directory listings.
	SimpleResourceLoaderBridge::Init(new TestShellRequestContext(cache_path, net::HttpCache::NORMAL));

	// TODO this should only be done once in the program execution
	icu_util::Initialize();

	net::NetModule::SetResourceProvider(TINetResourceProvider);


	WebPreferences web_prefs_;
	ti_initWebPrefs(&web_prefs_);

	this->host = WebViewHost::Create(this->hWnd, &delegate, web_prefs_);

	delegate.setMainWnd(this->hWnd);
	delegate.setHost(this->host);

	if (tiApp) {
		loadTiApp();
	}

	ShowWindow(host->window_handle(), SW_SHOW);

	ti_debug("done initializing TIWebShell");
}


#define SetFlag(x,flag,b) (b ? x |= flag : x &= ~flag)
#define UnsetFlag(x,flag) (x &= ~flag)

void TIWebShell::loadTiApp()
{
	std::string startPath = tiApp->getStartPath();
	std::wstring fileURL = L"file://";
	fileURL += resourcesPath;
	fileURL += L"/";
	fileURL += UTF8ToWide(tiApp->getStartPath());

	size_t index;
	while ((index = fileURL.find(file_util::kPathSeparator)) != std::string::npos) {
		fileURL.replace(index, 1, L"/");
	}
	
	delegate.bootstrapTitanium = true;
	loadURL(WideToUTF8(fileURL).c_str());
	SetWindowText(hWnd, UTF8ToWide(tiApp->getTitle()).c_str());

	
	long windowStyle = GetWindowLong(hWnd, GWL_STYLE);
	
	SetFlag(windowStyle, WS_MINIMIZEBOX, tiApp->isMinimizable());
	SetFlag(windowStyle, WS_MAXIMIZEBOX, tiApp->isMaximizable());
	SetFlag(windowStyle, WS_CAPTION, tiApp->isUsingChrome());
	
	SetWindowLong(hWnd, GWL_STYLE, windowStyle);

	UINT flags = SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED;
	if (!tiApp->isResizable()) {
		flags |= SWP_NOREPOSITION;
	}

	sizeTo(tiApp->getWidth(), tiApp->getHeight(), flags);
	SetLayeredWindowAttributes(hWnd, 0, (BYTE)floor(tiApp->getTransparency()*255), LWA_ALPHA);
}

void TIWebShell::loadURL(const char* url) {
	WebRequest *request = WebRequest::Create(GURL(url));
	WebFrame *frame = host->webview()->GetMainFrame();

	frame->LoadRequest(request);

	host->webview()->SetFocusedFrame(frame);

	SetFocus(host->window_handle());
	ShowWindow(host->window_handle(), SW_SHOW);
}


void TIWebShell::sizeTo(int width, int height, UINT flags) {
	RECT rc, rw;
	GetClientRect(hWnd, &rc);
	GetWindowRect(hWnd, &rw);

	int client_width = rc.right - rc.left;
	int window_width = rw.right - rw.left;
	window_width = (window_width - client_width) + width;

	int client_height = rc.bottom - rc.top;
	int window_height = rw.bottom - rw.top;
	window_height = (window_height - client_height) + height;

	SetWindowPos(hWnd, NULL, 0, 0, window_width, window_height, flags);
}

WebViewHost* TIWebShell::getHost() {
	return this->host;
}

HWND TIWebShell::getHWnd() {
	return this->hWnd;
}
HWND TIWebShell::getPopupHWnd() {
	return popupHost->window_handle();
}

void TIWebShell::include(std::string& relativePath)
{
	std::string absolutePath;

	if (relativePath.find_first_of("://") != std::string::npos) {
		absolutePath = TiURL::absolutePathForURL(tiApp, relativePath);
	}
	else {
		absolutePath = WideToUTF8(getResourcesPath());
		absolutePath += "\\";
		absolutePath += relativePath;
	}
	std::ifstream in(absolutePath.c_str());
	std::string s, line;
	while(getline(in, line)) {
		s += line + "\n";
	}

	WebView *webview = getHost()->webview();
	webview->GetMainFrame()->ExecuteJavaScript(s, absolutePath);
}

WebWidget* TIWebShell::CreatePopupWidget(WebView* webview) {
  popupHost = WebWidgetHost::Create(NULL, &delegate);
  ShowWindow(getPopupHWnd(), SW_SHOW);

  return popupHost->webwidget();
}

void TIWebShell::ClosePopup() {
  PostMessage(getPopupHWnd(), WM_CLOSE, 0, 0);
  popupHost = NULL;
}

void TIWebShell::createTrayIcon() {
	NOTIFYICONDATA tnd;
	tnd.cbSize = sizeof(NOTIFYICONDATA);
	tnd.hWnd = this->hWnd;
	tnd.uID = IDR_MAINFRAME;
	tnd.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	tnd.uCallbackMessage = WM_TITRAYMESSAGE;
	tnd.hIcon = LoadIcon(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_CHROME_SHELL3));

	lstrcpy(tnd.szTip, TEXT("Titanium Shell (this should prbly be dynamic)"));

	Shell_NotifyIcon(NIM_ADD, &tnd);
}

void TIWebShell::removeTrayIcon() {
	NOTIFYICONDATA tnid;
	tnid.cbSize = sizeof(NOTIFYICONDATA);
	tnid.hWnd = this->hWnd;
	tnid.uID = IDR_MAINFRAME;		// ensure we remove our app tray icon
	Shell_NotifyIcon(NIM_DELETE, &tnid);
}

void TIWebShell::showWindow(int nCmdShow) {
	ShowWindow(hWnd, nCmdShow);
}