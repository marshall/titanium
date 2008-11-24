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
#ifndef TI_MENU_H_
#define TI_MENU_H_

#include <string>
#include <windows.h>

#include "js_class.h"

#define TI_MENU_ITEM_ID_BEGIN 7500

/**
* Javascript wrapper for the main window menu in win32
*/
class TiMenu : public JsClass
{
private:
	std::string label;
	NPObject *callback;

	HMENU hMenu;
	int uID;

public:
	TiMenu(HMENU parentMenu, std::string& label);

	void addItem(const CppArgumentList &args, CppVariant *result);
	void addSubMenu(const CppArgumentList &args, CppVariant *result);
	void addSeparator(const CppArgumentList &args, CppVariant *result);
	/* hide and show don't make sense in the menu object
	   from JS, a call like ti.Menu.hideUserMenu()
	                        ti.Menu.showUserMenu() make more sense
	void hide(const CppArgumentList &args, CppVariant *result);
	void show(const CppArgumentList &args, CppVariant *result);
	*/

	static LRESULT CALLBACK handleMenuClick(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

#endif