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
#ifndef TI_APP_H_
#define TI_APP_H_
 
#include "js_class.h"
#include "webkit/glue/webview.h"
 
class TiWebShell;
 
class TiApp : public JsClass
{
	TiWebShell *tiWebShell;
public:
	TiApp(TiWebShell *tiWebShell);
	~TiApp(void);

	void debug (const CppArgumentList &args, CppVariant *result);
	void getResourcePath(const CppArgumentList &args, CppVariant *result);
	void include (const CppArgumentList &args, CppVariant *result);
	void hide (const CppArgumentList &args, CppVariant *result);
	void show (const CppArgumentList &args, CppVariant *result);
	void getTitle(const CppArgumentList &args, CppVariant *result);
	void setTitle(const CppArgumentList &args, CppVariant *result);
	void getIcon(const CppArgumentList &args, CppVariant *result);
	void setIcon(const CppArgumentList &args, CppVariant *result);

	void activate(const CppArgumentList &args, CppVariant *result);	// same as restore
	void minimize(const CppArgumentList &args, CppVariant *result);
	void maximize(const CppArgumentList &args, CppVariant *result);

	void beep(const CppArgumentList &args, CppVariant *result);
	void playSound(const CppArgumentList &args, CppVariant *result);
	void playNamedSound(const CppArgumentList &args, CppVariant *result);

	void quit(const CppArgumentList &args, CppVariant *result);
};
 
#endif // TI_APP_H define
