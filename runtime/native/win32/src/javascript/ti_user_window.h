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

#ifndef TI_USER_WINDOW_H_
#define TI_USER_WINDOW_H_

#include "js_class.h"
#include "webkit/glue/webview.h"

#include "ti_web_shell.h"
#include "ti_window.h"

class TiUserWindow : public JsClass
{
private:
	TiWebShell *tiWebShell;
	TiWindow *tiWindow;

	void updateWindow();
	void bind();

public:
	TiUserWindow();
	TiUserWindow(TiWebShell *tiWebShell);

	void hide(const CppArgumentList &args, CppVariant *result);
	void show(const CppArgumentList &args, CppVariant *result);
	void close(const CppArgumentList &args, CppVariant *result);

	void getURL(const CppArgumentList &args, CppVariant *result);
	void setURL(const CppArgumentList &args, CppVariant *result);
	
	void getTitle(const CppArgumentList &args, CppVariant *result);
	void setTitle(const CppArgumentList &args, CppVariant *result);

	void getIcon(const CppArgumentList &args, CppVariant *result);
	void setIcon(const CppArgumentList &args, CppVariant *result);

	void getBounds(const CppArgumentList &args, CppVariant *result);
	void setBounds(const CppArgumentList &args, CppVariant *result);

	void getHeight(const CppArgumentList &args, CppVariant *result);
	void setHeight(const CppArgumentList &args, CppVariant *result);

	void getWidth(const CppArgumentList &args, CppVariant *result);
	void setWidth(const CppArgumentList &args, CppVariant *result);

	void getX(const CppArgumentList &args, CppVariant *result);
	void setX(const CppArgumentList &args, CppVariant *result);
	void getY(const CppArgumentList &args, CppVariant *result);
	void setY(const CppArgumentList &args, CppVariant *result);

	void isResizable(const CppArgumentList &args, CppVariant *result);
	void setResizable(const CppArgumentList &args, CppVariant *result);

	void isMaximizable(const CppArgumentList &args, CppVariant *result);
	void setMaximizable(const CppArgumentList &args, CppVariant *result);

	void isMinimizable(const CppArgumentList &args, CppVariant *result);
	void setMinimizable(const CppArgumentList &args, CppVariant *result);

	void isCloseable(const CppArgumentList &args, CppVariant *result);
	void setCloseable(const CppArgumentList &args, CppVariant *result);

	void isFullscreen(const CppArgumentList &args, CppVariant *result);
	void setFullscreen(const CppArgumentList &args, CppVariant *result);

	void isVisible(const CppArgumentList &args, CppVariant *result);
	void setVisible(const CppArgumentList &args, CppVariant *result);

	void isUsingChrome(const CppArgumentList &args, CppVariant *result);
	void setUsingChrome(const CppArgumentList &args, CppVariant *result);

	void isUsingScrollbars(const CppArgumentList &args, CppVariant *result);
	void setUsingScrollbars(const CppArgumentList &args, CppVariant *result);

	void getTransparency(const CppArgumentList &args, CppVariant *result);
	void setTransparency(const CppArgumentList &args, CppVariant *result);

	void activate(const CppArgumentList &args, CppVariant *result);	// same as restore
	void minimize(const CppArgumentList &args, CppVariant *result);
	void maximize(const CppArgumentList &args, CppVariant *result);
};

#endif