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
//
// chrome_shell.cpp : Defines the entry point for the application.
//

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
#include "ti_utils.h"

#define MAX_LOADSTRING 100

TIWebShell* tiWebShell = NULL;

// Global Variables:
HINSTANCE hInstance;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
void				ResizeSubViews();

int main (int argc, char **argv) {
	// win32 stuff
	hInstance = ::GetModuleHandle(NULL);

	base::AtExitManager at_exit_manager;
	MessageLoopForUI message_loop;
	base::EnableTerminationOnHeapCorruption();
	
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_CHROME_SHELL3, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	INITCOMMONCONTROLSEX InitCtrlEx;

	InitCtrlEx.dwSize = sizeof(INITCOMMONCONTROLSEX);
	InitCtrlEx.dwICC  = ICC_STANDARD_CLASSES;
	InitCommonControlsEx(&InitCtrlEx);
	HRESULT res = OleInitialize(NULL);

	HWND hWnd = CreateWindowEx(WS_EX_LAYERED, szWindowClass, szTitle,
                           WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                           0, 0, 0, 0,
                           NULL, NULL, hInstance, NULL);

	//HWND hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
	//	CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, 1024, 768, NULL, NULL, hInstance, NULL);

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

	tiWebShell = new TIWebShell(hInstance, hWnd);
	tiWebShell->setResourcesPath(resourcesPath);

	TiApp *ti_app = new TiApp(tiAppXmlPath);
	tiWebShell->init(ti_app);
	
	////////////////////////////////////////////////////

	// show main window
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	HACCEL hAccelTable;
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CHROME_SHELL3));

	MessageLoop::current()->Run();
	MessageLoop::current()->RunAllPending();

	return 0;
}

void ResizeSubViews() {
	if (tiWebShell == NULL) return;

	RECT rc;
	GetClientRect(tiWebShell->getHWnd(), &rc);

	MoveWindow(tiWebShell->getHost()->window_handle(), 0, 0, rc.right, rc.bottom, TRUE);
}


//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance) {
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CHROME_SHELL3));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	//wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_CHROME_SHELL3);
	wcex.lpszMenuName	= 0;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	int wmId, wmEvent;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default: {
				LRESULT result = DefWindowProc(hWnd, message, wParam, lParam);
				return result;
			}
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_SIZE:
		ResizeSubViews();
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
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
