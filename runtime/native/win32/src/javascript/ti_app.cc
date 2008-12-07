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
 
#include "ti_app.h"
#include "ti_chrome_window.h"
#include "ti_runtime.h"
#include "ti_app_config.h"
#include "base/file_util.h"
#include "base/path_service.h"

TiApp::TiApp(TiRuntime *ti_)
	: ti(ti_)
{
	BindMethod("show", &TiApp::show);
	BindMethod("hide", &TiApp::hide);
	BindMethod("debug", &TiApp::debug);
	BindMethod("getResourcePath", &TiApp::getResourcePath);
	BindMethod("include", &TiApp::include);
	BindMethod("toString", &TiApp::toString);
	BindMethod("quit", &TiApp::quit);

	BindMethod("getID", &TiApp::getID);
	BindMethod("getGUID", &TiApp::getGUID);
	BindMethod("getUpdateURL", &TiApp::getUpdateURL);
	BindMethod("getVersion", &TiApp::getVersion);
	BindMethod("getName", &TiApp::getName);

}
 
TiApp::~TiApp()
{
}
 
void TiApp::hide (const CppArgumentList &args, CppVariant *result)
{
	std::vector<TiChromeWindow*>::iterator iter = TiChromeWindow::getOpenWindows().begin();
	for (; iter != TiChromeWindow::getOpenWindows().end(); iter++) {
		TiChromeWindow *openWindow = (*iter);
		
		openWindow->showWindow(SW_HIDE);
	}
}
 
void TiApp::show (const CppArgumentList &args, CppVariant *result)
{
	std::vector<TiChromeWindow*>::iterator iter = TiChromeWindow::getOpenWindows().begin();
	for (; iter != TiChromeWindow::getOpenWindows().end(); iter++) {
		TiChromeWindow *openWindow = (*iter);
		
		openWindow->showWindow(SW_SHOW);
	}
}

void TiApp::debug (const CppArgumentList &args, CppVariant *result)
{
	if (args.size() > 0)
		printf("[titanium:debug]: %s\n", args[0].ToString().c_str());
}

void TiApp::getResourcePath(const CppArgumentList &args, CppVariant *result)
{
	std::wstring resourcePath = TiAppConfig::instance()->getResourcePath();

	result->Set(WideToUTF8(resourcePath));
}

void TiApp::include(const CppArgumentList &args, CppVariant *result)
{
	if (args.size() > 0) {
		std::string relativeName = args[0].ToString();
		ti->getWindow()->include(ti->getWebFrame(), relativeName);
	}
}

void TiApp::quit(const CppArgumentList &args, CppVariant *result)
{
	TiChromeWindow::DestroyWindow(TiChromeWindow::getMainWindow());
}

void TiApp::getID(const CppArgumentList &args, CppVariant *result)
{
	result->Set(TiAppConfig::instance()->getAppID());
}

void TiApp::getGUID(const CppArgumentList &args, CppVariant *result)
{
	std::string guid;
	std::wstring path;
	PathService::Get(base::DIR_EXE, &path);
	file_util::AppendToPath(&path, L"Resources");
	file_util::AppendToPath(&path, L"aid");

	file_util::ReadFileToString(path, &guid);
	result->Set(guid);
}

void TiApp::getUpdateURL(const CppArgumentList &args, CppVariant *result)
{
	result->Set(TiAppConfig::instance()->getUpdateSite());
}

void TiApp::getVersion(const CppArgumentList &args, CppVariant *result)
{
	result->Set(TiAppConfig::instance()->getVersion());
}

void TiApp::getName(const CppArgumentList &args, CppVariant *result)
{
	result->Set(TiAppConfig::instance()->getAppName());
}


void TiApp::toString(const CppArgumentList &args, CppVariant *result)
{
	std::string str = "[TiApp native]";

	result->Set(str);
}
