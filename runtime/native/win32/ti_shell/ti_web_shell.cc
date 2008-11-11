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

TIWebShell::TIWebShell(HINSTANCE hInstance, HWND hWnd) {
	ti_debug("creating TIWebShell...");

	this->hInstance = hInstance;
	this->hWnd = hWnd;
}

TIWebShell::~TIWebShell(void) {
	ti_debug("destroying TIWebSehll...");
}

void TIWebShell::init() {
	ti_debug("initializing TIWebShell...");


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

	ti_debug("done initializing TIWebShell");
}


void TIWebShell::loadURL(char* url) {
	ti_debug("loadURL() called");

	WebRequest *request = WebRequest::Create(GURL(url));
	WebFrame *frame = host->webview()->GetMainFrame();

	frame->LoadRequest(request);

	host->webview()->SetFocusedFrame(frame);

	//SetFocus(host->window_handle());
	ShowWindow(host->window_handle(), SW_SHOW);
}


WebViewHost* TIWebShell::getHost() {
	return this->host;
}

HWND TIWebShell::getHWnd() {
	return this->hWnd;
}