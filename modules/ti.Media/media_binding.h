/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _MEDIA_BINDING_H_
#define _MEDIA_BINDING_H_

#include <api/module.h>
#include <api/binding/binding.h>
#include <map>
#include <vector>
#include <string>

namespace ti
{
	class MediaBinding : public StaticBoundObject
	{
	public:
		MediaBinding(SharedBoundObject);
	protected:
		virtual ~MediaBinding();
	private:
		SharedBoundObject global;
		void CreateSound(const ValueList& args, SharedValue result);
		void Beep(const ValueList& args, SharedValue result);
	};
}

#endif
