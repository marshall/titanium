/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef TI_DATABASE_TEST_H_
#define TI_DATABASE_TEST_H_
#include "database_test.h"
#include "database_module.h"
#include "database_binding.h"

namespace ti
{
	class DatabaseUnitTestSuite
	{
    public:
      DatabaseUnitTestSuite() {};
      virtual ~DatabaseUnitTestSuite() {};
      void Run(Host *host);
	};
}
#endif
