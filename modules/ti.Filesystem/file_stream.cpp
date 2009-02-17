/*
 * file_stream.cpp
 *
 *  Created on: Feb 15, 2009
 *      Author: jorge
 */

#include "file_stream.h"

namespace ti {
std::string FileStream::MODE_READ = "read";
std::string FileStream::MODE_APPEND = "append";
std::string FileStream::MODE_WRITE = "write";

FileStream::FileStream(std::string filename_) : stream(NULL) {
#ifdef OS_OSX
		// in OSX, we need to expand ~ in paths to their absolute path value
		// we do that with a nifty helper method in NSString
		this->filename = [[[NSString stringWithCString:filename_.c_str()] stringByExpandingTildeInPath] fileSystemRepresentation];
#else
		this->filename = filename_;
#endif

		this->SetMethod("open",&FileStream::Open);
		this->SetMethod("close",&FileStream::Close);
		this->SetMethod("read",&FileStream::Read);
		this->SetMethod("readLine",&FileStream::ReadLine);
		this->SetMethod("write",&FileStream::Write);
}

FileStream::~FileStream() {
	this->Close();
}

void FileStream::Open(const ValueList& args, SharedValue result)
{
	std::string mode = args.at(0)->ToString();
	bool opened = this->Open(mode);

	result->SetBool(opened);
}
bool FileStream::Open(std::string mode)
{
	// close the prev stream if needed
	this->Close();

	try
	{
		if(mode == FileStream::MODE_READ)
		{
			this->stream = new Poco::FileInputStream(this->filename);
		}
		else if(mode == FileStream::MODE_APPEND)
		{
			this->stream = new Poco::FileOutputStream(this->filename, std::ios_base::out | std::ios_base::app);
		}
		else if(mode == FileStream::MODE_WRITE)
		{
			this->stream = new Poco::FileOutputStream(this->filename, std::ios_base::out | std::ios_base::trunc);
		}
		else
		{
			throw ValueException::FromString("Unknown file stream open mode given");
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
	if(! this->stream)
	{
		throw ValueException::FromString("FileStream must be opened before calling write");
	}

	try
	{
		std::string text = args.at(0)->ToString();

		Poco::FileOutputStream* fos = dynamic_cast<Poco::FileOutputStream*>(this->stream);
		if(! fos)
		{
			throw ValueException::FromString("FileStream must be opened for writing before calling write");
		}

		fos->write(text.c_str(), text.size());

		result->SetBool(true);
	}
	catch (Poco::Exception& exc)
	{
		throw ValueException::FromString(exc.displayText());
	}
}
void FileStream::Read(const ValueList& args, SharedValue result)
{
	if(! this->stream)
	{
		throw ValueException::FromString("FileStream must be opened before calling read");
	}

	try
	{
		std::string contents;

		Poco::FileInputStream* fis = dynamic_cast<Poco::FileInputStream*>(this->stream);
		if(! fis)
		{
			throw ValueException::FromString("FileStream must be opened for reading before calling read");
		}

		while(! fis->eof())
		{
			std::string s;
			std::getline(*fis, s);

			contents.append(s);
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
