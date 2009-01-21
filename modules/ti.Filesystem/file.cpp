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
		bool isFile = false;

		try
		{
			Poco::File file(this->filename);
			isFile = file.isFile();
		}
		catch (Poco::Exception& exc)
		{
			throw exc.displayText().c_str();
		}

		result->SetBool(isFile);
	}
	void File::IsDirectory(const ValueList& args, SharedValue result)
	{
		bool isDir = false;

		try
		{
			Poco::File dir(this->filename);
			isDir = dir.isDirectory();
		}
		catch (Poco::Exception& exc)
		{
			throw exc.displayText().c_str();
		}

		result->SetBool(isDir);
	}
	void File::IsHidden(const ValueList& args, SharedValue result)
	{
		bool isHidden = false;

		try
		{
			Poco::File file(this->filename);
			isHidden = file.isHidden();
		}
		catch (Poco::Exception& exc)
		{
			throw exc.displayText().c_str();
		}

		result->SetBool(isHidden);
	}
	void File::IsSymbolicLink(const ValueList& args, SharedValue result)
	{
		bool isLink = false;

		try
		{
			Poco::File file(this->filename);
			isLink = file.isLink();
		}
		catch (Poco::Exception& exc)
		{
			throw exc.displayText().c_str();
		}

		result->SetBool(isLink);
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
			throw exc.displayText().c_str();
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
			throw exc.displayText().c_str();
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
			throw exc.displayText().c_str();
		}
	}
	void File::Copy(const ValueList& args, SharedValue result)
	{
		bool success = false;

		try
		{
			std::string dest = args.at(0)->ToString();
			// TODO need to verify parameters

			Poco::File from(this->filename);

			from.copyTo(dest);

			success = true;
		}
		catch (Poco::Exception& exc)
		{
			throw exc.displayText().c_str();
		}

		result->SetBool(success);
	}
	void File::Move(const ValueList& args, SharedValue result)
	{
		bool success = false;

		try
		{
			std::string dest = args.at(0)->ToString();
			// TODO need to verify parameters

			Poco::File from(this->filename);

			from.moveTo(dest);

			success = true;
		}
		catch (Poco::Exception& exc)
		{
			throw exc.displayText().c_str();
		}

		result->SetBool(success);
	}
	void File::CreateDirectoryX(const ValueList& args, SharedValue result)
	{
		bool created = false;

		try
		{
			bool createParents = false;
			if(args.size() > 0)
			{
				createParents = args.at(0)->ToBool();
			}

			Poco::File dir(this->filename);

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
		}
		catch (Poco::Exception& exc)
		{
			throw exc.displayText().c_str();
		}

		result->SetBool(created);
	}
	void File::DeleteDirectory(const ValueList& args, SharedValue result)
	{
		bool deleted = false;

		try
		{
			bool deleteContents = false;
			if(args.size() > 0)
			{
				deleteContents = args.at(0)->ToBool();
			}

			Poco::File dir(this->filename);

			if(dir.exists() && dir.isDirectory())
			{
				dir.remove(deleteContents);

				deleted = true;
			}
		}
		catch (Poco::Exception& exc)
		{
			throw exc.displayText().c_str();
		}

		result->SetBool(deleted);
	}
	void File::DeleteFileX(const ValueList& args, SharedValue result)
	{
		bool deleted = false;

		try
		{
			Poco::File file(this->filename);

			if(file.exists() && file.isFile())
			{
				file.remove();

				deleted = true;
			}
		}
		catch (Poco::Exception& exc)
		{
			throw exc.displayText().c_str();
		}

		result->SetBool(deleted);
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
					SharedValue value = Value::NewString(files.at(i));
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
			throw exc.displayText().c_str();
		}
	}
	void File::GetParent(const ValueList& args, SharedValue result)
	{
		try
		{
			Poco::Path path(this->filename);

			result->SetString(path.parent().toString().c_str());
		}
		catch (Poco::Exception& exc)
		{
			throw exc.displayText().c_str();
		}
	}
	void File::GetExists(const ValueList& args, SharedValue result)
	{
		bool exists = false;

		try
		{
			Poco::File file(this->filename);
			exists = file.exists();
		}
		catch (Poco::Exception& exc)
		{
			throw exc.displayText().c_str();
		}

		result->SetBool(exists);
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
			throw exc.displayText().c_str();
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
			throw exc.displayText().c_str();
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
			throw exc.displayText().c_str();
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
			throw exc.displayText().c_str();
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
			throw exc.displayText().c_str();
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
