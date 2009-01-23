/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _WINDOW_MODULE_H_
#define _WINDOW_MODULE_H_

#include <kroll/kroll.h>

#ifdef OS_WIN32
// A little disorganization; header include order is very sensitive in win32,
// and the build breaks if this below the other OS_ defines
#include "win32/win32_user_window.h"

#endif

#include <iostream>
#include "window_config.h"
#include "user_window.h"
#include "window.h"

#ifdef OS_LINUX
#include "linux/window_module_linux.h"
#include "url/app_url.h"
#endif

#ifdef OS_OSX
#include <WebKit/WebKit.h>
#include "osx/window_module_osx.h"
#include "osx/native_window.h"
#include "osx/osx_user_window.h"
#include "osx/ti_app.h"
#endif

namespace ti {

class WindowModule : public kroll::Module
{
	KROLL_MODULE_CLASS(WindowModule)

protected:
	
	DISALLOW_EVIL_CONSTRUCTORS(WindowModule);
};

}
#endif
