/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _APP_BINDING_H_
#define _APP_BINDING_H_

#include <api/module.h>
#include <api/binding/binding.h>


namespace ti
{
	class AppBinding : public StaticBoundObject
	{
	public:
		AppBinding(BoundObject*);
	protected:
		virtual ~AppBinding();
	private:
		BoundObject *global;
		void GetID(const ValueList& args, Value *result);
		void GetName(const ValueList& args, Value *result);
		void GetVersion(const ValueList& args, Value *result);
		void GetUpdateURL(const ValueList& args, Value *result);
		void GetGUID(const ValueList& args, Value *result);
	};
}

#endif
