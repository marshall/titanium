/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _WINDOW_BINDING_H_
#define _WINDOW_BINDING_H_

#include <kroll/kroll.h>
#include "window_module.h"
#include "window_config.h"

#ifdef OS_WIN32
#undef CreateWindow
#endif

// this is the code that needs to be executed once the window has bound
// into its window frame the JS binding code for the top level Titanium
// objects
#define TI_WINDOW_BINDING_JS_CODE \
	GLOBAL_NS_VARNAME".Window.createWindow = function(props) { return "\
	GLOBAL_NS_VARNAME".Window._createWindow("GLOBAL_NS_VARNAME\
	".currentWindow.window,props); };"

namespace ti
{
	class WindowBinding : public StaticBoundObject
	{
	public:
		WindowBinding(Host *,SharedBoundObject);
	protected:
		virtual ~WindowBinding();
	private:
		Host *host;
		SharedBoundObject global;

		void CreateWindow(const ValueList& args, SharedValue result);
		SharedBoundObject CreateWindow(UserWindow *,SharedBoundObject properties);
	};
}

#endif
