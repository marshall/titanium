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

#include <tchar.h>
#include <windows.h>
#include <atlbase.h>
#include <commdlg.h>
#include <objbase.h>
#include <shlwapi.h>
#include <wininet.h>
#include <commctrl.h>

#include "ti_shell.h"
#include "base/scoped_ptr.h"
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
#include "sandbox/src/sandbox_factory.h"
#include "sandbox/src/dep.h"
#include "webview_host.h"
#include "webwidget_host.h"
#include "simple_resource_loader_bridge.h"
#include "test_shell_request_context.h"

#include "ti_web_shell.h"
#include "ti_web_view_delegate.h"
#include "ti_url.h"
#include "ti_utils.h"

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
void				ResizeSubViews();

std::string TiGetDataResource(HMODULE module, int resource_id) {
	void *data_ptr;
	size_t data_size;
	return base::GetDataResourceFromModule(module, resource_id,
		&data_ptr, &data_size) ?
		std::string(static_cast<char*>(data_ptr), data_size) : std::string();

}

std::string TiNetResourceProvider(int key) {
	return TiGetDataResource(::GetModuleHandle(NULL), key);
}


int main (int argc, char **argv) {
	// win32 stuff
	HINSTANCE hInstance = ::GetModuleHandle(NULL);

	base::AtExitManager at_exit_manager;
	MessageLoopForUI message_loop;
	base::EnableTerminationOnHeapCorruption();
	
	INITCOMMONCONTROLSEX InitCtrlEx;

	InitCtrlEx.dwSize = sizeof(INITCOMMONCONTROLSEX);
	InitCtrlEx.dwICC  = ICC_STANDARD_CLASSES;
	InitCommonControlsEx(&InitCtrlEx);
	HRESULT res = OleInitialize(NULL);

	// Chrome stuff
	////////////////////////////////////////////////////

	win_util::WinVersion win_version = win_util::GetWinVersion();
	if (win_version == win_util::WINVERSION_XP ||
		win_version == win_util::WINVERSION_SERVER_2003) {
		// On Vista, this is unnecessary since it is controlled through the
		// /NXCOMPAT linker flag.
		// Enforces strong DEP support.
		sandbox::SetCurrentProcessDEP(sandbox::DEP_ENABLED);
	}

	std::wstring resourcesPath;
	PathService::Get(base::DIR_EXE, &resourcesPath);
	file_util::AppendToPath(&resourcesPath, L"Resources");

	std::wstring tiAppXmlPath = resourcesPath;
	file_util::AppendToPath(&tiAppXmlPath, L"tiapp.xml");

	TiApp *tiApp = new TiApp(tiAppXmlPath);
	
	std::wstring cache_path;
	PathService::Get(base::DIR_EXE, &cache_path);
    file_util::AppendToPath(&cache_path, L"cache");

	SimpleResourceLoaderBridge::Init(new TestShellRequestContext(cache_path, net::HttpCache::NORMAL));
	icu_util::Initialize();
	net::NetModule::SetResourceProvider(TiNetResourceProvider);
	TiURL::init();

	HACCEL hAccelTable;
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CHROME_SHELL3));

	TiWebShell::setTiApp(tiApp);
	TiWebShell *tiWebShell = new TiWebShell(tiApp->getMainWindow());
	tiWebShell->open();

	MessageLoop::current()->Run();
	MessageLoop::current()->RunAllPending();

	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
