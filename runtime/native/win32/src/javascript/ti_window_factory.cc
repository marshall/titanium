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

#include "ti_window_factory.h"

TiWindowFactory::TiWindowFactory(TiChromeWindow *window)
{
	BindMethod("createWindow", &TiWindowFactory::createWindow);
	BindMethod("getWindow", &TiWindowFactory::getWindow);
	BindProperty("mainWindow", &mainWindow);
	BindProperty("currentWindow", &currentWindow);

	mainWindow.Set(TiChromeWindow::getMainWindow()->getTiUserWindow()->ToNPObject());
	currentWindow.Set(window->getTiUserWindow()->ToNPObject());
}

void TiWindowFactory::createWindow(const CppArgumentList &args, CppVariant *result)
{
	TiUserWindow *window = new TiUserWindow();

	result->Set(window->ToNPObject());
}

void TiWindowFactory::getWindow(const CppArgumentList &args, CppVariant *result)
{
	if (args.size() > 0 && args[0].isString()) {
		TiChromeWindow *window = TiChromeWindow::getWindow(args[0].ToString().c_str());

		if (window != NULL) {
			result->Set(window->getTiUserWindow()->ToNPObject());
		} else {
			result->SetNull();
		}
	}
}