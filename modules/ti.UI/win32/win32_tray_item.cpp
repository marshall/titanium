/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "win32_tray_item.h"
#include "win32_user_window.h"
#include "../user_window.h"

#define STUB() printf("Method is still a stub, %s:%i\n", __FILE__, __LINE__)

namespace ti
{
Win32TrayItem::Win32TrayItem(SharedString iconPath, SharedBoundMethod cb)
{
	this->callback = cb;
	this->menuHandle = NULL;

	this->CreateTrayIcon(*iconPath, std::string("Titanium Application"));
}

Win32TrayItem::~Win32TrayItem()
{
	this->Remove();
}

void Win32TrayItem::SetIcon(SharedString iconPath)
{
	if (this->trayIconData == NULL)
	{
		// nothing to do
		return;
	}

	this->trayIconData->hIcon = (HICON) LoadImage(
			::GetModuleHandle(NULL), (*iconPath).c_str(), IMAGE_ICON,
			GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
			LR_DEFAULTCOLOR | LR_LOADFROMFILE);

	Shell_NotifyIcon(NIM_MODIFY, this->trayIconData);
}
void Win32TrayItem::SetMenu(SharedBoundList menu)
{
	STUB();
}
void Win32TrayItem::SetHint(SharedString hint)
{
	if (this->trayIconData == NULL)
	{
		// nothing to do
		return;
	}

	lstrcpy(this->trayIconData->szTip, (*hint).c_str());
	Shell_NotifyIcon(NIM_MODIFY, this->trayIconData);
}
void Win32TrayItem::Remove()
{
	if (this->trayIconData == NULL)
	{
		// nothing to do
		return;
	}

	Shell_NotifyIcon(NIM_DELETE, this->trayIconData);

	this->trayIconData = NULL;
}
void Win32TrayItem::CreateTrayIcon(std::string &iconPath, std::string &caption)
{
	UserWindow* uw = NULL;

	std::vector<UserWindow*>& windows = UserWindow::GetWindows();
	std::vector<UserWindow*>::iterator i = windows.begin();
	if (i != windows.end())
	{
		uw = *i;
	}

	Win32UserWindow* wuw = dynamic_cast<Win32UserWindow*> (uw);

	NOTIFYICONDATA* notifyIconData = new NOTIFYICONDATA;
	notifyIconData->cbSize = sizeof(NOTIFYICONDATA);
	notifyIconData->hWnd = wuw->GetWindowHandle();
	notifyIconData->uID = Win32MenuItemImpl::nextMenuUID();
	notifyIconData->uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	notifyIconData->uCallbackMessage = TI_TRAY_CLICKED;
	notifyIconData->hIcon = (HICON) LoadImage(
			::GetModuleHandle(NULL), iconPath.c_str(), IMAGE_ICON,
			GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
			LR_DEFAULTCOLOR | LR_LOADFROMFILE);

	lstrcpy(notifyIconData->szTip, caption.c_str());

	Shell_NotifyIcon(NIM_ADD, notifyIconData);

	this->trayIconData = notifyIconData;
}
}
