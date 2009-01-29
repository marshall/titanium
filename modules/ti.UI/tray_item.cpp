
/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include <kroll/kroll.h>
#include "ui_module.h"

namespace ti
{

	TrayItem::TrayItem() : StaticBoundObject() 
	{
		this->SetMethod("setIcon", &TrayItem::_SetIcon);
		this->SetMethod("setMenu", &TrayItem::_SetMenu);
		this->SetMethod("setHint", &TrayItem::_SetHint);
		this->SetMethod("remove", &TrayItem::_Remove);
	}

	TrayItem::~TrayItem()
	{
	}

	void TrayItem::SetIcon(SharedString icon_path)
	{
	}

	void TrayItem::SetMenu(SharedPtr<MenuItem> menu)
	{
	}

	void TrayItem::SetHint(SharedString hint)
	{

	}

	void TrayItem::Remove()
	{
	}

	void TrayItem::_SetIcon(const ValueList& args, SharedValue result)
	{
	}

	void TrayItem::_SetMenu(const ValueList& args, SharedValue result)
	{
	}

	void TrayItem::_SetHint(const ValueList& args, SharedValue result)
	{
	}

	void TrayItem::_Remove(const ValueList& args, SharedValue result)
	{
	}

}
