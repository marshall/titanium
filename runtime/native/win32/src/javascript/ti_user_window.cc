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

#include "ti_user_window.h"

TiUserWindow::TiUserWindow(TiWebShell *tiWebShell_)
{
	tiWebShell = tiWebShell_;
	if (tiWebShell != NULL)
		tiWindow = tiWebShell->getTiWindow();

	bind();
}

TiUserWindow::TiUserWindow ()
{
	tiWindow = new TiWindow();
	tiWebShell = new TiWebShell(tiWindow);
	tiWebShell->open();

	bind();
}

void TiUserWindow::bind()
{
	BindMethod("hide", &TiUserWindow::hide);
	BindMethod("show", &TiUserWindow::show);
	BindMethod("close", &TiUserWindow::close);
	BindMethod("activate", &TiUserWindow::activate);
	BindMethod("minimize", &TiUserWindow::minimize);
	BindMethod("maximize", &TiUserWindow::maximize);

	// snooore -- getters and setters
	BindMethod("getX", &TiUserWindow::getX);
	BindMethod("setX", &TiUserWindow::setX);
	BindMethod("getY", &TiUserWindow::getY);
	BindMethod("setY", &TiUserWindow::setY);
	BindMethod("getURL", &TiUserWindow::getURL);
	BindMethod("setURL", &TiUserWindow::setURL);
	BindMethod("getTitle", &TiUserWindow::getTitle);
	BindMethod("setTitle", &TiUserWindow::setTitle);
	//BindMethod("getIcon", &TiUserWindow::getIcon);
	//BindMethod("setIcon", &TiUserWindow::setIcon);
	BindMethod("getBounds", &TiUserWindow::getBounds);
	BindMethod("setBounds", &TiUserWindow::setBounds);
	BindMethod("getHeight", &TiUserWindow::getHeight);
	BindMethod("setHeight", &TiUserWindow::setHeight);
	BindMethod("getWidth", &TiUserWindow::getWidth);
	BindMethod("setWidth", &TiUserWindow::setWidth);
	BindMethod("isResizable", &TiUserWindow::isResizable);
	BindMethod("setResizable", &TiUserWindow::setResizable);
	BindMethod("isMaximizable", &TiUserWindow::isMaximizable);
	BindMethod("setMaximizable", &TiUserWindow::setMaximizable);
	BindMethod("isMinimizable", &TiUserWindow::isMinimizable);
	BindMethod("setMinimizable", &TiUserWindow::setMinimizable);
	BindMethod("isCloseable", &TiUserWindow::isCloseable);
	BindMethod("setCloseable", &TiUserWindow::setCloseable);
	BindMethod("isFullscreen", &TiUserWindow::isFullscreen);
	BindMethod("setFullscreen", &TiUserWindow::setFullscreen);
	BindMethod("isVisible", &TiUserWindow::isVisible);
	BindMethod("setVisible", &TiUserWindow::setVisible);
	BindMethod("isUsingChrome", &TiUserWindow::isUsingChrome);
	BindMethod("setUsingChrome", &TiUserWindow::setUsingChrome);
	BindMethod("isUsingScrollbars", &TiUserWindow::isUsingScrollbars);
	BindMethod("setUsingScrollbars", &TiUserWindow::setUsingScrollbars);
	BindMethod("getTransparency", &TiUserWindow::getTransparency);
	BindMethod("setTransparency", &TiUserWindow::setTransparency);
}

void TiUserWindow::updateWindow ()
{
	if (tiWebShell != NULL)
		tiWebShell->reloadTiWindow();
}

void TiUserWindow::hide(const CppArgumentList &args, CppVariant *result)
{
	tiWebShell->showWindow(SW_HIDE);
}

void TiUserWindow::show(const CppArgumentList &args, CppVariant *result)
{
	tiWebShell->showWindow(SW_SHOW);
}

void TiUserWindow::close(const CppArgumentList &args, CppVariant *result)
{
	tiWebShell->close();
}

void TiUserWindow::getURL(const CppArgumentList &args, CppVariant *result)
{
	result->Set(tiWindow->getURL());
}

void TiUserWindow::setURL(const CppArgumentList &args, CppVariant *result)
{
	if (args.size() > 0 && args[0].isString()) {
		tiWindow->setURL(args[0].ToString());
		updateWindow();
	}
}

void TiUserWindow::getTitle(const CppArgumentList &args, CppVariant *result)
{
	result->Set(tiWindow->getTitle());
}

void TiUserWindow::setTitle(const CppArgumentList &args, CppVariant *result)
{
	if (args.size() > 0 && args[0].isString()) {
		tiWindow->setTitle(args[0].ToString());
		updateWindow();
	}
}

void TiUserWindow::getIcon(const CppArgumentList &args, CppVariant *result)
{
	// ?
}

void TiUserWindow::setIcon(const CppArgumentList &args, CppVariant *result)
{
	// ?
}


class TiWindowBounds : public JsClass
{
public:
	CppVariant x, y, width, height;

	TiWindowBounds() {
		BindProperty("x", &x);
		BindProperty("y", &y);
		BindProperty("width", &width);
		BindProperty("height", &height);
	}
};

void TiUserWindow::getBounds(const CppArgumentList &args, CppVariant *result)
{
	TiWindowBounds *bounds = new TiWindowBounds();

	bounds->x.Set(tiWindow->getX());
	bounds->y.Set(tiWindow->getY());
	bounds->width.Set(tiWindow->getWidth());
	bounds->height.Set(tiWindow->getHeight());

	result->Set(bounds->ToNPObject());
}

void TiUserWindow::setBounds(const CppArgumentList &args, CppVariant *result)
{
	if (args.size() > 0 && args[0].isObject()) {
		int x = GetIntProperty(args[0], "x");
		int y = GetIntProperty(args[0], "y");
		int width = GetIntProperty(args[0], "width");
		int height = GetIntProperty(args[0], "height");

		if (x >= 0)
			tiWindow->setX(x);

		if (y >= 0)
			tiWindow->setY(y);

		if (width >= 0)
			tiWindow->setWidth(width);

		if (height >= 0)
			tiWindow->setHeight(height);

		updateWindow();
	}
}

void TiUserWindow::getHeight(const CppArgumentList &args, CppVariant *result)
{
	result->Set(tiWindow->getHeight());
}

void TiUserWindow::setHeight(const CppArgumentList &args, CppVariant *result)
{
	if (args.size() > 0 && args[0].isNumber()) {
		tiWindow->setHeight((int)args[0].ToInt32());
		updateWindow();
	}
}

void TiUserWindow::getWidth(const CppArgumentList &args, CppVariant *result)
{
	result->Set(tiWindow->getWidth());
}

void TiUserWindow::setWidth(const CppArgumentList &args, CppVariant *result)
{
	if (args.size() > 0 && args[0].isNumber()) {
		tiWindow->setWidth((int)args[0].ToInt32());
		updateWindow();
	}
}

void TiUserWindow::getX(const CppArgumentList &args, CppVariant *result)
{
	result->Set(tiWindow->getX());
}

void TiUserWindow::setX(const CppArgumentList &args, CppVariant *result)
{
	if (args.size() > 0 && args[0].isNumber()) {
		tiWindow->setX((int)args[0].ToInt32());
		updateWindow();
	}
}

void TiUserWindow::getY(const CppArgumentList &args, CppVariant *result)
{
	result->Set(tiWindow->getY());
}

void TiUserWindow::setY(const CppArgumentList &args, CppVariant *result)
{
	if (args.size() > 0 && args[0].isNumber()) {
		tiWindow->setY((int)args[0].ToInt32());
		updateWindow();
	}
}

void TiUserWindow::isResizable(const CppArgumentList &args, CppVariant *result)
{
	result->Set(tiWindow->isResizable());
}

void TiUserWindow::setResizable(const CppArgumentList &args, CppVariant *result)
{
	if (args.size() > 0 && args[0].isBool())
	{
		tiWindow->setResizable(args[0].ToBoolean());
		updateWindow();
	}
}

void TiUserWindow::isMaximizable(const CppArgumentList &args, CppVariant *result)
{
	result->Set(tiWindow->isMaximizable());
}

void TiUserWindow::setMaximizable(const CppArgumentList &args, CppVariant *result)
{
	if (args.size() > 0 && args[0].isBool())
	{
		tiWindow->setMaximizable(args[0].ToBoolean());
		updateWindow();
	}
}

void TiUserWindow::isMinimizable(const CppArgumentList &args, CppVariant *result)
{
	result->Set(tiWindow->isMinimizable());
}

void TiUserWindow::setMinimizable(const CppArgumentList &args, CppVariant *result)
{
	if (args.size() > 0 && args[0].isBool())
	{
		tiWindow->setMinimizable(args[0].ToBoolean());
		updateWindow();
	}
}

void TiUserWindow::isCloseable(const CppArgumentList &args, CppVariant *result)
{
	result->Set(tiWindow->isCloseable());
}

void TiUserWindow::setCloseable(const CppArgumentList &args, CppVariant *result)
{
	if (args.size() > 0 && args[0].isBool())
	{
		tiWindow->setCloseable(args[0].ToBoolean());
		updateWindow();
	}
}

void TiUserWindow::isFullscreen(const CppArgumentList &args, CppVariant *result)
{
	result->Set(tiWindow->isFullscreen());
}

void TiUserWindow::setFullscreen(const CppArgumentList &args, CppVariant *result)
{
	if (args.size() > 0 && args[0].isBool())
	{
		tiWindow->setFullscreen(args[0].ToBoolean());
		updateWindow();
	}
}

void TiUserWindow::isVisible(const CppArgumentList &args, CppVariant *result)
{
	result->Set(tiWindow->isVisible());
}

void TiUserWindow::setVisible(const CppArgumentList &args, CppVariant *result)
{
	if (args.size() > 0 && args[0].isBool())
	{
		tiWindow->setVisible(args[0].ToBoolean());
		updateWindow();
	}
}

void TiUserWindow::isUsingChrome(const CppArgumentList &args, CppVariant *result)
{
	result->Set(tiWindow->isUsingChrome());
}

void TiUserWindow::setUsingChrome(const CppArgumentList &args, CppVariant *result)
{
	if (args.size() > 0 && args[0].isBool())
	{
		tiWindow->setUsingChrome(args[0].ToBoolean());
		updateWindow();
	}
}

void TiUserWindow::isUsingScrollbars(const CppArgumentList &args, CppVariant *result)
{
	result->Set(tiWindow->isUsingScrollbars());
}

void TiUserWindow::setUsingScrollbars(const CppArgumentList &args, CppVariant *result)
{
	if (args.size() > 0 && args[0].isBool())
	{
		tiWindow->setUsingScrollbars(args[0].ToBoolean());
		updateWindow();
	}
}

void TiUserWindow::getTransparency(const CppArgumentList &args, CppVariant *result)
{
	result->Set(tiWindow->getTransparency());
}

void TiUserWindow::setTransparency(const CppArgumentList &args, CppVariant *result)
{
	if (args.size() > 0 && args[0].isDouble())
	{
		tiWindow->setTransparency((float)args[0].ToDouble());
		updateWindow();
	}
}

void TiUserWindow::activate(const CppArgumentList &args, CppVariant *result)
{
	this->tiWebShell->showWindow(SW_RESTORE);
}

void TiUserWindow::minimize(const CppArgumentList &args, CppVariant *result)
{
	this->tiWebShell->showWindow(SW_MINIMIZE);
}

void TiUserWindow::maximize(const CppArgumentList &args, CppVariant *result)
{
	this->tiWebShell->showWindow(SW_MAXIMIZE);
}