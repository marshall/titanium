/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _FILE_BINDING_H_
#define _FILE_BINDING_H_

#include <api/module.h>
#include <api/binding/binding.h>
#include <map>
#include <vector>
#include <string>

namespace ti
{
	class FileBinding : public StaticBoundObject
	{
	public:
		FileBinding(BoundObject*);
	protected:
		virtual ~FileBinding();
	private:
		BoundObject *global;
		void IsFile(const ValueList& args, Value *result);
		void IsDirectory(const ValueList& args, Value *result);
		void IsHidden(const ValueList& args, Value *result);
	};
}

#endif
