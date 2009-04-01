/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "async_copy.h"
#include "filesystem_binding.h"

#ifndef OS_WIN32
#include <unistd.h>
#include <string.h>
#include <errno.h>
#endif

namespace ti
{
	AsyncCopy::AsyncCopy(
		FilesystemBinding* parent, 
		Host *host, 
		std::vector<std::string> files, 
		std::string destination, 
		SharedBoundMethod callback)
			: parent(parent),
			  host(host),
			  files(files),
			  destination(destination),
			  callback(callback),
			  stopped(false)
	{
		/**
		 * @tiapi(property=True,type=boolean,name=Filesystem.AsyncCopy.running,since=0.3) boolean property to indicate if the copy operation is running
		 */
		this->Set("running",Value::NewBool(true));
		this->thread = new Poco::Thread();
		this->thread->start(&AsyncCopy::Run,this);
	}
	AsyncCopy::~AsyncCopy()
	{
		if (this->thread!=NULL)
		{
			this->thread->tryJoin(10); // precaution, should already be stopped
			delete this->thread;
			this->thread = NULL;
		}
	}
	void AsyncCopy::Copy(Poco::Path &src, Poco::Path &dest)
	{
		Poco::File from(src.toString());
#ifdef DEBUG
		std::cout << "COPY: " << src.toString() << " => " << dest.toString() << " LINK=" << from.isLink() << std::endl;
#endif
		if (from.isDirectory() && !from.isLink())
		{
			Poco::File d(dest.toString());
			if (!d.exists())
			{
				d.createDirectories();
			}
			std::vector<std::string> files;
			from.list(files);
			std::vector<std::string>::iterator i = files.begin();
			while(i!=files.end())
			{
				std::string fn = (*i++);
				Poco::Path sp(kroll::FileUtils::Join(src.toString().c_str(),fn.c_str(),NULL));
				Poco::Path dp(kroll::FileUtils::Join(dest.toString().c_str(),fn.c_str(),NULL));
				this->Copy(sp,dp);
			}
		}
#ifndef OS_WIN32
		else if (from.isLink())
		{
			char linkPath[PATH_MAX];
			ssize_t length = readlink(from.path().c_str(), linkPath, PATH_MAX);
			linkPath[length] = '\0';

			std::string newPath (dest.toString());
			const char *destPath = newPath.c_str();
			// unlink it first, fails in some OS if already there
			unlink(destPath);
			int result = symlink(linkPath, destPath);
#ifdef DEBUG
			std::cout << "Result: " << result << " for file: " << destPath << std::endl;
#endif
			if (result == -1)
			{
				std::string err = "Copy failed: Could not make symlink (";
				err.append(destPath);
				err.append(") from ");
				err.append(linkPath);
				err.append(" : ");
				err.append(strerror(errno));
				throw kroll::ValueException::FromString(err);
			}

		}
#endif
		else
		{
			// in this case it's a regular file
			Poco::File s(src.toString());
			s.copyTo(dest.toString().c_str());
		}
	}
	void AsyncCopy::Run(void* data)
	{
#ifdef OS_OSX
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
#endif

		AsyncCopy* ac = static_cast<AsyncCopy*>(data);
#ifdef DEBUG
		std::cout << "async copy started with " << ac->files.size() << " files" << std::endl;
#endif
		std::vector<std::string>::iterator iter = ac->files.begin();
		Poco::Path to(ac->destination);
		Poco::File tof(to.toString());
#ifdef DEBUG
		std::cout << "ASYNC DEST: " << ac->destination << std::endl;
#endif
		if (!tof.exists())
		{
			tof.createDirectory();
		}
		int c = 0;
		while(!ac->stopped && iter!=ac->files.end())
		{
			std::string file = (*iter++);
			c++;
#ifdef DEBUG
			std::cout << "copying async file: " << file << " (" << c << "/" << ac->files.size() << ")" << std::endl;
#endif
			try
			{
				Poco::Path from(file);
				Poco::File f(file);
				if (f.isDirectory())
				{
					ac->Copy(from,to);
				}
				else
				{
					Poco::Path dest(to,from.getFileName());
					ac->Copy(from,dest);
				}
#ifdef DEBUG
			std::cout << "copied async file: " << file << " (" << c << "/" << ac->files.size() << ")" << std::endl;
#endif
				SharedValue value = Value::NewString(file);
				ValueList args;
				args.push_back(value);
				args.push_back(Value::NewInt(c));
				args.push_back(Value::NewInt(ac->files.size()));
				ac->host->InvokeMethodOnMainThread(ac->callback, args, false);
#ifdef DEBUG
			std::cout << "after callback for async file: " << file << " (" << c << "/" << ac->files.size() << ")" << std::endl;
#endif
			}
			catch (ValueException &ex)
			{
				SharedString ss = ex.DisplayString();
				std::cerr << "Error running async file copy: " << *ss << " for file: " << file << std::endl;
			}
			catch (Poco::Exception &ex)
			{
				std::cerr << "Error running async file copy: " << ex.displayText() << " for file: " << file << std::endl;
			}
			catch (std::exception &ex)
			{
				std::cerr << "Error running async file copy: " << ex.what() << " for file: " << file << std::endl;
			}
			catch (...)
			{
				std::cerr << "Unknown error running async file copy for file: " << file << std::endl;
			}
		}
		ac->Set("running",Value::NewBool(false));
		ac->stopped = true;
#ifdef DEBUG
		std::cout << "async copy finished by copying " << c << " files" << std::endl;
#endif
#ifdef OS_OSX
		[pool release];
#endif
	}
	void AsyncCopy::ToString(const ValueList& args, SharedValue result)
	{
		result->SetString("[Async Copy]");
	}
	void AsyncCopy::Cancel(const ValueList& args, SharedValue result)
	{
		KR_DUMP_LOCATION
		if (thread!=NULL && thread->isRunning())
		{
			this->stopped = true;
			this->Set("running",Value::NewBool(false));
			result->SetBool(true);
		}
		else
		{
			result->SetBool(false);
		}
	}
}
