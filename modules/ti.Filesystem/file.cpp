/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "file.h"

#include <Poco/File.h>
#include <Poco/FileStream.h>
#include <Poco/Exception.h>

namespace ti
{
	File::File(BoundObject *global, std::string filename) : global(global), filename(filename)
	{
		KR_ADDREF(global);

		this->SetMethod("toString",&File::ToString);
		this->SetMethod("isFile",&File::IsFile);
		this->SetMethod("isDirectory",&File::IsDirectory);
		this->SetMethod("isHidden",&File::IsHidden);

		this->SetMethod("isSymbolicLink",&File::IsSymbolicLink);
		this->SetMethod("exists",&File::Exists);
		this->SetMethod("read",&File::Read);
		this->SetMethod("copy",&File::Copy);
		this->SetMethod("move",&File::Move);
		this->SetMethod("createDirectory",&File::CreateDirectoryX);
		this->SetMethod("deleteDirectory",&File::DeleteDirectory);
		this->SetMethod("deleteFile",&File::DeleteFileX);
		this->SetMethod("getDirectoryListing",&File::GetDirectoryListing);
	}

	File::~File()
	{
		KR_DECREF(global);
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
			std::cerr << "Problem getting file info:::: " << exc.displayText() << std::endl;
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
			std::cerr << "Problem getting file info:::: " << exc.displayText() << std::endl;
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
			std::cerr << "Problem getting file info:::: " << exc.displayText() << std::endl;
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
			std::cerr << "Problem getting file info:::: " << exc.displayText() << std::endl;
		}

		result->SetBool(isLink);
	}
	void File::Exists(const ValueList& args, SharedValue result)
	{
		bool exists = false;

		try
		{
			Poco::File file(this->filename);
			exists = file.exists();
		}
		catch (Poco::Exception& exc)
		{
			std::cerr << "Problem getting file info:::: " << exc.displayText() << std::endl;
		}

		result->SetBool(exists);
	}
	void File::Read(const ValueList& args, SharedValue result)
	{
		try
		{
			Poco::File file(this->filename);
			if(file.canRead())
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
			else
			{
				result->SetNull();
			}
		}
		catch (Poco::Exception& exc)
		{
			std::cerr << "Problem reading file:::: " << exc.displayText() << std::endl;

			result->SetNull();
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
			std::cerr << "Problem copying file:::: " << exc.displayText() << std::endl;
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
			std::cerr << "Problem moving file:::: " << exc.displayText() << std::endl;
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
			std::cerr << "Problem creating directory:::: " << exc.displayText() << std::endl;
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
			std::cerr << "Problem deleting directory:::: " << exc.displayText() << std::endl;
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
			std::cerr << "Problem deleting file:::: " << exc.displayText() << std::endl;
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

				for(int i = 0; i < files.size(); i++)
				{
					SharedValue value = new Value(files.at(i));
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
			std::cerr << "Problem deleting file:::: " << exc.displayText() << std::endl;

			result->SetNull();
		}
	}
}
