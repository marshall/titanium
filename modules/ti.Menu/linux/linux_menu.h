/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _LINUX_MENU_H_
#define _LINUX_MENU_H_

#include "../menu.h"

namespace ti
{
	class LinuxMenu : public Menu
	{
	public:
		LinuxMenu();
      	virtual ~LinuxMenu();
		
		// main API
		virtual void SetTitle(std::string &title);
		virtual std::string GetTitle();
	};
}
#endif
