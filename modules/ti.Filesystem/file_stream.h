/*
 * file_stream.h
 *
 *  Created on: Feb 15, 2009
 *      Author: jorge
 */

#ifndef FILE_STREAM_H_
#define FILE_STREAM_H_

#ifdef OS_WIN32
#include <api/base.h>
#include <string>
#elif OS_OSX
#import <Foundation/Foundation.h>
#endif


#include <api/binding/binding.h>
#include <api/binding/static_bound_list.h>
#include <string>
#include <Poco/FileStream.h>

namespace ti {

class FileStream : public StaticBoundObject {
public:
	FileStream(std::string filename_);
	virtual ~FileStream();

	static std::string MODE_READ;
	static std::string MODE_WRITE;
	static std::string MODE_APPEND;

	// TODO - these functions are public for now, but need to be made private when the corresponding functions
	//        in File are deleted
	/**
	 * Function: Open
	 *   Opens this file stream for reading or writing
	 *
	 * Parameters:
	 *   mode - string describing abilities of the file stream (reading/writing/etc)
	 *
	 * Returns:
	 *   true if the file stream is opened successfully; false otherwise
	 */
	void Open(const ValueList& args, SharedValue result);
	bool Open(std::string mode);
	/**
	 * Function: Close
	 *   Closes this file stream.  No further reads/writes can be performed on this file stream unless Open is called
	 *
	 * Parameters:
	 *
	 * Returns:
	 *   true if the file stream is closed successfully; false otherwise
	 */
	void Close(const ValueList& args, SharedValue result);
	bool Close();
	/**
	 * Function: Write
	 *   writes a string of text to a file
	 *
	 *   Typical usage is:
	 *     var fileStream = Titanium.Filesystem.getFileStream(filename);
	 *     fileStream.open(Titanium.Filesystem.FILESTREAM_MODE_READ);
	 *     var stringToWrite = "some text ... ";
	 *     var written = file.write(stringToWrite);
	 *     fileStream.close();
	 *
	 * Parameters:
	 *   text = text to write to file
	 *   append - boolean flag that indicates if the text should be appended to the file (if the file exists)
	 *
	 * Returns:
	 *   true if the text was written successfully; false otherwise
	 */
	void Write(const ValueList& args, SharedValue result);
	/**
	 * Function: Read
	 *   Reads the contents of this File
	 *
	 *   Typical usage is:
	 *     var fileStream = Titanium.Filesystem.getFileStream(filename);
	 *     fileStream.open(Titanium.Filesystem.FILESTREAM_MODE_READ);
	 *     var contents = file.read();
	 *     fileStream.close();
	 *
	 * Parameters:
	 *
	 * Returns:
	 *   the text file contents as a string
	 */
	void Read(const ValueList& args, SharedValue result);
	/**
	 * Function: ReadLine
	 *   Reads the next line of this File.
	 *   The file must be open for reading the first time this method is called.
	 *
	 *   Typical usage is:
	 *     var fileStream = Titanium.Filesystem.getFileStream(filename);
	 *     fileStream.open(Titanium.Filesystem.FILESTREAM_MODE_READ);
	 *     var line = file.readLine();
	 *     while(line != null) {
	 *       // do something with the line
	 *       line = file.readLine();
	 *     }
	 *     fileStream.close();
	 *
	 * Parameters:
	 *
	 * Returns:
	 *   a string representing the next line of text in the file.
	 */
	void ReadLine(const ValueList& args, SharedValue result);

private:
	std::string filename;
	Poco::FileIOS* stream;
};

}

#endif /* FILE_STREAM_H_ */
