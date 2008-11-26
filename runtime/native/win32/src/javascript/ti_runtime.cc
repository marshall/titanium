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
#include "ti_runtime.h"
#include "ti_web_shell.h"
#include "ti_window_factory.h"
#include "ti_menu_factory.h"
#include "Resource.h"

#include <string>
#include <fstream>

TiRuntime::TiRuntime(TiWebShell *tiWebShell)
{
	this->tiWebShell = tiWebShell;
	
	tiApp = new TiApp(tiWebShell);
	tiWindowFactory = new TiWindowFactory(tiWebShell);
	tiMenuFactory = new TiMenuFactory();

	App.Set(tiApp->ToNPObject());
	Window.Set(tiWindowFactory->ToNPObject());
	Menu.Set(tiMenuFactory->ToNPObject());

	BindProperty("App", &App);
	BindProperty("Window", &Window);
	BindProperty("Menu", &Menu);
}

TiRuntime::~TiRuntime()
{
}