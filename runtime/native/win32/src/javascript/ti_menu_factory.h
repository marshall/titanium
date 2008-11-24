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
#ifndef TI_MENU_FACTORY_H_
#define TI_MENU_FACTORY_H_

#include "js_class.h"
#include "webkit/glue/webview.h"
#include <shellapi.h>

class TiMenuFactory : public JsClass
{
private:
	void TiMenuFactory::createMenuBar();

	NOTIFYICONDATA createTrayIcon(std::wstring &iconPath, std::wstring &caption);
	void removeTrayIcon();
public:
	TiMenuFactory();

	void createSystemMenu(const CppArgumentList& args, CppVariant* result);
	void createUserMenu(const CppArgumentList& args, CppVariant* result);
};

#endif