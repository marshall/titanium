/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include <kroll/kroll.h>
#include "filesystem_binding.h"
#include "file.h"

#include "api/file_utils.h"

#ifdef OS_OSX
#include <Cocoa/Cocoa.h>
#elif defined(OS_WIN32)
#include <windows.h>
#include <shlobj.h>
#include <process.h>
#endif

#include <Poco/TemporaryFile.h>
#include <Poco/File.h>
#include <Poco/Path.h>
#include <Poco/LineEndingConverter.h>
#include <Poco/Exception.h>

namespace ti
{
	FilesystemBinding::FilesystemBinding(SharedBoundObject global) : global(global)
	{
		this->SetMethod("createTempFile",&FilesystemBinding::CreateTempFile);
		this->SetMethod("createTempDirectory",&FilesystemBinding::CreateTempDirectory);
		this->SetMethod("getFile",&FilesystemBinding::GetFile);
		this->SetMethod("getApplicationDirectory",&FilesystemBinding::GetApplicationDirectory);
		this->SetMethod("getResourcesDirectory",&FilesystemBinding::GetResourcesDirectory);
		this->SetMethod("getDesktopDirectory",&FilesystemBinding::GetDesktopDirectory);
		this->SetMethod("getDocumentsDirectory",&FilesystemBinding::GetDocumentsDirectory);
		this->SetMethod("getUserDirectory",&FilesystemBinding::GetUserDirectory);
		this->SetMethod("getLineEnding",&FilesystemBinding::GetLineEnding);
		this->SetMethod("getSeparator",&FilesystemBinding::GetSeparator);
		this->SetMethod("getRootDirectories",&FilesystemBinding::GetRootDirectories);
	}
	FilesystemBinding::~FilesystemBinding()
	{

	}
	void FilesystemBinding::CreateTempFile(const ValueList& args, SharedValue result)
	{
		try
		{
			Poco::TemporaryFile tempFile;
			tempFile.keepUntilExit();
			tempFile.createFile();

			ti::File* jsFile = new ti::File(tempFile.path());
			result->SetObject(jsFile);
		}
		catch (Poco::Exception& exc)
		{
			throw exc.displayText();
		}
	}
	void FilesystemBinding::CreateTempDirectory(const ValueList& args, SharedValue result)
	{
		try
		{
			Poco::TemporaryFile tempDir;
			tempDir.keepUntilExit();
			tempDir.createDirectory();

			ti::File* jsFile = new ti::File(tempDir.path());
			result->SetObject(jsFile);
		}
		catch (Poco::Exception& exc)
		{
			throw exc.displayText();
		}
	}
	void FilesystemBinding::GetFile(const ValueList& args, SharedValue result)
	{
		std::string filename = args.at(0)->ToString();

		ti::File* file = new ti::File(filename);

		result->SetObject(file);
	}
	void FilesystemBinding::GetApplicationDirectory(const ValueList& args, SharedValue result)
	{
		std::string dir = FileUtils::GetApplicationDirectory();

		result->SetString(dir.c_str());
	}
	void FilesystemBinding::GetResourcesDirectory(const ValueList& args, SharedValue result)
	{
		std::string dir = FileUtils::GetResourcesDirectory();

		result->SetString(dir.c_str());
	}
	void FilesystemBinding::GetDesktopDirectory(const ValueList& args, SharedValue result)
	{
		std::string dir;

#ifdef OS_WIN32
		char path[MAX_PATH];
		if(SHGetSpecialFolderPath(NULL,path,CSIDL_DESKTOP,FALSE))
		{
			dir.append(path);
		}
#elif OS_OSX
		NSString *fullPath = [@"~/Desktop" stringByExpandingTildeInPath];
		dir = [fullPath UTF8String];
#elif OS_LINUX
		// TODO
#endif
		if(dir.size() == 0)
		{
			result->SetNull();
		}
		else
		{
			result->SetString(dir.c_str());
		}
	}
	void FilesystemBinding::GetDocumentsDirectory(const ValueList& args, SharedValue result)
	{
		std::string dir;

#ifdef OS_WIN32
		char path[MAX_PATH];
		if(SHGetSpecialFolderPath(NULL,path,CSIDL_PERSONAL,FALSE))
		{
			dir.append(path);
		}
#elif OS_OSX
		NSString *fullPath = [@"~/Documents" stringByExpandingTildeInPath];
		dir = [fullPath UTF8String];
#elif OS_LINUX
		// TODO
#endif
		if(dir.size() == 0)
		{
			result->SetNull();
		}
		else
		{
			result->SetString(dir.c_str());
		}
	}
	void FilesystemBinding::GetUserDirectory(const ValueList& args, SharedValue result)
	{
		try
		{
			result->SetString(Poco::Path::home().c_str());
		}
		catch (Poco::Exception& exc)
		{
			throw exc.displayText();
		}
	}
	void FilesystemBinding::GetLineEnding(const ValueList& args, SharedValue result)
	{
		try
		{
			result->SetString(Poco::LineEnding::NEWLINE_LF.c_str());
		}
		catch (Poco::Exception& exc)
		{
			throw exc.displayText();
		}
	}
	void FilesystemBinding::GetSeparator(const ValueList& args, SharedValue result)
	{
		try
		{
			std::string sep;
			sep += Poco::Path::separator();
			result->SetString(sep.c_str());
		}
		catch (Poco::Exception& exc)
		{
			throw exc.displayText();
		}
	}
	void FilesystemBinding::GetRootDirectories(const ValueList& args, SharedValue result)
	{
		try
		{
			Poco::Path path;

			std::vector<std::string> roots;
			path.listRoots(roots);

			SharedPtr<StaticBoundList> rootList = new StaticBoundList();

			for(size_t i = 0; i < roots.size(); i++)
			{
				SharedValue value = Value::NewString(roots.at(i));
				rootList->Append(value);
			}

			SharedPtr<BoundList> list = rootList;
			result->SetList(list);
		}
		catch (Poco::Exception& exc)
		{
			throw exc.displayText();
		}
	}
}
