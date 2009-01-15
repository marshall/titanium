/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef TI_GROWL_TEST_H_
#define TI_GROWL_TEST_H_
#include "growl_test.h"
#include "growl_module.h"
#include "growl_binding.h"

namespace ti
{
	class GrowlUnitTestSuite
	{
    public:
      GrowlUnitTestSuite() {};
      virtual ~GrowlUnitTestSuite() {};
      void Run(Host *host);
	};
}
#endif
