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
#ifndef TI_NATIVE_H_
#define TI_NATIVE_H_

#include "js_class.h"
#include "webkit/glue/webview.h"
#include "webkit/glue/webframe.h"

#include "ti_app.h"

class TiChromeWindow;
class TiWindowFactory;
class TiMenuFactory;

class TiRuntime : public JsClass
{
	TiChromeWindow *window;
	TiApp *tiApp;
	TiWindowFactory *tiWindowFactory;
	TiMenuFactory *tiMenuFactory;
	WebFrame *webFrame;

public:
	TiRuntime(TiChromeWindow *window, WebFrame *webFrame);
	~TiRuntime(void);

	TiChromeWindow* getWindow() { return window; }
	WebFrame* getWebFrame() { return webFrame; }

	CppVariant App, Dock, Menu, Window;
};

#endif
