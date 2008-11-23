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
 
  BindMethod("show", &TiApp::show);
  BindMethod("hide", &TiApp::hide);
}
 
TiApp::~TiApp()
{
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
