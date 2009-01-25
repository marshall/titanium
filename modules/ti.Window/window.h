/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _WINDOW_H_
#define _WINDOW_H_

#include <kroll/kroll.h>

namespace ti {

class Window : public kroll::StaticBoundObject {
	public:
		Window(SharedBoundObject window);
	protected:
		~Window() {}
	private:
		DISALLOW_EVIL_CONSTRUCTORS(Window);
};

}
#endif

