/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef TI_MENU_TEST_H_
#define TI_MENU_TEST_H_
#include "menu_test.h"
#include "menu_module.h"
#include "menu_binding.h"

namespace ti
{
	class MenuUnitTestSuite
	{
    public:
      MenuUnitTestSuite() {};
      virtual ~MenuUnitTestSuite() {};
      void Run(Host *host);
	};
}
#endif
