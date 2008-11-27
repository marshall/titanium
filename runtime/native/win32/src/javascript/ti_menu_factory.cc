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

#include "ti_menu_factory.h"
#include "ti_menu.h"
#include "ti_chrome_window.h"

static HMENU hMenuBar = NULL;

TiMenuFactory::TiMenuFactory()
{
	BindMethod("createSystemMenu", &TiMenuFactory::createSystemMenu);
	BindMethod("createUserMenu", &TiMenuFactory::createUserMenu);
}


NOTIFYICONDATA TiMenuFactory::createTrayIcon(std::wstring &iconPath, std::wstring &caption) {
	NOTIFYICONDATA notifyIconData;
	notifyIconData.cbSize = sizeof(NOTIFYICONDATA);
	notifyIconData.hWnd = TiChromeWindow::getMainWindow()->getWindow();
	notifyIconData.uID = TiMenu::nextMenuUID();
	notifyIconData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	notifyIconData.uCallbackMessage = TI_TRAY_CLICKED;
	notifyIconData.hIcon = (HICON)LoadImage(::GetModuleHandle(NULL),
		iconPath.c_str(),
		IMAGE_ICON,
		GetSystemMetrics(SM_CXSMICON),
		GetSystemMetrics(SM_CYSMICON),
		LR_DEFAULTCOLOR | LR_LOADFROMFILE);

	lstrcpy(notifyIconData.szTip, caption.c_str());

	Shell_NotifyIcon(NIM_ADD, &notifyIconData);
	return notifyIconData;
}

void TiMenuFactory::createSystemMenu(const CppArgumentList& args, CppVariant* result)
{
	if (args.size() >= 3) {
		if (args[0].isString() && args[1].isString() && args[2].isObject()) {
			std::string iconURL = args[0].ToString();
			std::string caption = args[1].ToString();
			NPVariant variant;
			args[2].CopyToNPVariant(&variant);

			NPObject* callback = NPVARIANT_TO_OBJECT(variant);

			if (TiMenu::systemMenu == NULL) {
				TiMenu::systemMenu = new TiMenu(createTrayIcon(TiURL::getPathForURL(GURL(iconURL)), UTF8ToWide(caption)));
			}
			
			result->Set(TiMenu::systemMenu->ToNPObject());
		}
	}
}

/**
 * creates a menu item in the menu bar for the main titanium window
 *
 * if the menu bar for the main window has not been created yet, a new menu bar
 * is created
 *
 * if the menu item has already been created, then the existing menu is returned
 *
 * Sample JavaScript call:
 *   var myMenu = ti.Menu.createUserMenu("Orders");
 *   myMenu.addItem("Search Orders", searchOrdersCallback);
 *   myMenu.addItem("Add Order", addOrderCallback);
 *   myMenu.addSeparator();
 *
 *   var mySubMenu = myMenu.addSubMenu("Inventory");
 *   mySubMenu.addItem("Last Numbers", lastNumbersCallback);
 *   mySubMenu.addItem("Out Of Stock", outOfStockCallback);
 */
void TiMenuFactory::createUserMenu(const CppArgumentList& args, CppVariant* result)
{
	if (args.size() >= 1) {
		if (args[0].isString()) {
			std::string menuLabel = args[0].ToString();

			if(hMenuBar == NULL)
			{
				createMenuBar();
			}

			TiMenu *menu = new TiMenu(hMenuBar, menuLabel);

			result->Set(menu->ToNPObject());
		}
	}
}

void TiMenuFactory::createMenuBar()
{
	hMenuBar = CreateMenu();
	HWND hWnd = TiChromeWindow::getMainWindow()->getWindow();
	SetMenu(hWnd, hMenuBar);
}