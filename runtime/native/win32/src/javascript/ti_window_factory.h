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
#ifndef TI_WINDOW_JS_H_
#define TI_WINDOW_JS_H_

#include "js_class.h"
#include "webkit/glue/webview.h"

#include "ti_user_window.h"

/**
* A simple factory for Window objects
*/
class TiWindowFactory : public JsClass
{
public:
	TiWindowFactory(TiWebShell *tiWebShell);

	void createWindow(const CppArgumentList& args, CppVariant* result);

	CppVariant mainWindow, currentWindow;
};

#endif