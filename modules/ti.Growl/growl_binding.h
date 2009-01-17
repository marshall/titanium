/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _GROWL_BINDING_H_
#define _GROWL_BINDING_H_

#include <api/module.h>
#include <api/binding/binding.h>

using namespace kroll;

namespace ti
{
	class GrowlBinding : public StaticBoundObject
	{
		friend class SharedPtr;

	public:
		GrowlBinding(SharedBoundObject);
	protected:
		virtual ~GrowlBinding();
	private:
		kroll::SharedBoundObject global;

		void ShowNotification(const ValueList& args, SharedValue result);
	};
}

#endif
