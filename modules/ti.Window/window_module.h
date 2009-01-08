/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _WINDOW_MODULE_H_
#define _WINDOW_MODULE_H_

#include <kroll/kroll.h>


namespace ti {
class WindowModule;
class Window;
class UserWindow;
}

#include "window.h"
#include "user_window.h"

namespace ti {

class WindowModule : public kroll::Module
{
	KROLL_MODULE_CLASS(WindowModule)

protected:
	kroll::BoundObject *runtime;
};

}
#endif
