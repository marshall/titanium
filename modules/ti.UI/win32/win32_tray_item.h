/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef TI_WIN32_TRAY_ITEM_H_
#define TI_WIN32_TRAY_ITEM_H_

#include <kroll/base.h>
#include <kroll/kroll.h>
#include <windows.h>
#include <shellapi.h>
#include "../tray_item.h"

namespace ti
{

class Win32TrayItem: public TrayItem
{
public:
	Win32TrayItem(SharedString iconPath, SharedBoundMethod cb);
	virtual ~Win32TrayItem();

	void SetIcon(SharedString iconPath);
	void SetMenu(SharedBoundList menu);
	void SetHint(SharedString hint);
	void Remove();

	static bool InvokeLeftClickCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static bool InvokeLeftClickCallback(int trayIconID);
private:
	HMENU menuHandle;
	SharedBoundMethod callback;

	NOTIFYICONDATA* trayIconData;
	void CreateTrayIcon(std::string &iconPath, std::string &caption);
};

}

#endif /* TI_WIN32_TRAY_ITEM_H_ */
