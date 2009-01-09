/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef TI_APP_MODULE_H_
#define TI_APP_MODULE_H_

#include <kroll/kroll.h>

namespace ti {

class AppModule :
	public kroll::Module
{
	KROLL_MODULE_CLASS(AppModule)

public:

	/*
	virtual kroll::Value* Get(const char *name);
	virtual void Set(const char *name, kroll::Value *value);
	kroll::Value* Call(const char *name, const kroll::ValueList &args);
	virtual std::vector<std::string> GetPropertyNames ();
	*/
};

}
#endif
