/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _NETWORK_BINDING_H_
#define _NETWORK_BINDING_H_

#include <kroll/kroll.h>

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
		void Create(const ValueList& args, Value *result);
	};
}

#endif
