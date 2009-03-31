/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "file.h"
#include "filesystem_utils.h"

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
		this->filename = [[[NSString stringWithCString:filename.c_str()] stringByExpandingTildeInPath] fileSystemRepresentation];
#else
		this->filename = filename;
#endif

		/**
		 * @tiapi(method=True,returns=string,name=Filesystem.File.toString) returns a string representation of the File object
		 */
		this->SetMethod("toString",&File::ToString);
		/**
		 * @tiapi(method=True,returns=boolean,name=Filesystem.File.isFile) returns true if this is a file
		 */
		this->SetMethod("isFile",&File::IsFile);
		/**
		 * @tiapi(method=True,returns=boolean,name=Filesystem.File.isDirectory) returns true if this is a directory
		 */
		this->SetMethod("isDirectory",&File::IsDirectory);
		/**
		 * @tiapi(method=True,returns=boolean,name=Filesystem.File.isHidden) returns true if this is a hidden file or directory
		 */
		this->SetMethod("isHidden",&File::IsHidden);
		/**
		 * @tiapi(method=True,returns=boolean,name=Filesystem.File.isSymbolicLink) returns true if this points to a symbolic link
		 */
		this->SetMethod("isSymbolicLink",&File::IsSymbolicLink);
		/**
		 * @tiapi(method=True,returns=boolean,name=Filesystem.File.isExecutable) returns true if this file is executable
		 */
		this->SetMethod("isExecutable",&File::IsExecutable);
		/**
		 * @tiapi(method=True,returns=boolean,name=Filesystem.File.isReadonly) returns true if this file or directory is read only
		 */
		this->SetMethod("isReadonly",&File::IsReadonly);
		/**
		 * @tiapi(method=True,returns=boolean,name=Filesystem.File.isWriteable) returns true if the file or directory is writeable
		 */
		this->SetMethod("isWriteable",&File::IsWriteable);
		/**
		 * @tiapi(method=True,returns=boolean,name=Filesystem.File.resolve) resolves the file
		 */
		this->SetMethod("resolve",&File::Resolve);
		/**
		 * @tiapi(method=True,returns=void,name=Filesystem.File.write) write data to the file
		 */
		this->SetMethod("write",&File::Write);
		/**
		 * @tiapi(method=True,returns=string,name=Filesystem.File.read) returns data as a string from the file
		 */
		this->SetMethod("read",&File::Read);
		/**
		 * @tiapi(method=True,returns=string,name=Filesystem.File.readLine) returns one line (separated by line ending) from the file
		 */
		this->SetMethod("readLine",&File::ReadLine);
		/**
		 * @tiapi(method=True,returns=boolean,name=Filesystem.File.copy) copy the file and returns true if successful
		 */
		this->SetMethod("copy",&File::Copy);
		/**
		 * @tiapi(method=True,returns=boolean,name=Filesystem.File.move) moves the file and returns true if successful
		 */
		this->SetMethod("move",&File::Move);
		/**
		 * @tiapi(method=True,returns=boolean,name=Filesystem.File.rename) renames the file and returns true if successful
		 */
		this->SetMethod("rename",&File::Rename);
		/**
		 * @tiapi(method=True,returns=boolean,name=Filesystem.File.createDirectory) creates a directory and returns true if successful
		 */
		this->SetMethod("createDirectory",&File::CreateDirectory);
		/**
		 * @tiapi(method=True,returns=boolean,name=Filesystem.File.deleteDirectory) deletes the directory and returns true if successful
		 */
		this->SetMethod("deleteDirectory",&File::DeleteDirectory);
		/**
		 * @tiapi(method=True,returns=boolean,name=Filesystem.File.deleteFile) deletes the file and returns true if successful
		 */
		this->SetMethod("deleteFile",&File::DeleteFile);
		/**
		 * @tiapi(method=True,returns=list,name=Filesystem.File.getDirectoryListing) returns list of directory entries
		 */
		this->SetMethod("getDirectoryListing",&File::GetDirectoryListing);
		/**
		 * @tiapi(method=True,returns=file,name=Filesystem.File.parent) returns the parent file
		 */
		this->SetMethod("parent",&File::GetParent);
		/**
		 * @tiapi(method=True,returns=boolean,name=Filesystem.File.exists) returns true if the file or directory exists
		 */
		this->SetMethod("exists",&File::GetExists);
		/**
		 * @tiapi(method=True,returns=double,name=Filesystem.File.createTimestamp) returns the created timestamp
		 */
		this->SetMethod("createTimestamp",&File::GetCreateTimestamp);
		/**
		 * @tiapi(method=True,returns=double,name=Filesystem.File.modificationTimestamp) returns the modification timestamp
		 */
		this->SetMethod("modificationTimestamp",&File::GetModificationTimestamp);
		/**
		 * @tiapi(method=True,returns=string,name=Filesystem.File.name) returns the name of the file or directory
		 */
		this->SetMethod("name",&File::GetName);
		/**
		 * @tiapi(method=True,returns=string,name=Filesystem.File.extension) returns the name of the file extension
		 */
		this->SetMethod("extension",&File::GetExtension);
		/**
		 * @tiapi(method=True,returns=string,name=Filesystem.File.nativePath) returns the full native path
		 */
		this->SetMethod("nativePath",&File::GetNativePath);
		/**
		 * @tiapi(method=True,returns=double,name=Filesystem.File.size) returns the size of the file in bytes
		 */
		this->SetMethod("size",&File::GetSize);
		/**
		 * @tiapi(method=True,returns=double,name=Filesystem.File.spaceAvailable) returns the space available on the filesystem
		 */
		this->SetMethod("spaceAvailable",&File::GetSpaceAvailable);
		/**
		 * @tiapi(method=True,returns=boolean,name=Filesystem.File.createShortcut) create a shortcut to the file
		 */
		this->SetMethod("createShortcut",&File::CreateShortcut);
		/**
		 * @tiapi(method=True,returns=boolean,name=Filesystem.File.setExecutable) make the file or directory executable
		 */
		this->SetMethod("setExecutable",&File::SetExecutable);
		/**
		 * @tiapi(method=True,returns=boolean,name=Filesystem.File.setReadonly) make the file or directory readonly
		 */
		this->SetMethod("setReadonly",&File::SetReadonly);
		/**
		 * @tiapi(method=True,returns=boolean,name=Filesystem.File.setWriteable) make the file or directory writeable
		 */
		this->SetMethod("setWriteable",&File::SetWriteable);
		/**
		 * @tiapi(method=True,returns=boolean,name=Filesystem.File.unzip) unzip the file into a directory
		 */
		this->SetMethod("unzip",&File::Unzip);

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
		catch (Poco::FileNotFoundException &fnf)
		{
			result->SetBool(false);
		}
		catch (Poco::PathNotFoundException &fnf)
		{
			result->SetBool(false);
		}
		catch (Poco::Exception& exc)
		{
			throw ValueException::FromString(exc.displayText());
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
		catch (Poco::FileNotFoundException &fnf)
		{
			result->SetBool(false);
		}
		catch (Poco::PathNotFoundException &fnf)
		{
			result->SetBool(false);
		}
		catch (Poco::Exception& exc)
		{
			throw ValueException::FromString(exc.displayText());
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
		catch (Poco::FileNotFoundException &fnf)
		{
			result->SetBool(false);
		}
		catch (Poco::PathNotFoundException &fnf)
		{
			result->SetBool(false);
		}
		catch (Poco::Exception& exc)
		{
			throw ValueException::FromString(exc.displayText());
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
		catch (Poco::FileNotFoundException &fnf)
		{
			result->SetBool(false);
		}
		catch (Poco::PathNotFoundException &fnf)
		{
			result->SetBool(false);
		}
		catch (Poco::Exception& exc)
		{
			throw ValueException::FromString(exc.displayText());
		}
	}
	void File::IsExecutable(const ValueList& args, SharedValue result)
	{
		try
		{
			Poco::File file(this->filename);
			result->SetBool(file.canExecute());
		}
		catch (Poco::FileNotFoundException &fnf)
		{
			result->SetBool(false);
		}
		catch (Poco::PathNotFoundException &fnf)
		{
			result->SetBool(false);
		}
		catch (Poco::Exception& exc)
		{
			throw ValueException::FromString(exc.displayText());
		}
	}
	void File::IsReadonly(const ValueList& args, SharedValue result)
	{
		try
		{
			Poco::File file(this->filename);
			result->SetBool(file.canRead());
		}
		catch (Poco::FileNotFoundException &fnf)
		{
			result->SetBool(false);
		}
		catch (Poco::PathNotFoundException &fnf)
		{
			result->SetBool(false);
		}
		catch (Poco::Exception& exc)
		{
			throw ValueException::FromString(exc.displayText());
		}
	}
	void File::IsWriteable(const ValueList& args, SharedValue result)
	{
		try
		{
			Poco::File file(this->filename);
			result->SetBool(file.canWrite());
		}
		catch (Poco::FileNotFoundException &fnf)
		{
			result->SetBool(false);
		}
		catch (Poco::PathNotFoundException &fnf)
		{
			result->SetBool(false);
		}
		catch (Poco::Exception& exc)
		{
			throw ValueException::FromString(exc.displayText());
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
			throw ValueException::FromString(exc.displayText());
		}
	}
	void File::Write(const ValueList& args, SharedValue result)
	{
		std::string mode = FileStream::MODE_WRITE;

		if(args.size() > 1)
		{
			if(args.at(1)->ToBool())
			{
				mode = FileStream::MODE_APPEND;
			}
		}

		ti::FileStream fs(this->filename);
		fs.Open(mode);
		fs.Write(args, result);
		fs.Close();
	}
	void File::Read(const ValueList& args, SharedValue result)
	{
		FileStream fs(this->filename);
		fs.Open(FileStream::MODE_READ);
		fs.Read(args, result);
		fs.Close();
	}
	void File::ReadLine(const ValueList& args, SharedValue result)
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
				this->readLineFS->Close();
			}

			// now open the file
			this->readLineFS = new ti::FileStream(this->filename);
			this->readLineFS->Open(FileStream::MODE_READ);
		}

		if(this->readLineFS == NULL)
		{
			result->SetNull();
		}
		else
		{
			this->readLineFS->ReadLine(args, result);
		}
	}
	void File::Copy(const ValueList& args, SharedValue result)
	{
		try
		{
			std::string dest = FileSystemUtils::GetFileName(args.at(0));
			Poco::File from(this->filename);
			from.copyTo(dest);
			result->SetBool(true);
		}
		catch (Poco::Exception& exc)
		{
			throw ValueException::FromString(exc.displayText());
		}
	}
	void File::Move(const ValueList& args, SharedValue result)
	{
		try
		{
			std::string dest = FileSystemUtils::GetFileName(args.at(0));
			Poco::File from(this->filename);
			from.moveTo(dest);
			result->SetBool(true);
		}
		catch (Poco::Exception& exc)
		{
			throw ValueException::FromString(exc.displayText());
		}
	}
	void File::Rename(const ValueList& args, SharedValue result)
	{
		try
		{
			std::string name = args.at(0)->ToString();
			Poco::File f(this->filename);
			Poco::Path p(this->filename);
			p.setFileName(name);
			f.renameTo(p.toString());
			result->SetBool(true);
		}
		catch (Poco::Exception& exc)
		{
			throw ValueException::FromString(exc.displayText());
		}
	}
	void File::CreateDirectory(const ValueList& args, SharedValue result)
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
			throw ValueException::FromString(exc.displayText());
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
			throw ValueException::FromString(exc.displayText());
		}
	}
	void File::DeleteFile(const ValueList& args, SharedValue result)
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
			throw ValueException::FromString(exc.displayText());
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
					std::string entry = files.at(i);
					// store it as the fullpath
					std::string filename = kroll::FileUtils::Join(this->filename.c_str(),entry.c_str(),NULL);
					ti::File* file = new ti::File(filename);
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
			throw ValueException::FromString(exc.displayText());
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
			throw ValueException::FromString(exc.displayText());
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
			throw ValueException::FromString(exc.displayText());
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
			throw ValueException::FromString(exc.displayText());
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
			throw ValueException::FromString(exc.displayText());
		}
	}
	void File::GetName(const ValueList& args, SharedValue result)
	{
		try
		{
			Poco::Path path(this->filename);
			result->SetString(path.getFileName().c_str());
		}
		catch (Poco::Exception& exc)
		{
			throw ValueException::FromString(exc.displayText());
		}
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
			throw ValueException::FromString(exc.displayText());
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
			throw ValueException::FromString(exc.displayText());
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
			throw ValueException::FromString(exc.displayText());
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
	void File::CreateShortcut(const ValueList& args, SharedValue result)
	{
		if (args.size()<1)
		{
			throw ValueException::FromString("createShortcut takes a parameter");
		}
		std::string from = this->filename;
		std::string to = args.at(0)->IsString() ? args.at(0)->ToString() : FileSystemUtils::GetFileName(args.at(0));

#ifdef OS_OSX	//TODO: My spidey sense tells me that Cocoa might have a better way for this. --BTH
		NSMutableString* originalPath = [NSMutableString stringWithCString:from.c_str()];
		NSString* destPath = [NSString stringWithCString:to.c_str()];
		NSString* cwd = nil;
		NSFileManager* fm = [NSFileManager defaultManager];

		// support 2nd argument as a relative path to symlink for
		if (args.size()>1)
		{
			cwd = [fm currentDirectoryPath];
			NSString *p = [NSString stringWithCString:FileSystemUtils::GetFileName(args.at(1))];
			BOOL isDirectory = NO;
			if ([fm fileExistsAtPath:p isDirectory:&isDirectory])
			{
				if (!isDirectory)
				{
					// trim it off to see if it's a directory
					p = [p stringByDeletingLastPathComponent];
				}
				[fm changeCurrentDirectoryPath:p];
				
				NSString * doomedString = [p stringByAppendingString: @"/"];
				[originalPath replaceOccurrencesOfString:doomedString withString: @"" options: NSLiteralSearch range:NSMakeRange(0,[originalPath length])];
//				originalPath = [originalPath stringByReplacingOccurrencesOfString:[NSString stringWithFormat:@"%@/",p] withString:@""];
			}
		}

		int rc = symlink([originalPath UTF8String],[destPath UTF8String]);
		BOOL worked = rc >= 0;
#ifdef DEBUG
		NSLog(@"++++ SYMLINK:%@=>%@ (%d)",originalPath,destPath,worked);
#endif
		result->SetBool(worked);
		if (cwd)
		{
			[[NSFileManager defaultManager] changeCurrentDirectoryPath:cwd];
		}
#elif OS_WIN32
		HRESULT hResult;
		IShellLink* psl;

		if(from.length() == 0 || to.length() == 0) {
			std::string ex = "Invalid arguments given to createShortcut()";
			throw ValueException::FromString(ex);
		}

		hResult = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);

		if(SUCCEEDED(hResult))
		{
			IPersistFile* ppf;

			// set path to the shortcut target and add description
			psl->SetPath(from.c_str());
			psl->SetDescription("File shortcut");

			hResult = psl->QueryInterface(IID_IPersistFile, (LPVOID*) &ppf);

			if(SUCCEEDED(hResult))
			{
				// ensure to ends with .lnk
				to.append(".lnk");
				WCHAR wsz[MAX_PATH];

				// ensure string is unicode
				if(MultiByteToWideChar(CP_ACP, 0, to.c_str(), -1, wsz, MAX_PATH))
				{
					// save the link
					hResult = ppf->Save(wsz, TRUE);
					ppf->Release();

					if(SUCCEEDED(hResult))
					{
						result->SetBool(true);
						return ;
					}
				}
			}
		}
		result->SetBool(false);
#elif OS_LINUX
		result->SetBool(link(this->filename.c_str(), to.c_str()) == 0);
#else
		result->SetBool(false);
#endif

	}
	void File::SetExecutable(const ValueList& args, SharedValue result)
	{
		try
		{
			Poco::File file(this->filename);
			file.setExecutable(args.at(0)->ToBool());
			result->SetBool(true);
		}
		catch (Poco::FileNotFoundException &fnf)
		{
			result->SetBool(false);
		}
		catch (Poco::PathNotFoundException &fnf)
		{
			result->SetBool(false);
		}
		catch (Poco::Exception& exc)
		{
			throw ValueException::FromString(exc.displayText());
		}
	}
	void File::SetReadonly(const ValueList& args, SharedValue result)
	{
		try
		{
			Poco::File file(this->filename);
			file.setReadOnly(args.at(0)->ToBool());
			result->SetBool(true);
		}
		catch (Poco::FileNotFoundException &fnf)
		{
			result->SetBool(false);
		}
		catch (Poco::PathNotFoundException &fnf)
		{
			result->SetBool(false);
		}
		catch (Poco::Exception& exc)
		{
			throw ValueException::FromString(exc.displayText());
		}
	}
	void File::SetWriteable(const ValueList& args, SharedValue result)
	{
		try
		{
			Poco::File file(this->filename);
			file.setWriteable(args.at(0)->ToBool());
			result->SetBool(true);
		}
		catch (Poco::FileNotFoundException &fnf)
		{
			result->SetBool(false);
		}
		catch (Poco::PathNotFoundException &fnf)
		{
			result->SetBool(false);
		}
		catch (Poco::Exception& exc)
		{
			throw ValueException::FromString(exc.displayText());
		}
	}
	/**
	 * Function: Unzip
	 *   unzip this file to destination
	 *
	 * Parameters:
	 *   dest - destination directory to unzip this file
	 *
	 * Returns:
	 *   true if succeeded
	 */
	void File::Unzip(const ValueList& args, SharedValue result)
	{
		if (args.size()!=1)
		{
			throw ValueException::FromString("invalid arguments - expected destination");
		}
		try
		{
			Poco::File from(this->filename);
			Poco::File to(FileSystemUtils::GetFileName(args.at(0)));
			std::string from_s = from.path();
			std::string to_s = to.path();
			if (!to.exists())
			{
				to.createDirectory();
			}
			if (!to.isDirectory())
			{
				throw ValueException::FromString("destination must be a directory");
			}
			kroll::FileUtils::Unzip(from_s,to_s);
			result->SetBool(true);
		}
		catch (Poco::FileNotFoundException &fnf)
		{
			result->SetBool(false);
		}
		catch (Poco::PathNotFoundException &fnf)
		{
			result->SetBool(false);
		}
		catch (Poco::Exception& exc)
		{
			throw ValueException::FromString(exc.displayText());
		}
	}
}
