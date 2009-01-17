/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _TI_FILE_H_
#define _TI_FILE_H_

#include <api/binding/binding.h>
#include <string>

namespace ti
{
	class File : public StaticBoundObject
	{
		public:
			File(BoundObject *global, std::string filename);
		protected:
			virtual ~File();
		private:
			BoundObject *global;
			std::string filename;

			void IsFile(const ValueList& args, SharedValue result);
			void IsDirectory(const ValueList& args, SharedValue result);
			void IsHidden(const ValueList& args, SharedValue result);
			void ToString(const ValueList& args, SharedValue result);
	};
}

#endif /* FILE_H_ */
