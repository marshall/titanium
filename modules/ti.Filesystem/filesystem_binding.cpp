/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include <kroll/kroll.h>
#include "filesystem_binding.h"
#include "file.h"

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
	FilesystemBinding::FilesystemBinding(BoundObject *global) : global(global)
	{
		this->SetMethod("createTempFile",&FilesystemBinding::CreateTempFile);
		this->SetMethod("createTempDirectory",&FilesystemBinding::CreateTempDirectory);
		this->SetMethod("getFile",&FilesystemBinding::GetFile);
		this->SetMethod("getApplicationDirectory",&FilesystemBinding::GetApplicationDirectory);
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
			std::cerr << "Problem creating temp file:::: " << exc.displayText() << std::endl;

			// TODO test this
			SharedValue v = Value::NewString(exc.displayText());
			throw v;
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
			std::cerr << "Problem creating temp directory:::: " << exc.displayText() << std::endl;
			result->SetNull();
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
		std::cout << "GetApplicationDirectory() called" << std::endl;
	}
	void FilesystemBinding::GetDesktopDirectory(const ValueList& args, SharedValue result)
	{
		std::cout << "GetDesktopDirectory() called" << std::endl;

		std::string dir;

#ifdef OS_WIN32
		char path[MAX_PATH];
		if(SHGetSpecialFolderPath(NULL,path,CSIDL_DESKTOP,FALSE))
		{
			dir.append(path);
		}
#elif OS_OSX
		// TODO
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
		std::cout << "GetDocumentsDirectory() called" << std::endl;
		std::string dir;

#ifdef OS_WIN32
		char path[MAX_PATH];
		if(SHGetSpecialFolderPath(NULL,path,CSIDL_PERSONAL,FALSE))
		{
			dir.append(path);
		}
#elif OS_OSX
		// TODO
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
			std::cerr << "Problem getting home dir:::: " << exc.displayText() << std::endl;

			// TODO test this
			//SharedValue v = Value::NewString(exc.displayText());
			//throw v;
		}
	}
	void FilesystemBinding::GetLineEnding(const ValueList& args, SharedValue result)
	{
		std::cout << "GetLineEnding() called" << std::endl;

		try
		{
			result->SetString(Poco::LineEnding::NEWLINE_LF.c_str());
		}
		catch (Poco::Exception& exc)
		{
			std::cerr << "Problem getting line ending:::: " << exc.displayText() << std::endl;

			// TODO test this
			//SharedValue v = Value::NewString(exc.displayText());
			//throw v;
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
			std::cerr << "Problem getting separator:::: " << exc.displayText() << std::endl;

			// TODO test this
			//SharedValue v = Value::NewString(exc.displayText());
			//throw v;
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
			std::cerr << "Problem getting root directories:::: " << exc.displayText() << std::endl;

			// TODO test this
			//SharedValue v = Value::NewString(exc.displayText());
			///throw v;
		}
	}
}
