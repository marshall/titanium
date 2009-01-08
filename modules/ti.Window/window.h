/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef __TI_WINDOW_H__
#define __TI_WINDOW_H__

#include <kroll/kroll.h>

namespace ti {

class Window : public kroll::StaticBoundObject {
	public:
		Window(UserWindow* window);
};

}
#endif

