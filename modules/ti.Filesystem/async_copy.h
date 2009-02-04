/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _TI_ASYNC_COPY_H
#define _TI_ASYNC_COPY_H

#ifdef OS_WIN32
#include <api/base.h>
#include <windows.h>
#elif OS_OSX
#import <Foundation/Foundation.h>
#endif

#include <api/binding/binding.h>
#include <api/binding/static_bound_list.h>
#include <string>
#include <vector>
#include <Poco/Thread.h>
#include <Poco/Exception.h>
#include <Poco/File.h>


namespace ti
{
	class AsyncCopy : public StaticBoundObject
	{
		public:
			AsyncCopy(kroll::Host *host,std::vector<std::string> files, std::string destination, SharedBoundMethod callback);
			virtual ~AsyncCopy();

		private:
			Host *host;
			std::vector<std::string> files;
			std::string destination;
			SharedBoundMethod callback;
			Poco::Thread *thread;
			bool stopped;
			
			static void Run(void*);

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
			void Cancel(const ValueList& args, SharedValue result);
	};
}

#endif
