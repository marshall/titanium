/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef TI_MONKEY_TEST_H_
#define TI_MONKEY_TEST_H_
#include "monkey_test.h"
#include "monkey_module.h"
#include "monkey_binding.h"

namespace ti
{
	class MonkeyUnitTestSuite
	{
    public:
      MonkeyUnitTestSuite() {};
      virtual ~MonkeyUnitTestSuite() {};
      void Run(Host *host);
	};
}
#endif
