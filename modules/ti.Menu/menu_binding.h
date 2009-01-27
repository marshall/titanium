/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _MENU_BINDING_H_
#define _MENU_BINDING_H_

#include <api/module.h>
#include <api/binding/binding.h>
#include "menu_item.h"

namespace ti
{
	class MenuBinding : public StaticBoundObject
	{
		friend class SharedPtr<BoundObject>;
	public:
		MenuBinding(Host *host, SharedPtr<BoundObject>);
	protected:
		virtual ~MenuBinding();
	private:
		Host *host;
		BoundObject *global;
		void CreateMenu(const ValueList& args, SharedValue result);
	};
}

#endif
