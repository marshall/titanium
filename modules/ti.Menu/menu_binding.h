/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _MENU_BINDING_H_
#define _MENU_BINDING_H_

#include <api/module.h>
#include <api/binding/binding.h>

namespace ti
{
	class MenuBinding : public StaticBoundObject
	{
	public:
		MenuBinding(BoundObject*);
	protected:
		virtual ~MenuBinding();
	private:
		BoundObject *global;
	};
}

#endif
