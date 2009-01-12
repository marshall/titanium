/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _NETWORK_BINDING_H_
#define _NETWORK_BINDING_H_

#include <api/module.h>
#include <api/binding/binding.h>
#include <map>
#include <vector>
#include <string>

namespace ti
{
	class NetworkBinding : public StaticBoundObject
	{
	public:
		NetworkBinding(BoundObject*);
	protected:
		virtual ~NetworkBinding();
	private:
		BoundObject *global;
	};
}

#endif
