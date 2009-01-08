/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef __WINDOWING_PLUGIN_H__
#define __WINDOWING_PLUGIN_H__

#include <titanium/titanium.h>

class WindowPlugin;
class TiWindow;
class TiUserWindow;
#include "ti_window.h"
#include "ti_user_window.h"

class WindowingPlugin : public TiPlugin
{
	TIPLUGIN_CLASS(WindowingPlugin)

protected:
	TiBoundObject *runtime;
};

#endif
