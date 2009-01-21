/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _OSX_MENU_H_
#define _OSX_MENU_H_

#import <Cocoa/Cocoa.h>
#include "../menu.h"

namespace ti
{
	class OSXMenu : public Menu
	{
	public:
		OSXMenu(NSMenu *menu);
      	virtual ~OSXMenu();
		
		// main API
		virtual void SetTitle(std::string &title);
		virtual std::string GetTitle();
	private:
		NSMenu *menu;
	};
}
#endif
