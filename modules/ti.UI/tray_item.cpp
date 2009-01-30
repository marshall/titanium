
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

	void TrayItem::_SetIcon(const ValueList& args, SharedValue result)
	{
		// Cannot set a NULL icon
		if (args.size() > 0 && args.at(0)->IsString())
		{
			const char *icon_url = args.at(0)->ToString();
			SharedString icon_path = UIModule::GetResourcePath(icon_url);
			if (!icon_path.isNull())
			{
				this->SetIcon(icon_path);
			}
		}
	}

	void TrayItem::_SetMenu(const ValueList& args, SharedValue result)
	{
		SharedPtr<BoundList> menu = NULL; // A NULL value is an unset
		if (args.size() > 0 && args.at(0)->IsList())
		{
			menu = args.at(0)->ToList().cast<BoundList>();
		}
		this->SetMenu(menu);
	}

	void TrayItem::_SetHint(const ValueList& args, SharedValue result)
	{
		SharedString hint = NULL; // A NULL value is an unset
		if (args.size() > 0 && args.at(0)->IsString())
		{
			hint = new std::string(args.at(0)->ToString());
		}
		this->SetHint(hint);
	}

	void TrayItem::_Remove(const ValueList& args, SharedValue result)
	{
		this->Remove();
	}

}
