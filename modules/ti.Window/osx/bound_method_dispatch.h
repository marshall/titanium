/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "preinclude.h"

namespace ti
{
	class BoundMethodDispatch
	{
	public:
		BoundMethodDispatch(SharedBoundMethod);
		virtual ~BoundMethodDispatch();
		
		SharedValue Call(ValueList args, Value *exception);
		
	private:
		SharedBoundMethod method;
	};
}