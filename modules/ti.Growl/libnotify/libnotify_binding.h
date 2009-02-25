/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */


#ifndef _LIBNOTIFY_BINDING_H_
#define _LIBNOTIFY_BINDING_H_

#include <api/module.h>
#include <api/binding/binding.h>

using kroll::SharedBoundObject;

namespace ti
{
	class LibNotifyBinding : public GrowlBinding
	{
	public:
		LibNotifyBinding(SharedBoundObject);
		~LibNotifyBinding();

	protected:
		bool IsRunning();
		void ShowNotification(
			std::string& title,
			std::string& description,
			std::string& iconURL,
			int notification_delay,
			SharedBoundMethod callback);

		std::string GetAppName();
		SharedString GetResourcePath(const char *URL);
	};
}

#endif
