/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "file.h"

#include "Poco/File.h"
#include <Poco/Exception.h>

namespace ti
{
	File::File(BoundObject *global, std::string filename) : global(global), filename(filename)
	{
		KR_ADDREF(global);

		this->SetMethod("isFile",&File::IsFile);
		this->SetMethod("isDirectory",&File::IsDirectory);
		this->SetMethod("isHidden",&File::IsHidden);
		this->SetMethod("toString",&File::ToString);
	}

	File::~File()
	{
		KR_DECREF(global);
	}

	void File::ToString(const ValueList& args, Value *result)
	{
		result->Set(this->filename);
	}
	void File::IsFile(const ValueList& args, Value *result)
	{
		bool isFile = false;

		try
		{
			Poco::File file(this->filename);
			isFile = file.exists() && file.isFile();
		}
		catch (Poco::Exception& exc)
		{
			std::cerr << "Problem getting file info:::: " << exc.displayText() << std::endl;
		}

		result->Set(isFile);
	}
	void File::IsDirectory(const ValueList& args, Value *result)
	{
		bool isDir = false;

		try
		{
			Poco::File dir(this->filename);
			isDir = dir.exists() && dir.isDirectory();
		}
		catch (Poco::Exception& exc)
		{
			std::cerr << "Problem getting file info:::: " << exc.displayText() << std::endl;
		}

		result->Set(isDir);
	}
	void File::IsHidden(const ValueList& args, Value *result)
	{
		bool isHidden = false;

		try
		{
			Poco::File file(this->filename);
			isHidden = file.exists() && file.isHidden();
		}
		catch (Poco::Exception& exc)
		{
			std::cerr << "Problem getting file info:::: " << exc.displayText() << std::endl;
		}

		result->Set(isHidden);
	}
}
