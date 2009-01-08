/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef TI_OSX_PREINCLUDE
#define TI_OSX_PREINCLUDE

#if __LP64__
	#define NSInteger long
	#define NSUInteger unsigned long
#else
	#define NSInteger int
	#define NSUInteger unsigned int
#endif

#ifdef BUILDING_ON_TIGER
typedef int WebNSInteger;
typedef unsigned int WebNSUInteger;
#else
typedef NSInteger WebNSInteger;
typedef NSUInteger WebNSUInteger;
#endif

#include <titanium/titanium.h>

#endif
