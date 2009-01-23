/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _DATABASE_BINDING_H_
#define _DATABASE_BINDING_H_

#include <api/module.h>
#include <api/binding/binding.h>

namespace ti
{
	class DatabaseBinding : public StaticBoundObject
	{
	public:
		DatabaseBinding(SharedBoundObject);
	protected:
		virtual ~DatabaseBinding();
	private:
		SharedBoundObject global;
	};
}

#endif
