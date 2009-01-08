/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef __WINDOWING_PLUGIN_H__
#define __WINDOWING_PLUGIN_H__

#include <kroll/kroll.h>


namespace ti {
class WindowPlugin;
class Window;
class UserWindow;
}

#include "window.h"
#include "user_window.h"

namespace ti {

class WindowPlugin : public kroll::Module
{
	KROLL_MODULE_CLASS(WindowPlugin)

protected:
	kroll::BoundObject *runtime;
};

}
#endif
