/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _PROCESS_BINDING_H_
#define _PROCESS_BINDING_H_

#include <api/module.h>
#include <api/binding/binding.h>
#include <vector>

namespace ti
{
	class ProcessBinding : public StaticBoundObject
	{
	public:
		ProcessBinding(SharedBoundObject);
		virtual ~ProcessBinding();
		
		void Terminated(SharedBoundObject p);
		
	private:
		SharedBoundObject global;
		std::vector< SharedBoundObject > processes;
		
		void Launch(const ValueList& args, SharedValue result);
		void GetEnv(const ValueList& args, SharedValue result);
		void HasEnv(const ValueList& args, SharedValue result);
		void SetEnv(const ValueList& args, SharedValue result);
		
	};
}

#endif
