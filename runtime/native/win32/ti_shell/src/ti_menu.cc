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

			// TODO replace 789 with a good callback msg ID
			// TODO need to add logic to handle menu item clicks
			// NPN_Invoke(object, method, args)
			AppendMenu(this->hMenu, MF_STRING, 789, (LPCTSTR) UTF8ToWide(label).c_str());
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