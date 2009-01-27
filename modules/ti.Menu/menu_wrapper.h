
/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _MENU_CREATOR_H_
#define _MENU_CREATOR_H_

#include "menu_item.h"

namespace ti
{
	class MenuWrapper
	{
	public:
		MenuWrapper(SharedBoundList root_item, SharedBoundObject global);
		~MenuWrapper();

	protected:
		SharedValue GetIconPath(const char *url);
		static const char* ItemGetStringProp(SharedBoundList item, const char* prop_name);
		static bool ItemIsSubMenu(SharedBoundList item);
		static bool ItemIsSeparator(SharedBoundList item);

		SharedBoundObject global;
	};
}

#endif

