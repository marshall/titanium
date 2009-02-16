/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _MEDIA_OSX_H_
#define _MEDIA_OSX_H_

#include <api/binding/binding.h>
#import <Cocoa/Cocoa.h>

using namespace kroll;

namespace ti
{
	class OSXMedia
	{
	public:
		static void Beep();
	private:
		OSXMedia();
		~OSXMedia();
	};
}

#endif
