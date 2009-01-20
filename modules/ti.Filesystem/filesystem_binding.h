/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _FILESYSTEM_BINDING_H_
#define _FILESYSTEM_BINDING_H_

#include <api/module.h>
#include <api/binding/binding.h>

namespace ti
{
	class FilesystemBinding : public StaticBoundObject
	{
	public:
		FilesystemBinding(BoundObject*);
	protected:
		virtual ~FilesystemBinding();
	private:
		BoundObject *global;

		/**
		 * Function: CreateTempFile
		 *   creates a new temporary file
		 *
		 * Params:
		 *
		 * Returns:
		 *   the filename for the new temporary file
		 */
		void CreateTempFile(const ValueList& args, SharedValue result);
		/**
		 * Function: CreateTempDirectory
		 *   creates a new temporary directory
		 *
		 * Params:
		 *
		 * Returns:
		 *   the directory name for the new temporary directory
		 */
		void CreateTempDirectory(const ValueList& args, SharedValue result);
		/**
		 * Function: GetFile
		 *   creates a new File object for a given filename
		 *
		 * Params:
		 *   filename - the name of the file to create the File object for
		 *
		 * Returns:
		 *   a new File object
		 */
		void GetFile(const ValueList& args, SharedValue result);
		/**
		 * Function: GetApplicationDirectory
		 *   returns a path to the application directory (path to Resources folder)
		 *
		 * Params:
		 *
		 * Returns:
		 *   path to the Resources folder
		 */
		void GetApplicationDirectory(const ValueList& args, SharedValue result);
		/**
		 * Function: GetDesktopDirectory
		 *   return the path to the user's desktop directory
		 *
		 * Params:
		 *
		 * Returns:
		 *   path to the user's Desktop folder
		 */
		void GetDesktopDirectory(const ValueList& args, SharedValue result);
		/**
		 * Function: GetDocumentsDirectory
		 *   return the path to the user's documents directory
		 *
		 * Params:
		 *
		 * Returns:
		 *   path to the user's document's folder
		 */
		void GetDocumentsDirectory(const ValueList& args, SharedValue result);
		/**
		 * Function: GetUserDirectory
		 *   return the path to the user's home directory
		 *
		 * Params:
		 *
		 * Returns:
		 *   path to the user's home directory
		 */
		void GetUserDirectory(const ValueList& args, SharedValue result);
		/**
		 * Function: GetLineEnding
		 *   return a string that represents the line ending
		 *
		 * Params:
		 *
		 * Returns:
		 *  line ending as a string
		 */
		void GetLineEnding(const ValueList& args, SharedValue result);
		/**
		 * Function: GetSeparator
		 *   gets the separator used in file paths
		 *
		 * Params:
		 *
		 * Returns:
		 *   separator used in file paths
		 */
		void GetSeparator(const ValueList& args, SharedValue result);
		/**
		 * Function: GetRootDirectories
		 *   gets a list of strings that represent the root directories
		 *
		 * Params:
		 *
		 * Returns:
		 *   a list of strings
		 */
		void GetRootDirectories(const ValueList& args, SharedValue result);
	};
}

#endif
