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

using namespace kroll;

namespace ti
{
	class DesktopBinding : public StaticBoundObject
	{
	public:
		DesktopBinding(SharedPtr<BoundObject>);
	protected:
		virtual ~DesktopBinding();
	private:
		SharedPtr<BoundObject> global;
		void CreateShortcut(const ValueList& args, SharedPtr<Value> result);
		void OpenFiles(const ValueList& args, SharedPtr<Value> result);
		void OpenApplication(const ValueList& args, SharedPtr<Value> result);
		void OpenURL(const ValueList& args, SharedPtr<Value> result);
		void GetSystemIdleTime(const ValueList& args, SharedPtr<Value> result);
	};
}

#endif
