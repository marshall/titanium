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
		 * @tiapi(method=True,name=Filesystem.File.toString,since=0.2) returns a string representation of the File object
		 * @tiresult(for=Filesystem.File.toString,type=string)
		 */
		this->SetMethod("toString",&File::ToString);
		/**
		 * @tiapi(method=True,name=Filesystem.File.isFile,since=0.2) returns true if this is a file
		 * @tiresult(for=Filesystem.File.isFile,type=boolean)
		 */
		this->SetMethod("isFile",&File::IsFile);
		/**
		 * @tiapi(method=True,name=Filesystem.File.isDirectory,since=0.2) returns true if this is a directory
		 * @tiresult(for=Filesystem.File.isDirectory,type=boolean)
		 */
		this->SetMethod("isDirectory",&File::IsDirectory);
		/**
		 * @tiapi(method=True,name=Filesystem.File.isHidden,since=0.2) returns true if this is a hidden file or directory
		 * @tiresult(for=Filesystem.File.isHidden,type=boolean)
		 */
		this->SetMethod("isHidden",&File::IsHidden);
		/**
		 * @tiapi(method=True,name=Filesystem.File.isSymbolicLink,since=0.2) returns true if this points to a symbolic link
		 * @tiresult(for=Filesystem.File.isSymbolicLink,type=boolean)
		 */
		this->SetMethod("isSymbolicLink",&File::IsSymbolicLink);
		/**
		 * @tiapi(method=True,name=Filesystem.File.isExecutable,since=0.2) returns true if this file is executable
		 * @tiresult(for=Filesystem.File.isExecutable,type=boolean)
		 */
		this->SetMethod("isExecutable",&File::IsExecutable);
		/**
		 * @tiapi(method=True,name=Filesystem.File.isReadonly,since=0.2) returns true if this file or directory is read only
		 * @tiresult(for=Filesystem.File.isReadonly,type=boolean)
		 */
		this->SetMethod("isReadonly",&File::IsReadonly);
		/**
		 * @tiapi(method=True,name=Filesystem.File.isWriteable,since=0.2) returns true if the file or directory is writeable
		 * @tiresult(for=Filesystem.File.isWriteable,type=boolean)
		 */
		this->SetMethod("isWriteable",&File::IsWriteable);
		/**
		 * @tiapi(method=True,name=Filesystem.File.resolve,since=0.2) resolves a file path
		 * @tiarg(for=Filesystem.File.resolve,name=path) path to resolve
		 * @tiresult(for=Filesystem.File.resolve,type=object)
		 */
		this->SetMethod("resolve",&File::Resolve);
		/**
		 * @tiapi(method=True,returns=void,name=Filesystem.File.write,since=0.2) write data to the file
		 * @tiarg(for=Filesystem.File.write,type=string,name=data) data to write
		 */
		this->SetMethod("write",&File::Write);
		/**
		 * @tiapi(method=True,name=Filesystem.File.read,since=0.2) returns data as a string from the file
		 * @tiresult(for=Filesystem.File.read,type=string)
		 */
		this->SetMethod("read",&File::Read);
		/**
		 * @tiapi(method=True,name=Filesystem.File.readLine,since=0.2) returns one line (separated by line ending) from the file
		 * @tiresult(for=Filesystem.File.readLine,type=string) data
		 */
		this->SetMethod("readLine",&File::ReadLine);
		/**
		 * @tiapi(method=True,name=Filesystem.File.copy,since=0.2) copy the file and returns true if successful
		 * @tiarg(for=Filesystem.File.copy,type=string,name=destination) destination to copy to
		 * @tiresult(for=Filesystem.File.copy,type=boolean) true if successful
		 */
		this->SetMethod("copy",&File::Copy);
		/**
		 * @tiapi(method=True,name=Filesystem.File.move,since=0.2) moves the file and returns true if successful
		 * @tiarg(for=Filesystem.File.move,type=string,name=destination) destination to move to
		 * @tiresult(for=Filesystem.File.move,type=boolean) true if successful
		 */
		this->SetMethod("move",&File::Move);
		/**
		 * @tiapi(method=True,name=Filesystem.File.rename,since=0.2) renames the file and returns true if successful
		 * @tiarg(for=Filesystem.File.rename,type=string,name=destination) destination to rename to
		 * @tiresult(for=Filesystem.File.rename,type=boolean) true if successful
		 */
		this->SetMethod("rename",&File::Rename);
		/**
		 * @tiapi(method=True,name=Filesystem.File.createDirectory,since=0.2) creates a directory and returns true if successful
		 * @tiresult(for=Filesystem.File.createDirectory,type=boolean) true if successful
		 */
		this->SetMethod("createDirectory",&File::CreateDirectory);
		/**
		 * @tiapi(method=True,name=Filesystem.File.deleteDirectory,since=0.2) deletes the directory and returns true if successful
		 * @tiresult(for=Filesystem.File.deleteDirectory,type=boolean) true if successful
		 */
		this->SetMethod("deleteDirectory",&File::DeleteDirectory);
		/**
		 * @tiapi(method=True,name=Filesystem.File.deleteFile,since=0.2) deletes the file and returns true if successful
		 * @tiresult(for=Filesystem.File.deleteFile,type=boolean) true if successful
		 */
		this->SetMethod("deleteFile",&File::DeleteFile);
		/**
		 * @tiapi(method=True,returns=list,name=Filesystem.File.getDirectoryListing,since=0.2) returns list of directory entries
		 * @tiresult(for=Filesystem.File.getDirectoryListing,type=list) returns a list of names
		 */
		this->SetMethod("getDirectoryListing",&File::GetDirectoryListing);
		/**
		 * @tiapi(method=True,returns=file,name=Filesystem.File.parent,since=0.2) returns the parent file
		 * @tiresult(for=Filesystem.File.parent,type=object) returns the parent file
		 */
		this->SetMethod("parent",&File::GetParent);
		/**
		 * @tiapi(method=True,name=Filesystem.File.exists,since=0.2) returns true if the file or directory exists
		 * @tiresult(for=Filesystem.File.exists,type=boolean) returns true if exists
		 */
		this->SetMethod("exists",&File::GetExists);
		/**
		 * @tiapi(method=True,name=Filesystem.File.createTimestamp,since=0.2) returns the created timestamp
		 * @tiresult(for=Filesystem.File.createTimestamp,type=double) created timestamp for file
		 */
		this->SetMethod("createTimestamp",&File::GetCreateTimestamp);
		/**
		 * @tiapi(method=True,name=Filesystem.File.modificationTimestamp,since=0.2) returns the modification timestamp
		 * @tiresult(for=Filesystem.File.modificationTimestamp,type=double) modification timestamp for file
		 */
		this->SetMethod("modificationTimestamp",&File::GetModificationTimestamp);
		/**
		 * @tiapi(method=True,name=Filesystem.File.name,since=0.2) returns the name of the file or directory
		 * @tiresult(for=Filesystem.File.name,type=string) name of the file part of the path
		 */
		this->SetMethod("name",&File::GetName);
		/**
		 * @tiapi(method=True,name=Filesystem.File.extension,since=0.2) returns the name of the file extension
		 * @tiresult(for=Filesystem.File.extension,type=string) file extension
		 */
		this->SetMethod("extension",&File::GetExtension);
		/**
		 * @tiapi(method=True,name=Filesystem.File.nativePath,since=0.2) returns the full native path
		 * @tiresult(for=Filesystem.File.nativePath,type=string) full native path
		 */
		this->SetMethod("nativePath",&File::GetNativePath);
		/**
		 * @tiapi(method=True,name=Filesystem.File.size,since=0.2) returns the size of the file in bytes
		 * @tiresult(for=Filesystem.File.size,type=double) file size
		 */
		this->SetMethod("size",&File::GetSize);
		/**
		 * @tiapi(method=True,name=Filesystem.File.spaceAvailable,since=0.2) returns the space available on the filesystem
		 * @tiresult(for=Filesystem.File.spaceAvailable,type=double) space available
		 */
		this->SetMethod("spaceAvailable",&File::GetSpaceAvailable);
		/**
		 * @tiapi(method=True,name=Filesystem.File.createShortcut,since=0.2) create a shortcut to the file
		 * @tiresult(for=Filesystem.File.createShortcut,type=boolean) returns true if successful
		 */
		this->SetMethod("createShortcut",&File::CreateShortcut);
		/**
		 * @tiapi(method=True,name=Filesystem.File.setExecutable,since=0.2) make the file or directory executable
		 * @tiresult(for=Filesystem.File.setExecutable,type=boolean) returns true if successful
		 */
		this->SetMethod("setExecutable",&File::SetExecutable);
		/**
		 * @tiapi(method=True,name=Filesystem.File.setReadonly,since=0.2) make the file or directory readonly
		 * @tiresult(for=Filesystem.File.setReadonly,type=boolean) returns true if successful
		 */
		this->SetMethod("setReadonly",&File::SetReadonly);
		/**
		 * @tiapi(method=True,name=Filesystem.File.setWriteable,since=0.2) make the file or directory writeable
		 * @tiresult(for=Filesystem.File.setWriteable,type=boolean) returns true if successful
		 */
		this->SetMethod("setWriteable",&File::SetWriteable);
		/**
		 * @tiapi(method=True,name=Filesystem.File.unzip,since=0.3) unzip the file into a directory
		 * @tiarg(for=Filesystem.File.unzip,type=object,name=destination) destination to unzip to
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
					SharedValue value = Value::NewObject((SharedKObject) file);
					fileList->Append(value);
				}

				SharedKList list = fileList;
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
