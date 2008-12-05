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
#include "ti_chrome_window.h"
#include "ti_window_factory.h"
#include "ti_menu_factory.h"
#include "ti_media.h"
#include "ti_dock.h"

#include "Resource.h"

#include <string>
#include <fstream>

TiRuntime::TiRuntime(TiChromeWindow *window, WebFrame *webFrame)
{
	this->window = window;
	this->webFrame = webFrame;
	
	tiApp = new TiApp(this);
	tiWindowFactory = new TiWindowFactory(this);
	tiMenuFactory = new TiMenuFactory();
	tiMedia = new TiMedia();
	tiDock = new TiDock();

	App.Set(tiApp->ToNPObject());
	Window.Set(tiWindowFactory->ToNPObject());
	Menu.Set(tiMenuFactory->ToNPObject());
	Media.Set(tiMedia->ToNPObject());
	Dock.Set(tiMedia->ToNPObject());

	BindProperty("App", &App);
	BindProperty("Window", &Window);
	BindProperty("Menu", &Menu);
	BindProperty("Media", &Media);
	BindProperty("Dock", &Dock);
}

TiRuntime::~TiRuntime()
{
	App.FreeData();
	Window.FreeData();
	Menu.FreeData();
	Media.FreeData();
	Dock.FreeData();

	delete tiMedia;
	delete tiMenuFactory;
	delete tiWindowFactory;
	delete tiApp;
	delete tiDock;
}
