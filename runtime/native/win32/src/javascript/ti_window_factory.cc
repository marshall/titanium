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
#include "ti_runtime.h"

TiWindowFactory::TiWindowFactory(TiRuntime *ti)
{
	BindMethod("createWindow", &TiWindowFactory::createWindow);
	BindMethod("getWindow", &TiWindowFactory::getWindow);
	BindProperty("mainWindow", &mainWindow);
	BindProperty("currentWindow", &currentWindow);

	mainWindow.Set(TiChromeWindow::getMainWindow()->getTiUserWindow()->ToNPObject());
	currentWindow.Set(ti->getWindow()->getTiUserWindow()->ToNPObject());
}

void TiWindowFactory::createWindow(const CppArgumentList &args, CppVariant *result)
{
	TiUserWindow *window;

	if (args.size() > 0) {
		if (args[0].isString()) {
			std::string id = args[0].ToString();
			if (args.size() > 1) {
				window = new TiUserWindow(id.c_str(), args[1].ToBoolean());
			}
			else {
				window = new TiUserWindow(id.c_str());
			}
		} else if (args[0].isObject()) {
			TiWindowConfig *config = new TiWindowConfig();

#define BindObjectAttribute(obj, attr, bindObj, setter, type) if (ObjectHasProperty(obj, attr)) { bindObj->setter(type(obj, attr)); }
#define BindStringAttribute(obj, attr, bindObj, setter) BindObjectAttribute(obj,attr,bindObj,setter,GetStringProperty)
#define BindBoolAttribute(obj, attr, bindObj, setter) BindObjectAttribute(obj,attr,bindObj,setter,GetBoolProperty)
#define BindDoubleAttribute(obj, attr, bindObj, setter) BindObjectAttribute(obj,attr,bindObj,setter,GetDoubleProperty)
#define BindIntAttribute(obj, attr, bindObj, setter) BindObjectAttribute(obj,attr,bindObj,setter,GetIntProperty)

			BindStringAttribute(args[0], "id", config, setId);
			BindBoolAttribute(args[0], "usingChrome", config, setUsingChrome);
			BindBoolAttribute(args[0], "usingScrollbars", config, setUsingScrollbars);
			BindIntAttribute(args[0], "x", config, setX);
			BindIntAttribute(args[0], "y", config, setY);
			BindIntAttribute(args[0], "width", config, setWidth);
			BindIntAttribute(args[0], "height", config, setHeight);
			BindDoubleAttribute(args[0], "transparency", config, setTransparency);
			BindStringAttribute(args[0], "url", config, setURL);

			window = new TiUserWindow(config);
		}
	} else {
		window = new TiUserWindow();
	}

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