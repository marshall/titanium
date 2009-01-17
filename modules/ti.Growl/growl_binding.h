/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _GROWL_BINDING_H_
#define _GROWL_BINDING_H_

#include <api/module.h>
#include <api/binding/binding.h>

namespace ti
{
	class GrowlBinding : public StaticBoundObject
	{
	public:
		GrowlBinding(SharedPtr<BoundObject>);
	protected:
		virtual ~GrowlBinding();
	private:
		SharedPtr<BoundObject> global;

		void ShowNotification(const ValueList& args, SharedPtr<Value> result);
	};
}

#endif
