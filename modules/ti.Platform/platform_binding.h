/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _PLATFORM_BINDING_H_
#define _PLATFORM_BINDING_H_

#include <api/module.h>
#include <api/binding/binding.h>

namespace ti
{
	class PlatformBinding : public StaticBoundObject
	{
	public:
		PlatformBinding(SharedBoundObject);
	protected:
		virtual ~PlatformBinding();
	private:
		SharedBoundObject global;
		
		DECLAREBOUNDMETHOD(CreateUUID);
	};
}

#endif
