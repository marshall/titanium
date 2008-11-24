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
#include "ti_web_shell.h"

#include <string>
#include <cstdlib>

struct MenuItemCallback
{
public:
	int uID;
	std::string label;
	NPObject* callback;
};

static std::vector<MenuItemCallback *> callbacks;

TiMenu::TiMenu(HMENU parentMenu, std::string& label_)
	: label(label_)
{
	hMenu = CreatePopupMenu();
	AppendMenu(parentMenu, MF_STRING | MF_POPUP, (UINT_PTR) hMenu, (LPCTSTR) UTF8ToWide(label).c_str());

	// redraw the menu bar
	HWND hWnd = TiWebShell::getMainTiWebShell()->getWindow();
	DrawMenuBar(hWnd);

	BindMethod("addItem", &TiMenu::addItem);
	BindMethod("addSubMenu", &TiMenu::addSubMenu);
	BindMethod("addSeparator", &TiMenu::addSeparator);
	//BindMethod("hide", &TiMenu::hide);
	//BindMethod("show", &TiMenu::show);
}

void TiMenu::addItem(const CppArgumentList &args, CppVariant *result)
{
	if (args.size() >= 2) {
		if (args[0].isString() && args[1].isObject()) {
			std::string label = args[0].ToString();
			NPVariant variant;
			args[1].CopyToNPVariant(&variant);

			NPObject* callback = NPVARIANT_TO_OBJECT(variant);

			// TODO better way of getting the uID?  to guarantee no collisions
			uID = TI_MENU_ITEM_ID_BEGIN + ((int)rand()&0xFFFF);

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

LRESULT CALLBACK TiMenu::handleMenuClick(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	TiWebShell *tiWebShell = TiWebShell::fromWindow(hWnd);

	if(message == WM_COMMAND)
	{
		int wmId    = LOWORD(wParam);
		//wmEvent = HIWORD(wParam);

		for(size_t i = 0; i < callbacks.size(); i++)
		{
			MenuItemCallback* itemCallback = callbacks[i];

			if(itemCallback->uID == wmId) {
				printf("handle menu item %s (%d)\n", itemCallback->label.c_str(), itemCallback->uID);

				// TODO - callback the JS function
				// NPN_Invoke(object, method, args)
				return TRUE;
			}
		}
	}

	return FALSE;
}