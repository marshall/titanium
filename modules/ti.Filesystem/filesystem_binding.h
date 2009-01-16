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
		void CreateTempFile(const ValueList& args, Value *result);
		void CreateTempDirectory(const ValueList& args, Value *result);
		void GetFile(const ValueList& args, Value *result);
	};
}

#endif
