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
		
		SharedBoundObject CreateWindow(SharedBoundObject properties);
	};
}

#endif
