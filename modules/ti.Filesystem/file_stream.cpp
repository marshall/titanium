/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#include "file_stream.h"
#include <cstring>

#include <Poco/LineEndingConverter.h>

namespace ti {
std::string FileStream::MODE_READ = "read";
std::string FileStream::MODE_APPEND = "append";
std::string FileStream::MODE_WRITE = "write";

FileStream::FileStream(std::string filename_) : stream(NULL)
{
#ifdef OS_OSX
	// in OSX, we need to expand ~ in paths to their absolute path value
	// we do that with a nifty helper method in NSString
	this->filename = [[[NSString stringWithCString:filename_.c_str()] stringByExpandingTildeInPath] fileSystemRepresentation];
#else
	this->filename = filename_;
#endif

	/**
	 * @tiapi(method=True,name=Filesystem.Filestream.open,since=0.2) open the file
	 * @tiresult(for=Filesystem.Filestream.open,type=boolean) returns true if successful
	 */
	this->SetMethod("open",&FileStream::Open);
	/**
	 * @tiapi(method=True,name=Filesystem.Filestream.close,since=0.2) close the file
	 * @tiresult(for=Filesystem.Filestream.close,type=boolean) returns true if successful
	 */
	this->SetMethod("close",&FileStream::Close);
	/**
	 * @tiapi(method=True,name=Filesystem.Filestream.read,since=0.2) read from the file
	 * @tiresult(for=Filesystem.Filestream.read,type=string) returns data as string
	 */
	this->SetMethod("read",&FileStream::Read);
	/**
	 * @tiapi(method=True,name=Filesystem.Filestream.readLine,since=0.2) read one line from the file
	 * @tiresult(for=Filesystem.Filestream.readLine,type=string) returns data as string
	 */
	this->SetMethod("readLine",&FileStream::ReadLine);
	/**
	 * @tiapi(method=True,name=Filesystem.Filestream.write,since=0.2) write into the file
	 * @tiarg(for=Filesystem.Filestream.write,type=string,name=data) data to write
	 * @tiresult(for=Filesystem.Filestream.write,type=boolean) returns true if successful
	 */
	this->SetMethod("write",&FileStream::Write);
}

FileStream::~FileStream()
{
	this->Close();
}

void FileStream::Open(const ValueList& args, SharedValue result)
{
	std::string mode = FileStream::MODE_READ;
	bool binary = false;
	bool append = false;
	if (args.size()>=1) mode = args.at(0)->ToString();
	if (args.size()>=2) binary = args.at(1)->ToBool();
	if (args.size()>=3) append = args.at(2)->ToBool();
	bool opened = this->Open(mode,binary,append);
	result->SetBool(opened);
}
bool FileStream::Open(std::string mode, bool binary, bool append)
{
	// close the prev stream if needed
	this->Close();

	try
	{
		std::ios::openmode flags = (std::ios::openmode) 0;
		bool output = false;
		if (binary)
		{
			flags|=std::ios::binary;
		}
		if (mode == FileStream::MODE_APPEND)
		{
			flags|=std::ios::out|std::ios::app;
			output = true;
		}
		else if (mode == FileStream::MODE_WRITE)
		{
			flags |= std::ios::out|std::ios::trunc;
			output = true;
		}
		else if (mode == FileStream::MODE_READ)
		{
			flags |= std::ios::in;
		}

#ifdef DEBUG
		Logger& logger = Logger::Get("Filesystem.FileStream");
		logger.Debug("FILE OPEN FLAGS = %d, binary=%d, mode=%s, append=%d",flags,binary,mode.c_str(),append);
#endif
		if (output)
		{
			this->stream = new Poco::FileOutputStream(this->filename,flags);
		}
		else
		{
			this->stream = new Poco::FileInputStream(this->filename,flags);
		}
		return true;
	}
	catch (Poco::Exception& exc)
	{
		throw ValueException::FromString(exc.displayText());
	}
}
void FileStream::Close(const ValueList& args, SharedValue result)
{
	bool closed = this->Close();
	result->SetBool(closed);
}
bool FileStream::Close()
{
	try
	{
		if(this->stream)
		{
			this->stream->close();
			this->stream = NULL;
			return true;
		}
	}
	catch (Poco::Exception& exc)
	{
		throw ValueException::FromString(exc.displayText());
	}

	return false;
}
void FileStream::Write(const ValueList& args, SharedValue result)
{
	try
	{
		char *text = NULL;
		int size = 0;
		if (args.at(0)->IsObject())
		{
			SharedBoundObject b = args.at(0)->ToObject();
			SharedPtr<Blob> blob = b.cast<Blob>();
			if (!blob.isNull())
			{
				text = (char*)blob->Get();
				size = (int)blob->Length();
			}
		}
		else if (args.at(0)->IsString())
		{
			text = (char*)args.at(0)->ToString();
		}

		if (size==0)
		{
			size = strlen(text);
		}

		if (text == NULL)
		{
			throw ValueException::FromString("couldn't determine value");
		}
		if (size <= 0)
		{
			throw ValueException::FromString("couldn't determine size");
		}

		Poco::FileOutputStream* fos = dynamic_cast<Poco::FileOutputStream*>(this->stream);
		if(!fos)
		{
			throw ValueException::FromString("FileStream must be opened for writing before calling write");
		}

		fos->write(text, size);
		result->SetBool(true);
#ifdef DEBUG
		std::cout << "wrote: " << size << " bytes" << std::endl;
#endif
	}
	catch (Poco::Exception& exc)
	{
		throw ValueException::FromString(exc.displayText());
	}
}
void FileStream::Read(const ValueList& args, SharedValue result)
{
	if(!this->stream)
	{
		throw ValueException::FromString("FileStream must be opened before calling read");
	}

	try
	{
		std::string contents;

		Poco::FileInputStream* fis = dynamic_cast<Poco::FileInputStream*>(this->stream);
		if(!fis)
		{
			throw ValueException::FromString("FileStream must be opened for reading before calling read");
		}

		while(!fis->eof())
		{
			std::string s;
			std::getline(*fis, s);

			contents.append(s);
			contents.append(Poco::LineEnding::NEWLINE_DEFAULT);
		}

		result->SetString(contents.c_str());
	}
	catch (Poco::Exception& exc)
	{
		throw ValueException::FromString(exc.displayText());
	}
}
void FileStream::ReadLine(const ValueList& args, SharedValue result)
{
	if(! this->stream)
	{
		throw ValueException::FromString("FileStream must be opened before calling readLine");
	}

	try
	{
		std::string line;

		Poco::FileInputStream* fis = dynamic_cast<Poco::FileInputStream*>(this->stream);
		if(! fis)
		{
			throw ValueException::FromString("FileStream must be opened for reading before calling readLine");
		}

		if(fis->eof())
		{
			// close the file
			result->SetNull();
		}
		else
		{
			std::string line;
			std::getline(*fis, line);
			if (line.empty())
			{
				result->SetString("");
			}
			else
			{
				result->SetString(line.c_str());
			}
		}
	}
	catch (Poco::Exception& exc)
	{
		throw ValueException::FromString(exc.displayText());
	}
}

}
