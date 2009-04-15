/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "filesystem_utils.h"

#include <Poco/File.h>
#include <Poco/Path.h>
#include <Poco/FileStream.h>
#include <Poco/Exception.h>

namespace ti
{
	FileSystemUtils::FileSystemUtils() { }
	FileSystemUtils::~FileSystemUtils() { }
	
	
	const char* FileSystemUtils::GetFileName(SharedValue v)
	{
		if (v->IsString())
		{
			return v->ToString();
		}
		else if (v->IsObject())
		{
			SharedKObject bo = v->ToObject();
			SharedPtr<File> file = bo.cast<File>();
			if (file.isNull())
			{
				throw ValueException::FromString("invalid type");
			}
			return file->GetFilename().c_str();
		}
		else
		{
			throw ValueException::FromString("invalid type");
		}
	}
}
