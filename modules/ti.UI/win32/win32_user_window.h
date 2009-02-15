/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef __TI_WIN32_TYPES_H
#define __TI_WIN32_TYPES_H

#include <kroll/base.h>
#include <windows.h>
#include <stdlib.h>
#include <malloc.h>
#include <tchar.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <wininet.h>
#include "WebKit.h"

#include "../ui_module.h"
#include <kroll/kroll.h>
#include "../../../kroll/host/win32/host.h"
#include "../user_window.h"
#include "script_evaluator.h"
#include "win32_menu_item_impl.h"

#include <map>

namespace ti {

class Win32WebKitFrameLoadDelegate;
class Win32WebKitUIDelegate;

class Win32UserWindow : public UserWindow {

protected:
	static bool ole_initialized;

	kroll::Win32Host *win32_host;
	Win32WebKitFrameLoadDelegate *frameLoadDelegate;
	Win32WebKitUIDelegate *uiDelegate;
	Bounds restore_bounds;
	long restore_styles;
	ScriptEvaluator script_evaluator;

	HWND window_handle, view_window_handle;
	IWebView* web_view;
	IWebFrame *main_frame;
	std::string title, id;
	bool showing, full_screen, using_scrollbars,
		resizable, using_chrome, minimizable, maximizable, closeable;
	double transparency;
	std::map<long, SharedBoundMethod> messageHandlers;

	/*
	 * The window-specific menu.
	 */
	SharedPtr<Win32MenuItemImpl> menu;

	/*
	 * The menu this window is using. This
	 * might just be a copy of the app menu.
	 */
	SharedPtr<Win32MenuItemImpl> menuInUse;

	HMENU menuBarHandle;

	SharedPtr<Win32MenuItemImpl> contextMenu;
	HMENU contextMenuHandle;

	void RemoveMenu();
	void ReloadTiWindowConfig();

public:
	static void RegisterWindowClass(HINSTANCE hInstance);
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static Win32UserWindow* FromWindow(HWND hWnd);

	Win32UserWindow(kroll::Host *host, WindowConfig *config);
	virtual ~Win32UserWindow();
	UserWindow* WindowFactory(Host*, WindowConfig*);

	void AddMessageHandler(const ValueList& args, SharedValue result);

	void ResizeSubViews();

	void AppMenuChanged();

	HWND GetWindowHandle();
	void Hide();
	void Show();
	bool IsUsingChrome() { return config->IsUsingChrome(); }
	bool IsUsingScrollbars() { return config->IsUsingScrollbars(); }
	bool IsFullScreen() { return config->IsFullScreen(); }
	std::string GetId() { return config->GetID(); }
	void Open();
	void Close();
	double GetX();
	void SetX(double x);
	double GetY();
	void SetY(double y);
	double GetWidth();
	void SetWidth(double width);
	double GetMaxWidth();
	void SetMaxWidth(double width);
	double GetMinWidth();
	void SetMinWidth(double width);
	double GetHeight();
	void SetHeight(double height);
	double GetMaxHeight();
	void SetMaxHeight(double height);
	double GetMinHeight();
	void SetMinHeight(double height);
	Bounds GetBounds();
	void SetBounds(Bounds bounds);
	std::string GetTitle() { return config->GetTitle(); }
	void SetTitle(std::string& title);
	std::string GetURL() { return config->GetURL(); }
	void SetURL(std::string& url);
	bool IsResizable() { return config->IsResizable(); }
	void SetResizable(bool resizable);
	bool IsMaximizable() { return config->IsMaximizable(); }
	void SetMaximizable(bool maximizable);
	bool IsMinimizable() { return config->IsMinimizable(); }
	void SetMinimizable(bool minimizable);
	bool IsCloseable() { return config->IsCloseable(); }
	void SetCloseable(bool closeable);
	bool IsVisible();
	void SetVisible(bool visible);
	double GetTransparency() { return config->GetTransparency(); }
	void SetTransparency(double transparency);
	void SetFullScreen(bool fullscreen);
	void SetUsingChrome(bool chrome);
	void SetMenu(SharedBoundList menu);
	SharedBoundList GetMenu();
	void SetContextMenu(SharedPtr<MenuItem> menu);
	SharedPtr<MenuItem> GetContextMenu();
	HMENU GetContextMenuHandle() { return this->contextMenuHandle; }
	void SetIcon(SharedString icon_path);
	SharedString GetIcon();

	void SetupMenu();

};

}

 void STDMETHODCALLTYPE addScriptEvaluator(IWebScriptEvaluator *evaluator);

#endif
