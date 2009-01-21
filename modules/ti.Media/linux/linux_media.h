/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _MEDIA_LINUX_H_
#define _MEDIA_LINUX_H_

#include <api/module.h>
#include <api/binding/binding.h>

using namespace kroll;

namespace ti
{
	class LinuxMedia
	{
	public:
		static void Beep();
	private:
		LinuxMedia();
		~LinuxMedia();
	};
}

#endif
