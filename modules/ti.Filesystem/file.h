/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _TI_FILE_H_
#define _TI_FILE_H_

#include <api/binding/binding.h>
#include <api/binding/static_bound_list.h>
#include <string>

namespace ti
{
	class File : public StaticBoundObject
	{
		public:
			File(std::string filename);
		protected:
			virtual ~File();
		private:
			std::string filename;

			/**
			 * Function: ToString
			 *   Returns the File name
			 *
			 * Parameters:
			 *
			 * Returns:
			 *   the file name
			 */
			void ToString(const ValueList& args, SharedValue result);
			/**
			 * Function: IsFile
			 *   Determines if this File represents a file
			 *
			 * Parameters:
			 *
			 * Returns:
			 *   true if this File repesents a file; false otherwise
			 */
			void IsFile(const ValueList& args, SharedValue result);
			/**
			 * Function: IsDirectory
			 *   Determines if this File represents a directory
			 *
			 * Parameters:
			 *
			 * Returns:
			 *   true if this File represents a directory; false otherwise
			 */
			void IsDirectory(const ValueList& args, SharedValue result);
			/**
			 * Function: IsHidden
			 *   Determines if this File represents a hidden file or directory
			 *
			 * Parameters:
			 *
			 * Returns:
			 *   true if this File is hidden; false otherwise
			 */
			void IsHidden(const ValueList& args, SharedValue result);
			/**
			 * Function: IsSymbolicLink
			 *   Determines if this File represents a symbolic link
			 *
			 * Parameters:
			 *
			 * Returns:
			 *   true if this File is a symbolic link; false otherwise
			 */
			void IsSymbolicLink(const ValueList& args, SharedValue result);
			/**
			 * Function: Read
			 *   Reads the contents of this File
			 *
			 * Parameters:
			 *
			 * Returns:
			 *   the text file contents as a string
			 */
			void Read(const ValueList& args, SharedValue result);
			/**
			 * Function: Copy
			 *   Copies this file or directory to a given destination
			 *
			 *   If this File is a file, the file is copied to the destination specified
			 *   If this File is a directory, the directory contents is recursively copied to the destination
			 *
			 * Parameters:
			 *   destination - the destination where the file or directory is copied to
			 *
			 * Returns:
			 *   true if the copy operation is successful; false otherwise
			 */
			void Copy(const ValueList& args, SharedValue result);
			/**
			 * Function: Move
			 *   Moves this file or directory to the given destination
			 *
			 * Parameters:
			 *   destination - the destination where the file or directory is moved to
			 *
			 * Returns:
			 *   true if the move operation is successful; false otherwise
			 */
			void Move(const ValueList& args, SharedValue result);
			/**
			 * Function: CreateDirectory
			 *   Creates this directory
			 *
			 * Parameters:
			 *   recursive [= false] - controls if all parent directories are created if needed
			 *
			 * Returns:
			 *   true if the directory is created; false otherwise
			 */
			void CreateDirectoryX(const ValueList& args, SharedValue result);
			/**
			 * Function: DeleteDirectory
			 *   Deletes this directory
			 *
			 * Parameters:
			 *   deleteContents [= false] - controls if the contents inside the directory are also deleted
			 *
			 * Returns:
			 *   true if the directory is deleted; false otherwise
			 */
			void DeleteDirectory(const ValueList& args, SharedValue result);
			/**
			 * Function: DeleteFile
			 *   Deletes this file
			 *
			 * Parameters:
			 *
			 * Returns:
			 *   true if the file is deleted; false otherwise
			 */
			void DeleteFileX(const ValueList& args, SharedValue result);
			/**
			 * Function: GetDirectoryListing
			 *   Gets a list of files/directories under this directory
			 *
			 * Parameters:
			 *
			 * Returns:
			 *   An array of file/directory names under this directory.
			 */
			void GetDirectoryListing(const ValueList& args, SharedValue result);
			/**
			 * Function: GetParent
			 *   returns the parent for this file or directory
			 *
			 * Parameters:
			 *
			 * Returns:
			 *   the parent for this file
			 */
			void GetParent(const ValueList& args, SharedValue result);
			/**
			 * Function: GetExists
			 *   Determines if this File represents an existing file or directory
			 *
			 * Parameters:
			 *
			 * Returns:
			 *   true if this File exists; false otherwise
			 */
			void GetExists(const ValueList& args, SharedValue result);
			/**
			 * Function: GetCreateDate
			 *   Returns the date this file or directory was created
			 *
			 * Parameters:
			 *
			 * Returns:
			 *   timestamp when the file/directory was created; null is returned if the timestamp can't be determined
			 */
			void GetCreateTimestamp(const ValueList& args, SharedValue result);
			/**
			 * Function: GetModificationDate
			 *   Returns the date this file or directory was modified
			 *
			 * Parameters:
			 *
			 * Returns:
			 *   timestamp when the file/directory was modified; null is returned if the timestamp can't be determined
			 */
			void GetModificationTimestamp(const ValueList& args, SharedValue result);
			/**
			 * Function: GetName
			 *   Returns the file name
			 *
			 * Parameters:
			 *
			 * Returns:
			 *   the file name
			 */
			void GetName(const ValueList& args, SharedValue result);
			/**
			 * Function: GetExctension
			 *   returns the file extension
			 *
			 * Parameters:
			 *
			 * Returns:
			 *   the file extension; null if this is an invalid file
			 */
			void GetExtension(const ValueList& args, SharedValue result);
			/**
			 * Function: GetNativePath
			 *   returns the native path for this file or directory
			 *
			 * Parameters:
			 *
			 * Returns:
			 *   the native path
			 */
			void GetNativePath(const ValueList& args, SharedValue result);
			/**
			 * Function: GetSize
			 *   return the size of this file in bytes
			 *
			 * Parameters:
			 *
			 * Returns:
			 *   the file size in bytes
			 */
			void GetSize(const ValueList& args, SharedValue result);
	};
}

#endif /* FILE_H_ */
