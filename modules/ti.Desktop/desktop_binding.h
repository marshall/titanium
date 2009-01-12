/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _DESKTOP_BINDING_H_
#define _DESKTOP_BINDING_H_

#include <api/module.h>
#include <api/binding/binding.h>
#include <map>
#include <vector>
#include <string>

namespace ti
{
	class DesktopBinding : public StaticBoundObject
	{
	public:
		DesktopBinding(BoundObject*);
	protected:
		virtual ~DesktopBinding();
	private:
		BoundObject *global;
		void CreateShortcut(const ValueList& args, Value *result);
		void OpenFiles(const ValueList& args, Value *result);
	};
}

#endif
