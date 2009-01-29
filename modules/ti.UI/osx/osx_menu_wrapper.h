/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _OSX_MENU_CREATOR_H_
#define _OSX_MENU_CREATOR_H_

#include <Cocoa/Cocoa.h>
#include <vector>
#include "../menu_item.h"
#include "../menu_wrapper.h"
#include "menu_action.h"

namespace ti
{
	class OSXMenuWrapper : public MenuWrapper
	{
	public:
		OSXMenuWrapper(SharedBoundList root_item, SharedBoundObject global);
		~OSXMenuWrapper();
		
		NSMenu* getNSMenu();
		
	private:
		NSMenu *menu;
		std::vector<MenuAction*> actions;

		NSMenuItem* ItemFromValue(SharedBoundList menu_item);
		void AddChildrenToMenu(SharedBoundList menu, NSMenu *osxmenu);
	};
}

#endif

