/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#include "file_binding.h"
#include <kroll/kroll.h>

namespace ti
{
	FileBinding::FileBinding(BoundObject *global) : global(global)
	{
		KR_ADDREF(global);
		this->SetMethod("isFile",&FileBinding::IsFile);
		this->SetMethod("isDirectory",&FileBinding::IsDirectory);
	}
	FileBinding::~FileBinding()
	{
		KR_DECREF(global);
	}
	void FileBinding::IsFile(const ValueList& args, Value *result)
	{
		std::string file = args.at(0)->ToString();
		result->Set(FileUtils::IsFile(file));
	}
	void FileBinding::IsDirectory(const ValueList& args, Value *result)
	{
		std::string dir = args.at(0)->ToString();
		result->Set(FileUtils::IsDirectory(dir));
	}
	void FileBinding::IsHidden(const ValueList& args, Value *result)
	{
		std::string file = args.at(0)->ToString();
		result->Set(FileUtils::IsHidden(file));
	}
}
