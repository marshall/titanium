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

#include <fstream>

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
}

void TIWebShell::init(TiApp *ti_app) {
	ti_debug("initializing TIWebShell...");
	this->ti_app = ti_app;

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

	delegate.setHWND(this->hWnd);
	this->host = WebViewHost::Create(this->hWnd, &delegate, web_prefs_);
	delegate.setHost(this->host);

	if (ti_app) {
		sizeTo(ti_app->getWidth(), ti_app->getHeight());
		std::string startPath = ti_app->getStartPath();
		std::wstring fileURL = L"file://";
		fileURL += resourcesPath;
		fileURL += L"/";
		fileURL += UTF8ToWide(ti_app->getStartPath());

		size_t index;
		while ((index = fileURL.find(file_util::kPathSeparator)) != std::string::npos) {
			fileURL.replace(index, 1, L"/");
		}
		
		delegate.bootstrapTitanium = true;
		loadURL(WideToUTF8(fileURL).c_str());
		SetWindowText(hWnd, UTF8ToWide(ti_app->getTitle()).c_str());
	}

	ti_debug("done initializing TIWebShell");
}


void TIWebShell::loadURL(const char* url) {
	WebRequest *request = WebRequest::Create(GURL(url));
	WebFrame *frame = host->webview()->GetMainFrame();

	frame->LoadRequest(request);

	host->webview()->SetFocusedFrame(frame);

	//SetFocus(host->window_handle());
	ShowWindow(host->window_handle(), SW_SHOW);
}


void TIWebShell::sizeTo(int width, int height) {
	RECT rc, rw;
	GetClientRect(hWnd, &rc);
	GetWindowRect(hWnd, &rw);

	int client_width = rc.right - rc.left;
	int window_width = rw.right - rw.left;
	window_width = (window_width - client_width) + width;

	int client_height = rc.bottom - rc.top;
	int window_height = rw.bottom - rw.top;
	window_height = (window_height - client_height) + height;

	SetWindowPos(hWnd, NULL, 0, 0, window_width, window_height,
		SWP_NOMOVE | SWP_NOZORDER);
}

WebViewHost* TIWebShell::getHost() {
	return this->host;
}

HWND TIWebShell::getHWnd() {
	return this->hWnd;
}

void TIWebShell::include(std::string& relativePath)
{
	std::string absolutePath;

	if (relativePath.find_first_of("://") != std::string::npos) {
		absolutePath = TiURL::absolutePathForURL(ti_app, relativePath);
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