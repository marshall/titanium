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

#include "ti_menu.h"
#include "ti_chrome_window.h"

#include <string>
#include <cstdlib>

struct MenuItemCallback
{
public:
	int uID;
	std::string label;
	NPObject* callback;
};

std::vector<MenuItemCallback *> callbacks;

int TiMenu::currentUID = TI_MENU_ITEM_ID_BEGIN + 1;
TiMenu* TiMenu::trayMenu = NULL;
NPObject *TiMenu::leftClickCallback = NULL;

TiMenu::TiMenu(NOTIFYICONDATA notifyIconData_) 
	: notifyIconData(notifyIconData_)
{
	hMenu = CreatePopupMenu();
	bind();
}

TiMenu::TiMenu(HMENU parentMenu, std::string& label_)
	: label(label_)
{
	hMenu = CreatePopupMenu();
	AppendMenu(parentMenu, MF_STRING | MF_POPUP, (UINT_PTR) hMenu, (LPCTSTR) UTF8ToWide(label).c_str());

	// redraw the menu bar
	HWND hWnd = TiChromeWindow::getMainWindow()->getWindowHandle();
	DrawMenuBar(hWnd);
	
	bind();
}

void TiMenu::bind() {
	BindMethod("addItem", &TiMenu::addItem);
	BindMethod("addSubMenu", &TiMenu::addSubMenu);
	BindMethod("addSeparator", &TiMenu::addSeparator);
	BindMethod("remove", &TiMenu::remove);
	//BindMethod("hide", &TiMenu::hide);
	//BindMethod("show", &TiMenu::show);
}

void TiMenu::remove(const CppArgumentList &args, CppVariant *result)
{
	if (this == TiMenu::trayMenu) {
		Shell_NotifyIcon(NIM_DELETE, &notifyIconData);
	}
}

void TiMenu::addItem(const CppArgumentList &args, CppVariant *result)
{
	if (args.size() >= 2) {
		if (args[0].isString() && args[1].isObject()) {
			std::string label = args[0].ToString();
			NPVariant variant;
			args[1].CopyToNPVariant(&variant);

			NPObject* callback = NPVARIANT_TO_OBJECT(variant);
			uID = nextMenuUID();

			MenuItemCallback* itemCallBack = new MenuItemCallback();
			itemCallBack->uID = uID;
			itemCallBack->callback = callback;
			itemCallBack->label = label;

			callbacks.push_back(itemCallBack);

			AppendMenu(this->hMenu, MF_STRING, uID, (LPCTSTR) UTF8ToWide(label).c_str());
		}
	}
}

void TiMenu::addSubMenu(const CppArgumentList &args, CppVariant *result)
{
	if (args.size() >= 1) {
		if (args[0].isString()) {
			std::string menuLabel = args[0].ToString();

			TiMenu *subMenu = new TiMenu(this->hMenu, menuLabel);

			result->Set(subMenu->ToNPObject());
		}
	}
}

void TiMenu::addSeparator(const CppArgumentList &args, CppVariant *result)
{
	AppendMenu(this->hMenu, MF_SEPARATOR, 1, L"Separator");
}

/*static*/
bool TiMenu::invokeCallback(int menuItemUID)
{
	for(size_t i = 0; i < callbacks.size(); i++)
	{
		MenuItemCallback* itemCallback = callbacks[i];

		if(itemCallback->uID == menuItemUID) {
			NPVariant args[] = { StringToNPVariant(itemCallback->label) };

			NPVariant result;
			if (NPN_InvokeDefault(0, itemCallback->callback, static_cast<const NPVariant*>(args), 1, &result)) {
				return true;
			}

			return false;
		}
	}
	return false;
}

/*static*/
LRESULT CALLBACK TiMenu::handleMenuClick(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	TiChromeWindow *window = TiChromeWindow::fromWindow(hWnd);

	if(message == WM_COMMAND)
	{
		int wmId    = LOWORD(wParam);
		//wmEvent = HIWORD(wParam);

		return invokeCallback(wmId);
	}

	return FALSE;
}

/*static*/
void TiMenu::showTrayMenu ()
{
	if (trayMenu != NULL) {
		// handle the tray menu
		POINT pt;
		GetCursorPos(&pt);
		TrackPopupMenu(trayMenu->getMenu(), 
			TPM_BOTTOMALIGN,
			pt.x, pt.y, 0,
			TiChromeWindow::getMainWindow()->getWindowHandle(), NULL);
	}
}

/*static*/
void TiMenu::removeTrayMenu()
{
	if (trayMenu != NULL) {
		Shell_NotifyIcon(NIM_DELETE, &notifyIconData);
	}
}

/*static*/
void TiMenu::invokeLeftClickCallback()
{
	if (leftClickCallback != NULL) {
		const NPVariant args[] = { ObjectToNPVariant(TiMenu::trayMenu->ToNPObject()) };

		NPVariant result;
		NPN_InvokeDefault(0, leftClickCallback, args, 1, &result);
	}
}