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

#include "ti_system_menu.h"
#include "ti_web_shell.h"
#include "ti_url.h"

#include <cstdlib>

TiSystemMenu::TiSystemMenu(std::string& iconURL_, std::string& caption_, NPObject *callback_)
	: iconURL(iconURL_), caption(caption_), callback(callback_), menu(NULL)
{
	uID = TI_MENU_ID_BEGIN + ((int)rand()&0xFFFF);

	std::wstring path = TiURL::getPathForURL(GURL(iconURL));

	NOTIFYICONDATA tnd;
	tnd.cbSize = sizeof(NOTIFYICONDATA);
	tnd.hWnd = TiWebShell::getMainTiWebShell()->getWindow();
	tnd.uID = uID;
	tnd.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	tnd.uCallbackMessage = TI_SYSTEM_MENU_CALLBACK;
	tnd.hIcon = (HICON)LoadImage(::GetModuleHandle(NULL),
		path.c_str(),
		IMAGE_ICON,
		GetSystemMetrics(SM_CXSMICON),
		GetSystemMetrics(SM_CYSMICON),
		LR_DEFAULTCOLOR | LR_LOADFROMFILE);

	lstrcpy(tnd.szTip, UTF8ToWide(caption).c_str());

	Shell_NotifyIcon(NIM_ADD, &tnd);

	BindMethod("addItem", &TiSystemMenu::addItem);
	BindMethod("addSeparator", &TiSystemMenu::addSeparator);
	BindMethod("hide", &TiSystemMenu::hide);
	BindMethod("show", &TiSystemMenu::show);

}

void TiSystemMenu::addItem(const CppArgumentList &args, CppVariant *result)
{
	if (menu == NULL) {
		menu = CreatePopupMenu();
	}
}

void TiSystemMenu::addSeparator(const CppArgumentList &args, CppVariant *result)
{

}

void TiSystemMenu::hide(const CppArgumentList &args, CppVariant *result)
{

}

void TiSystemMenu::show(const CppArgumentList &args, CppVariant *result)
{

}