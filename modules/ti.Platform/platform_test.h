/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef TI_PLATFORM_TEST_H_
#define TI_PLATFORM_TEST_H_
#include "platform_test.h"
#include "platform_module.h"
#include "platform_binding.h"

namespace ti
{
	class PlatformUnitTestSuite
	{
    public:
      PlatformUnitTestSuite() {};
      virtual ~PlatformUnitTestSuite() {};
      void Run(Host *host);
	};
}
#endif
