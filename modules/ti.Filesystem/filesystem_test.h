/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef TI_FILESYSTEM_TEST_H_
#define TI_FILESYSTEM_TEST_H_
#include "filesystem_test.h"
#include "filesystem_module.h"
#include "filesystem_binding.h"

namespace ti
{
	class FilesystemUnitTestSuite
	{
    public:
      FilesystemUnitTestSuite() {};
      virtual ~FilesystemUnitTestSuite() {};
      void Run(Host *host);
	};
}
#endif
