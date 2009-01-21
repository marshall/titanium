/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _MENU_H_
#define _MENU_H_

#include "menu_module.h"

namespace ti
{
	class Menu : public StaticBoundObject
	{
	public:
		Menu();
      	virtual ~Menu();

		
		// bound object API

		void SetTitle(const ValueList& args, SharedValue result);
		void GetTitle(const ValueList& args, SharedValue result);


		// main API
		virtual void SetTitle(std::string &title) = 0;
		virtual std::string GetTitle() = 0;
	};
}
#endif
