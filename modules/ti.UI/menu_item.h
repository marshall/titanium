/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */


#ifndef _MENU_ITEM_H_
#define _MENU_ITEM_H_

#include <kroll/kroll.h>

namespace ti
{
	class MenuItem : public StaticBoundList
	{

	public:
		MenuItem();
		~MenuItem();

		void SetMethod(const char *name,
		               void (MenuItem::*method)(const ValueList&, SharedValue));

		void SetParent(MenuItem*);
		MenuItem* GetParent();

		bool IsSeparator();
		bool IsSubMenu();
		bool IsItem();
		void _IsSeparator(const ValueList& args, SharedValue result);
		void _IsItem(const ValueList& args, SharedValue result);
		void _IsSubMenu(const ValueList& args, SharedValue result);


		virtual SharedValue AddSeparator() = 0;
		virtual SharedValue AddItem(SharedValue label,
		                    SharedValue callback,
		                    SharedValue icon_url) = 0;
		virtual SharedValue AddSubMenu(SharedValue label,
		                               SharedValue icon_url) = 0;
		void _AddSeparator(const ValueList& args, SharedValue result);
		void _AddItem(const ValueList& args, SharedValue result);
		void _AddSubMenu(const ValueList& args, SharedValue result);

		SharedValue AppendItem(MenuItem* item);

		void MakeSeparator();
		void MakeItem(SharedValue label,
		              SharedValue callback,
		              SharedValue icon_url);
		void MakeSubMenu(SharedValue label,
		                 SharedValue icon_url);

		void Enable();
		void Disable();
		void _Enable(const ValueList& args, SharedValue result);
		void _Disable(const ValueList& args, SharedValue result);

		const char* GetLabel();
		const char* GetIconURL();

	private:
		enum Type { SEP, ITEM, SUBMENU };

	};
}

#endif
