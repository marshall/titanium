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

TiUserWindow::TiUserWindow(TiChromeWindow *window_)
{
	window = window_;
	if (window != NULL)
		config = window->getTiWindowConfig();

	bind();
}

TiUserWindow::TiUserWindow (const char *id, bool usingChrome)
{
	config = new TiWindowConfig();
	if (id != NULL)
		config->setId(id);
	
	config->setUsingChrome(usingChrome);
	
	window = new TiChromeWindow(TiChromeWindow::getMainWindow()->getInstanceHandle(), config);
	window->setOpenOnLoad(false);
	
	bind();
}

TiUserWindow::TiUserWindow(TiWindowConfig *config_)
	: config(config_)
{
	window = new TiChromeWindow(TiChromeWindow::getMainWindow()->getInstanceHandle(), config);
	window->setOpenOnLoad(false);
	
	bind();
}

void TiUserWindow::bind()
{
	BindMethod("hide", &TiUserWindow::hide);
	BindMethod("show", &TiUserWindow::show);
	BindMethod("open", &TiUserWindow::open);
	BindMethod("close", &TiUserWindow::close);
	BindMethod("activate", &TiUserWindow::activate);
	BindMethod("minimize", &TiUserWindow::minimize);
	BindMethod("maximize", &TiUserWindow::maximize);

	// snooore -- getters and setters
	BindPropertyFunctions("id", &TiUserWindow::getID);
	BindPropertyFunctions("x", &TiUserWindow::getX, &TiUserWindow::setX);
	BindPropertyFunctions("y", &TiUserWindow::getY, &TiUserWindow::setY);
	BindPropertyFunctions("width", &TiUserWindow::getWidth, &TiUserWindow::setHeight);
	BindPropertyFunctions("height", &TiUserWindow::getHeight, &TiUserWindow::setHeight);
	BindPropertyFunctions("url", &TiUserWindow::getURL, &TiUserWindow::setURL);
	BindPropertyFunctions("title", &TiUserWindow::getTitle, &TiUserWindow::setTitle);
	BindPropertyFunctions("bounds", &TiUserWindow::getBounds, &TiUserWindow::setBounds);
	BindPropertyFunctions("resizable", &TiUserWindow::isResizable, &TiUserWindow::setResizable);
	BindPropertyFunctions("maximizable", &TiUserWindow::isMaximizable, &TiUserWindow::setMaximizable);
	BindPropertyFunctions("minimizable", &TiUserWindow::isMinimizable, &TiUserWindow::setMinimizable);
	BindPropertyFunctions("closeable", &TiUserWindow::isCloseable, &TiUserWindow::setCloseable);
	BindPropertyFunctions("fullscreen", &TiUserWindow::isFullscreen, &TiUserWindow::setFullscreen);
	BindPropertyFunctions("visible", &TiUserWindow::isVisible, &TiUserWindow::setVisible);
	BindPropertyFunctions("usingChrome", &TiUserWindow::isUsingChrome);
	BindPropertyFunctions("usingScrollbars", &TiUserWindow::isUsingScrollbars, &TiUserWindow::setUsingScrollbars);
	BindPropertyFunctions("transparency", &TiUserWindow::getTransparency, &TiUserWindow::setTransparency);

	BindMethod("getID", &TiUserWindow::getID);
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
	BindMethod("isUsingScrollbars", &TiUserWindow::isUsingScrollbars);
	BindMethod("setUsingScrollbars", &TiUserWindow::setUsingScrollbars);
	BindMethod("getTransparency", &TiUserWindow::getTransparency);
	BindMethod("setTransparency", &TiUserWindow::setTransparency);
}

void TiUserWindow::updateWindow ()
{
	if (window != NULL)
		window->reloadTiWindowConfig();
}

void TiUserWindow::hide(const CppArgumentList &args, CppVariant *result)
{
	config->setVisible(false);
	window->showWindow(SW_HIDE);
}

void TiUserWindow::show(const CppArgumentList &args, CppVariant *result)
{
	config->setVisible(true);
	window->showWindow(SW_SHOW);
}

void TiUserWindow::open(const CppArgumentList &args, CppVariant *result)
{
	window->open();
}

void TiUserWindow::close(const CppArgumentList &args, CppVariant *result)
{
	window->close();
}

void TiUserWindow::getID(const CppArgumentList &args, CppVariant *result)
{
	result->Set(config->getId());
}

void TiUserWindow::getURL(const CppArgumentList &args, CppVariant *result)
{
	result->Set(config->getURL());
}

void TiUserWindow::setURL(const CppArgumentList &args, CppVariant *result)
{
	if (args.size() > 0 && args[0].isString()) {
		config->setURL(args[0].ToString());
		if (window != NULL)
			window->maybeLoadURL(config->getURL().c_str());
	}
}

void TiUserWindow::getTitle(const CppArgumentList &args, CppVariant *result)
{
	result->Set(config->getTitle());
}

void TiUserWindow::setTitle(const CppArgumentList &args, CppVariant *result)
{
	if (args.size() > 0 && args[0].isString()) {
		config->setTitle(args[0].ToString());
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

	bounds->x.Set(config->getX());
	bounds->y.Set(config->getY());
	bounds->width.Set(config->getWidth());
	bounds->height.Set(config->getHeight());

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
			config->setX(x);

		if (y >= 0)
			config->setY(y);

		if (width >= 0)
			config->setWidth(width);

		if (height >= 0)
			config->setHeight(height);

		updateWindow();
	}
}

void TiUserWindow::getHeight(const CppArgumentList &args, CppVariant *result)
{
	result->Set(config->getHeight());
}

void TiUserWindow::setHeight(const CppArgumentList &args, CppVariant *result)
{
	if (args.size() > 0 && args[0].isNumber()) {
		config->setHeight((int)args[0].ToInt32());
		updateWindow();
	}
}

void TiUserWindow::getWidth(const CppArgumentList &args, CppVariant *result)
{
	result->Set(config->getWidth());
}

void TiUserWindow::setWidth(const CppArgumentList &args, CppVariant *result)
{
	if (args.size() > 0 && args[0].isNumber()) {
		config->setWidth((int)args[0].ToInt32());
		updateWindow();
	}
}

void TiUserWindow::getX(const CppArgumentList &args, CppVariant *result)
{
	result->Set(config->getX());
}

void TiUserWindow::setX(const CppArgumentList &args, CppVariant *result)
{
	if (args.size() > 0 && args[0].isNumber()) {
		config->setX((int)args[0].ToInt32());
		updateWindow();
	}
}

void TiUserWindow::getY(const CppArgumentList &args, CppVariant *result)
{
	result->Set(config->getY());
}

void TiUserWindow::setY(const CppArgumentList &args, CppVariant *result)
{
	if (args.size() > 0 && args[0].isNumber()) {
		config->setY((int)args[0].ToInt32());
		updateWindow();
	}
}

void TiUserWindow::isResizable(const CppArgumentList &args, CppVariant *result)
{
	result->Set(config->isResizable());
}

void TiUserWindow::setResizable(const CppArgumentList &args, CppVariant *result)
{
	if (args.size() > 0 && args[0].isBool())
	{
		config->setResizable(args[0].ToBoolean());
		updateWindow();
	}
}

void TiUserWindow::isMaximizable(const CppArgumentList &args, CppVariant *result)
{
	result->Set(config->isMaximizable());
}

void TiUserWindow::setMaximizable(const CppArgumentList &args, CppVariant *result)
{
	if (args.size() > 0 && args[0].isBool())
	{
		config->setMaximizable(args[0].ToBoolean());
		updateWindow();
	}
}

void TiUserWindow::isMinimizable(const CppArgumentList &args, CppVariant *result)
{
	result->Set(config->isMinimizable());
}

void TiUserWindow::setMinimizable(const CppArgumentList &args, CppVariant *result)
{
	if (args.size() > 0 && args[0].isBool())
	{
		config->setMinimizable(args[0].ToBoolean());
		updateWindow();
	}
}

void TiUserWindow::isCloseable(const CppArgumentList &args, CppVariant *result)
{
	result->Set(config->isCloseable());
}

void TiUserWindow::setCloseable(const CppArgumentList &args, CppVariant *result)
{
	if (args.size() > 0 && args[0].isBool())
	{
		config->setCloseable(args[0].ToBoolean());
		updateWindow();
	}
}

void TiUserWindow::isFullscreen(const CppArgumentList &args, CppVariant *result)
{
	result->Set(config->isFullscreen());
}

void TiUserWindow::setFullscreen(const CppArgumentList &args, CppVariant *result)
{
	if (args.size() > 0 && args[0].isBool())
	{
		config->setFullscreen(args[0].ToBoolean());
		updateWindow();
	}
}

void TiUserWindow::isVisible(const CppArgumentList &args, CppVariant *result)
{
	result->Set(config->isVisible());
}

void TiUserWindow::setVisible(const CppArgumentList &args, CppVariant *result)
{
	if (args.size() > 0 && args[0].isBool())
	{
		config->setVisible(args[0].ToBoolean());
		updateWindow();
	}
}

void TiUserWindow::isUsingChrome(const CppArgumentList &args, CppVariant *result)
{
	result->Set(config->isUsingChrome());
}

void TiUserWindow::isUsingScrollbars(const CppArgumentList &args, CppVariant *result)
{
	result->Set(config->isUsingScrollbars());
}

void TiUserWindow::setUsingScrollbars(const CppArgumentList &args, CppVariant *result)
{
	if (args.size() > 0 && args[0].isBool())
	{
		config->setUsingScrollbars(args[0].ToBoolean());
		updateWindow();
	}
}

void TiUserWindow::getTransparency(const CppArgumentList &args, CppVariant *result)
{
	result->Set(config->getTransparency());
}

void TiUserWindow::setTransparency(const CppArgumentList &args, CppVariant *result)
{
	if (args.size() > 0 && args[0].isDouble())
	{
		config->setTransparency((float)args[0].ToDouble());
		updateWindow();
	}
}

void TiUserWindow::activate(const CppArgumentList &args, CppVariant *result)
{
	this->window->showWindow(SW_RESTORE);
}

void TiUserWindow::minimize(const CppArgumentList &args, CppVariant *result)
{
	this->window->showWindow(SW_MINIMIZE);
}

void TiUserWindow::maximize(const CppArgumentList &args, CppVariant *result)
{
	this->window->showWindow(SW_MAXIMIZE);
}