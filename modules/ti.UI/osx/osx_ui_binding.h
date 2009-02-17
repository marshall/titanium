/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
 */


#ifndef _OSX_UI_BINDING_H_
#define _OSX_UI_BINDING_H_

#include "../ui_module.h"

namespace ti
{
	class OSXUIBinding : public UIBinding
	{

		public:
		OSXUIBinding(Host *host);
		~OSXUIBinding();

		SharedPtr<MenuItem> CreateMenu(bool trayMenu);
		void SetMenu(SharedPtr<MenuItem>);
		void SetContextMenu(SharedPtr<MenuItem>);
		void SetIcon(SharedString icon_path);
		SharedPtr<TrayItem> AddTray(SharedString icon_path,
		                            SharedBoundMethod cb);

		virtual void SetDockIcon(SharedString icon_path);
		virtual void SetDockMenu(SharedPtr<MenuItem>);
		virtual void SetBadge(SharedString badge_label);
		virtual void SetBadgeImage(SharedString badge_path);

		long GetIdleTime();
	private:
		NSView *savedDockView;
	};
}

#endif
