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

FileStream::FileStream(std::string filename_) : filename(filename_), stream(NULL) {
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

		if(this->stream->eof())
		{
			// close the file
			result->SetNull();
		}
		else
		{
			std::string line;
			Poco::FileInputStream* fis = dynamic_cast<Poco::FileInputStream*>(this->stream);
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
