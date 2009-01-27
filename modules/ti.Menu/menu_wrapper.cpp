/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "menu_wrapper.h"

namespace ti
{
	MenuWrapper::MenuWrapper(SharedBoundList root_item, SharedBoundObject global) : global(global)
	{
	}
	MenuWrapper::~MenuWrapper()
	{
	}
	bool MenuWrapper::ItemIsSubMenu(SharedBoundList item)
	{
		SharedBoundMethod submenu_test = item->Get("isSubMenu")->ToMethod();
		if (submenu_test.isNull())
			return false;

		ValueList args;
		SharedValue result = submenu_test->Call(args);
		return result->IsBool() && result->ToBool();
	}

	bool MenuWrapper::ItemIsSeparator(SharedBoundList item)
	{
		SharedBoundMethod sep_test = item->Get("isSeparator")->ToMethod();
		if (sep_test.isNull())
			return false;

		ValueList args;
		SharedValue result = sep_test->Call(args);
		return result->IsBool() && result->ToBool();
	}

	const char* MenuWrapper::ItemGetStringProp(SharedBoundList item, const char* prop_name)
	{
		SharedValue prop = item->Get(prop_name);
		if (prop->IsString())
			return prop->ToString();
		else
			return "";
	}

	SharedValue MenuWrapper::GetIconPath(const char *url)
	{
		if (!strcmp(url, ""))
		{
			return Value::Undefined;
		}

		SharedValue app_obj = this->global->Get("App");
		if (!app_obj->IsObject())
		{
			return Value::Undefined;
		}

		SharedValue meth_val = app_obj->ToObject()->Get("appURLToPath");
		if (!meth_val->IsMethod())
		{
			return Value::Undefined;
		}

		SharedBoundMethod meth = meth_val->ToMethod();
		ValueList args;
		args.push_back(Value::NewString(url));
		return meth->Call(args);
	}
}