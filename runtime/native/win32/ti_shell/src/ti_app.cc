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

TiApp::TiApp(TiWebShell *tiWebShell)
{
	this->tiWebShell = tiWebShell;

	BindMethod("getTitle", &TiApp::getTitle);
	BindMethod("setTitle", &TiApp::setTitle);
	//BindMethod("getIcon(const CppArgumentList &args, CppVariant *result);
	//BindMethod("setIcon(const CppArgumentList &args, CppVariant *result);

	BindMethod("show", &TiApp::show);
	BindMethod("hide", &TiApp::hide);

	
	BindMethod("activate", &TiApp::activate);
	BindMethod("minimize", &TiApp::minimize);
	BindMethod("maximize", &TiApp::maximize);

	/*
	void beep(const CppArgumentList &args, CppVariant *result);
	void playSound(const CppArgumentList &args, CppVariant *result);
	void playNamedSound(const CppArgumentList &args, CppVariant *result);

	void quit(const CppArgumentList &args, CppVariant *result);
	*/
}

TiApp::~TiApp()
{
}


void TiApp::getTitle(const CppArgumentList &args, CppVariant *result)
{
	std::string title = this->tiWebShell->getTitle();

	result->Set(title);
}

void TiApp::setTitle(const CppArgumentList &args, CppVariant *result)
{
	if (args.size() > 0) {
		std::string title = args[0].ToString();
		this->tiWebShell->setTitle(title);
	}
}

void TiApp::getIcon(const CppArgumentList &args, CppVariant *result)
{
	// TODO
}

void TiApp::setIcon(const CppArgumentList &args, CppVariant *result)
{
	// TODO
}


void TiApp::hide (const CppArgumentList &args, CppVariant *result)
{
	this->tiWebShell->showWindow(SW_HIDE);
	this->tiWebShell->createTrayIcon();
}

void TiApp::show (const CppArgumentList &args, CppVariant *result)
{
	this->tiWebShell->showWindow(SW_SHOW);
	this->tiWebShell->removeTrayIcon();
}

void TiApp::activate(const CppArgumentList &args, CppVariant *result)
{
	this->tiWebShell->showWindow(SW_RESTORE);
}

void TiApp::minimize(const CppArgumentList &args, CppVariant *result)
{
	this->tiWebShell->showWindow(SW_MINIMIZE);
}

void TiApp::maximize(const CppArgumentList &args, CppVariant *result)
{
	this->tiWebShell->showWindow(SW_MAXIMIZE);
}

void TiApp::beep(const CppArgumentList &args, CppVariant *result)
{
	// TODO
}

void TiApp::playSound(const CppArgumentList &args, CppVariant *result)
{
	// TODO
}

void TiApp::playNamedSound(const CppArgumentList &args, CppVariant *result)
{
	// TODO
}

void TiApp::quit(const CppArgumentList &args, CppVariant *result)
{
	// TODO
}
