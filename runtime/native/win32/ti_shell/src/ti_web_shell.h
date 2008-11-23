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

#pragma once

#include "ti_web_view_delegate.h"

#include <tchar.h>
#include <windows.h>
#include <atlbase.h>
#include <commdlg.h>
#include <objbase.h>
#include <shlwapi.h>
#include <wininet.h>
#include <commctrl.h>

#include "webkit/glue/webpreferences.h"
#include "webkit/glue/weburlrequest.h"
#include "webkit/glue/webframe.h"
#include "webkit/glue/webview.h"
#include "webkit/glue/webview_delegate.h"
#include "webkit/glue/webwidget_delegate.h"
#include "webkit/glue/plugins/webplugin_delegate_impl.h"
#include "webkit/glue/webkit_glue.h"
#include "base/gfx/point.h"
#include "base/file_util.h"
#include "base/string_util.h"
#include "base/basictypes.h"
#include "base/resource_util.h"
#include "base/ref_counted.h"
#include "base/path_service.h"
#include "base/at_exit.h"
#include "base/process_util.h"
#include "base/message_loop.h"
#include "base/icu_util.h"
#include "base/win_util.h"
#include "net/base/net_module.h"
#include "webview_host.h"
#include "webwidget_host.h"
#include "simple_resource_loader_bridge.h"
#include "test_shell_request_context.h"

#include "ti_window.h"
#include "ti_app_config.h"
#include "ti_url.h"

#include <vector>

class TiWebViewDelegate;

class TiWebShell
{
private:
	HINSTANCE hInstance;
	HWND hWnd;
	WebViewHost* host;
	std::wstring resourcePath;
	TiWindow *tiWindow;
	std::string url;
	TiWebViewDelegate* webViewDelegate;
	bool created;

	static TiAppConfig *tiAppConfig;
	static std::vector<TiWebShell*> openShells;
	static TCHAR defaultWindowTitle[128];
	static TCHAR windowClassName[128];

	static void initWindowClass();
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	void initWindow();

public:
	TiWebShell(TiWindow *tiWindow);
	TiWebShell(const char *url);
	~TiWebShell(void);

	void setTiWindow(TiWindow *tiWindow);
	void createWindow();
	void open();

	void loadURL(const char* url);
	void sizeTo(int width, int height, UINT flags);
	void resizeHost();

	WebViewHost* getHost() { return host; }
	HWND getWindow() { return hWnd; }
	TiWindow* getTiWindow() { return tiWindow; }
	HINSTANCE getInstance() { return hInstance; }

	void include (std::string& relativePath);

	void createTrayIcon();
	void removeTrayIcon();

	void showWindow(int nCmdShow);

	std::string getTitle();
	void setTitle(std::string title);

	void close();

	static std::vector<TiWebShell *>& getOpenShells() { return openShells; }
	static void setTiAppConfig(TiAppConfig* tiAppConfig_) { tiAppConfig = tiAppConfig_; }
	static TiAppConfig* getTiAppConfig() { return tiAppConfig; }
	static TiWebShell* fromWindow(HWND hWnd);
	static TiWebShell* getMainTiWebShell();
};
