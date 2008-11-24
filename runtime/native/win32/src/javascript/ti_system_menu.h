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
#ifndef TI_SYSTEM_MENU_H_
#define TI_SYSTEM_MENU_H_

#include "js_class.h"
#include "webkit/glue/webview.h"
#include "ti_menu.h"

#include <string>

#define TI_MENU_ID_BEGIN 1500
#define TI_SYSTEM_MENU_CALLBACK 1600
/**
* Javascript wrapper for the tray menu in win32
*/
class TiSystemMenu : public TiMenu
{
private:
	NPObject *callback;
	HMENU menu;
	int uID;
	std::string iconURL;
	std::string caption;

public:
	TiSystemMenu(std::string& iconURL, std::string& caption, NPObject *callback);

	void addItem(const CppArgumentList &args, CppVariant *result);
	void addSeparator(const CppArgumentList &args, CppVariant *result);
	void hide(const CppArgumentList &args, CppVariant *result);
	void show(const CppArgumentList &args, CppVariant *result);
};

#endif