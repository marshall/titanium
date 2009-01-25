/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "bound_method_dispatch.h"

namespace ti
{
	BoundMethodDispatch::BoundMethodDispatch(SharedBoundMethod m): method(m)
	{
	}
	BoundMethodDispatch::~BoundMethodDispatch()
	{
		// KR_DUMP_LOCATION
	}	
	SharedValue BoundMethodDispatch::Call(ValueList args, Value *exception)
	{
		try
		{
			return method->Call(args);
		}
		catch (SharedValue &e)
		{
			exception->SetString(e->ToString());
		}
		catch (std::exception &e)
		{
			exception->SetString(e.what());
		}
		catch (std::string &e)
		{
			exception->SetString(e.c_str());
		}
		catch (const char *e)
		{
			exception->SetString(e);
		}
		catch (...)
		{
			exception->SetString("caught unknown exception");
		}
		return Value::Undefined;
	}
}
