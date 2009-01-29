/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */


#ifndef _TRAY_ITEM_H_
#define _TRAY_ITEM_H_

#include <kroll/kroll.h>

namespace ti
{
	class TrayItem : public StaticBoundObject
	{

	public:
		TrayItem();
		~TrayItem();

		void SetIcon(SharedString icon_path);
		void SetMenu(SharedPtr<MenuItem> menu);
		void SetHint(SharedString hint);
		void Remove();

		void _SetIcon(const ValueList& args, SharedValue result);
		void _SetMenu(const ValueList& args, SharedValue result);
		void _SetHint(const ValueList& args, SharedValue result);
		void _Remove(const ValueList& args, SharedValue result);

	};
}

#endif
