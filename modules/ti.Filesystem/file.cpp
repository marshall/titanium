/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "file.h"

#include <Poco/File.h>
#include <Poco/Path.h>
#include <Poco/FileStream.h>
#include <Poco/Exception.h>

namespace ti
{
	File::File(std::string filename)
	{
#ifdef OS_OSX
		// in OSX, we need to expand ~ in paths to their absolute path value
		// we do that with a nifty helper method in NSString
		this->filename = [[[NSString stringWithCString:filename.c_str()] stringByExpandingTildeInPath] UTF8String];
#else
		this->filename = filename;
#endif

		this->SetMethod("toString",&File::ToString);
		this->SetMethod("isFile",&File::IsFile);
		this->SetMethod("isDirectory",&File::IsDirectory);
		this->SetMethod("isHidden",&File::IsHidden);
		this->SetMethod("isSymbolicLink",&File::IsSymbolicLink);

		this->SetMethod("resolve",&File::Resolve);
		this->SetMethod("write",&File::Write);
		this->SetMethod("read",&File::Read);
		this->SetMethod("readLine",&File::ReadLine);
		this->SetMethod("copy",&File::Copy);
		this->SetMethod("move",&File::Move);
		this->SetMethod("createDirectory",&File::CreateDirectoryX);
		this->SetMethod("deleteDirectory",&File::DeleteDirectory);
		this->SetMethod("deleteFile",&File::DeleteFileX);
		this->SetMethod("getDirectoryListing",&File::GetDirectoryListing);

		this->SetMethod("parent",&File::GetParent);
		this->SetMethod("exists",&File::GetExists);
		this->SetMethod("createTimestamp",&File::GetCreateTimestamp);
		this->SetMethod("modificationTimestamp",&File::GetModificationTimestamp);
		this->SetMethod("name",&File::GetName);
		this->SetMethod("extension",&File::GetExtension);
		this->SetMethod("nativePath",&File::GetNativePath);
		this->SetMethod("size",&File::GetSize);
		this->SetMethod("spaceAvailable",&File::GetSpaceAvailable);

		this->readLineFS = NULL;
	}

	File::~File()
	{
		if (this->readLineFS)
		{
			delete this->readLineFS;
		}
	}

	void File::ToString(const ValueList& args, SharedValue result)
	{
		result->SetString(this->filename.c_str());
	}
	void File::IsFile(const ValueList& args, SharedValue result)
	{
		try
		{
			Poco::File file(this->filename);
			bool isFile = file.isFile();
			result->SetBool(isFile);
		}
		catch (Poco::Exception& exc)
		{
			throw exc.displayText();
		}
	}
	void File::IsDirectory(const ValueList& args, SharedValue result)
	{
		try
		{
			Poco::File dir(this->filename);
			bool isDir = dir.isDirectory();
			result->SetBool(isDir);
		}
		catch (Poco::Exception& exc)
		{
			throw exc.displayText();
		}
	}
	void File::IsHidden(const ValueList& args, SharedValue result)
	{
		try
		{
			Poco::File file(this->filename);
			bool isHidden = file.isHidden();
			result->SetBool(isHidden);
		}
		catch (Poco::Exception& exc)
		{
			throw exc.displayText();
		}
	}
	void File::IsSymbolicLink(const ValueList& args, SharedValue result)
	{
		try
		{
			Poco::File file(this->filename);
			bool isLink = file.isLink();
			result->SetBool(isLink);
		}
		catch (Poco::Exception& exc)
		{
			throw exc.displayText();
		}
	}
	void File::Resolve(const ValueList& args, SharedValue result)
	{
		try
		{
			std::string pathToResolve = args.at(0)->ToString();

			Poco::Path path(this->filename);
			path.resolve(pathToResolve);

			ti::File* file = new ti::File(path.toString());
			result->SetObject(file);
		}
		catch (Poco::Exception& exc)
		{
			throw exc.displayText();
		}
	}
	void File::Write(const ValueList& args, SharedValue result)
	{
		try
		{
			std::string text = args.at(0)->ToString();
			bool append = false;

			if(args.size() > 1)
			{
				append = args.at(1)->ToBool();
			}

			std::ios_base::openmode mode;
			if(append)
			{
				mode = std::ios_base::out | std::ios_base::app;
			}
			else
			{
				mode = std::ios_base::out | std::ios_base::trunc;
			}

			Poco::FileOutputStream fos(this->filename, mode);
			fos.write(text.c_str(), text.size());
			fos.close();

			result->SetBool(true);
		}
		catch (Poco::Exception& exc)
		{
			throw exc.displayText();
		}
	}
	void File::Read(const ValueList& args, SharedValue result)
	{
		try
		{
			Poco::FileInputStream fis(this->filename);

			std::string contents;

			while(! fis.eof())
			{
				std::string s;
				std::getline(fis, s);

				contents.append(s);
			}

			result->SetString(contents.c_str());
		}
		catch (Poco::Exception& exc)
		{
			throw exc.displayText();
		}
	}
	void File::ReadLine(const ValueList& args, SharedValue result)
	{
		try
		{
			bool openFile = false;
			if(args.size() > 0)
			{
				openFile = args.at(0)->ToBool();
			}

			if(openFile)
			{
				// close file if it's already open
				if(this->readLineFS)
				{
					this->readLineFS->close();
				}

				// now open the file
				this->readLineFS = new Poco::FileInputStream(this->filename);
			}

			if(this->readLineFS == NULL)
			{
				result->SetNull();
			}
			else
			{
				std::string line;

				if(this->readLineFS->eof())
				{
					// close the file
					this->readLineFS->close();
					this->readLineFS = NULL;
					result->SetNull();
				}
				else {
					std::string line;
					std::getline(*(this->readLineFS), line);
					result->SetString(line.c_str());
				}
			}
		}
		catch (Poco::Exception& exc)
		{
			throw exc.displayText();
		}
	}
	void File::Copy(const ValueList& args, SharedValue result)
	{
		try
		{
			std::string dest = args.at(0)->ToString();
			// TODO need to verify parameters

			Poco::File from(this->filename);
			from.copyTo(dest);
			result->SetBool(true);
		}
		catch (Poco::Exception& exc)
		{
			throw exc.displayText();
		}


	}
	void File::Move(const ValueList& args, SharedValue result)
	{
		try
		{
			std::string dest = args.at(0)->ToString();
			// TODO need to verify parameters

			Poco::File from(this->filename);
			from.moveTo(dest);
			result->SetBool(true);
		}
		catch (Poco::Exception& exc)
		{
			throw exc.displayText();
		}
	}
	void File::CreateDirectoryX(const ValueList& args, SharedValue result)
	{
		try
		{
			bool createParents = false;
			if(args.size() > 0)
			{
				createParents = args.at(0)->ToBool();
			}

			Poco::File dir(this->filename);
			bool created = false;
			if(! dir.exists())
			{
				if(createParents)
				{
					dir.createDirectories();
					created = true;
				}
				else {
					created = dir.createDirectory();
				}
			}
			result->SetBool(created);
		}
		catch (Poco::Exception& exc)
		{
			throw exc.displayText();
		}
	}
	void File::DeleteDirectory(const ValueList& args, SharedValue result)
	{
		try
		{
			bool deleteContents = false;
			if(args.size() > 0)
			{
				deleteContents = args.at(0)->ToBool();
			}

			Poco::File dir(this->filename);
			bool deleted = false;
			if(dir.exists() && dir.isDirectory())
			{
				dir.remove(deleteContents);

				deleted = true;
			}
			result->SetBool(deleted);
		}
		catch (Poco::Exception& exc)
		{
			throw exc.displayText();
		}
	}
	void File::DeleteFileX(const ValueList& args, SharedValue result)
	{
		try
		{
			Poco::File file(this->filename);
			bool deleted = false;
			if(file.exists() && file.isFile())
			{
				file.remove();

				deleted = true;
			}
			result->SetBool(deleted);
		}
		catch (Poco::Exception& exc)
		{
			throw exc.displayText();
		}
	}
	void File::GetDirectoryListing(const ValueList& args, SharedValue result)
	{
		try
		{
			Poco::File dir(this->filename);

			if(dir.exists() && dir.isDirectory())
			{
				std::vector<std::string> files;
				dir.list(files);

				SharedPtr<StaticBoundList> fileList = new StaticBoundList();

				for(size_t i = 0; i < files.size(); i++)
				{
					ti::File* file = new ti::File(files.at(i));
					SharedValue value = Value::NewObject((SharedBoundObject) file);
					fileList->Append(value);
				}

				SharedPtr<BoundList> list = fileList;
				result->SetList(list);
			}
			else
			{
				result->SetNull();
			}
		}
		catch (Poco::Exception& exc)
		{
			throw exc.displayText();
		}
	}
	void File::GetParent(const ValueList& args, SharedValue result)
	{
		try
		{
			Poco::Path path(this->filename);

			ti::File* file = new ti::File(path.parent().toString());
			result->SetObject(file);
		}
		catch (Poco::Exception& exc)
		{
			throw exc.displayText();
		}
	}
	void File::GetExists(const ValueList& args, SharedValue result)
	{
		try
		{
			Poco::File file(this->filename);
			bool exists = file.exists();
			result->SetBool(exists);
		}
		catch (Poco::Exception& exc)
		{
			throw exc.displayText();
		}
	}
	void File::GetCreateTimestamp(const ValueList& args, SharedValue result)
	{
		try
		{
			Poco::File file(this->filename);
			Poco::Timestamp ts = file.created();

			result->SetDouble(ts.epochTime());
		}
		catch (Poco::Exception& exc)
		{
			throw exc.displayText();
		}
	}
	void File::GetModificationTimestamp(const ValueList& args, SharedValue result)
	{
		try
		{
			Poco::File file(this->filename);
			Poco::Timestamp ts = file.getLastModified();

			result->SetDouble(ts.epochTime());
		}
		catch (Poco::Exception& exc)
		{
			throw exc.displayText();
		}
	}
	void File::GetName(const ValueList& args, SharedValue result)
	{
		result->SetString(this->filename.c_str());
	}
	void File::GetExtension(const ValueList& args, SharedValue result)
	{
		try
		{
			Poco::Path path(this->filename);

			if(path.isDirectory())
			{
				result->SetNull();
			}
			else
			{
				result->SetString(path.getExtension().c_str());
			}
		}
		catch (Poco::Exception& exc)
		{
			throw exc.displayText();
		}
	}
	void File::GetNativePath(const ValueList& args, SharedValue result)
	{
		try
		{
			Poco::Path path(this->filename);

			result->SetString(path.makeAbsolute().toString().c_str());
		}
		catch (Poco::Exception& exc)
		{
			throw exc.displayText();
		}
	}
	void File::GetSize(const ValueList& args, SharedValue result)
	{
		try
		{
			Poco::File file(this->filename);

			result->SetDouble(file.getSize());
		}
		catch (Poco::Exception& exc)
		{
			throw exc.displayText();
		}
	}
	void File::GetSpaceAvailable(const ValueList& args, SharedValue result)
	{
		long avail = -1;
		Poco::Path path(this->filename);

#ifdef OS_OSX
		NSString *p = [NSString stringWithCString:this->filename.c_str()];
		avail = [[[[NSFileManager defaultManager] fileSystemAttributesAtPath:p] objectForKey:NSFileSystemFreeSize] longValue];
#elif OS_WIN32
		PULARGE_INTEGER freeBytesAvail = 0;
		PULARGE_INTEGER totalNumOfBytes = 0;
		PULARGE_INTEGER totalNumOfFreeBytes = 0;
		if(GetDiskFreeSpaceEx(path.absolute().getFileName().c_str(), freeBytesAvail, totalNumOfBytes, totalNumOfFreeBytes))
		{
			avail = long(freeBytesAvail);
		}
#elif OS_LINUX
		// TODO complete and test this
#endif

		if(avail == -1)
		{
			result->SetNull();
		}
		else
		{
			result->SetDouble(avail);
		}
	}
}
