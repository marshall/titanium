/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _APP_BINDING_H_
#define _APP_BINDING_H_

#include <kroll/kroll.h>


namespace ti
{
	class AppBinding : public StaticBoundObject
	{
	public:
		AppBinding(Host *host,SharedBoundObject);
		virtual ~AppBinding();
	private:
		Host *host;
		SharedBoundObject global;
		void GetID(const ValueList& args, SharedValue result);
		void GetName(const ValueList& args, SharedValue result);
		void GetVersion(const ValueList& args, SharedValue result);
		void GetUpdateURL(const ValueList& args, SharedValue result);
		void GetGUID(const ValueList& args, SharedValue result);
		void AppURLToPath(const ValueList& args, SharedValue result);
		void SetMenu(const ValueList& args, SharedValue result);
		void Exit(const ValueList& args, SharedValue result);

		void LoadProperties(const ValueList& args, SharedValue result);
	};
}

#endif
