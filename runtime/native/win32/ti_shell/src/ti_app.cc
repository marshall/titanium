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
#include "ti_web_shell.h"

TiApp::TiApp(TiWebShell *tiWebShell_)
	: tiWebShell(tiWebShell_)
{
	BindMethod("show", &TiApp::show);
	BindMethod("hide", &TiApp::hide);
	BindMethod("debug", &TiApp::debug);
	BindMethod("getResourcePath", &TiApp::getResourcePath);
	BindMethod("include", &TiApp::include);
}
 
TiApp::~TiApp()
{
}
 
void TiApp::hide (const CppArgumentList &args, CppVariant *result)
{
	std::vector<TiWebShell*>::iterator iter = TiWebShell::getOpenShells().begin();
	for (; iter != TiWebShell::getOpenShells().end(); iter++) {
		TiWebShell *openShell = (*iter);
		
		openShell->showWindow(SW_HIDE);
	}
}
 
void TiApp::show (const CppArgumentList &args, CppVariant *result)
{
	std::vector<TiWebShell*>::iterator iter = TiWebShell::getOpenShells().begin();
	for (; iter != TiWebShell::getOpenShells().end(); iter++) {
		TiWebShell *openShell = (*iter);
		
		openShell->showWindow(SW_SHOW);
	}
}

void TiApp::debug (const CppArgumentList &args, CppVariant *result)
{
	if (args.size() > 0)
		printf("[titanium:debug]: %s\n", args[0].ToString().c_str());
}

void TiApp::getResourcePath(const CppArgumentList &args, CppVariant *result)
{
	std::wstring resourcePath = TiWebShell::getTiAppConfig()->getResourcePath();

	result->Set(WideToUTF8(resourcePath));
}

void TiApp::include(const CppArgumentList &args, CppVariant *result)
{
	if (args.size() > 0) {
		std::string relativeName = args[0].ToString();
		tiWebShell->include(relativeName);
	}
}