/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef TI_PROCESS_TEST_H_
#define TI_PROCESS_TEST_H_
#include "process_test.h"
#include "process_module.h"
#include "process_binding.h"

namespace ti
{
	class ProcessUnitTestSuite
	{
    public:
      ProcessUnitTestSuite() {};
      virtual ~ProcessUnitTestSuite() {};
      void Run(Host *host);
	};
}
#endif
