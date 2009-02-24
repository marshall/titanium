/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _USER_WINDOW_H_
#define _USER_WINDOW_H_

#include <string>
#include <vector>
#include <map>
#include <kroll/kroll.h>

#ifdef OS_WIN32
#undef CreateWindow
#endif

#include "../ti.App/app_config.h"

namespace ti {

typedef struct {
	double x;
	double y;
	double height;
	double width;
} Bounds;

enum UserWindowEvent
{
	FOCUSED,
	UNFOCUSED,
	OPENED,
	CLOSED,
	HIDDEN,
	SHOWN,
	FULLSCREENED,
	UNFULLSCREENED,
	MAXIMIZED,
	MINIMIZED,
	RESIZED,
	MOVED
};

class UserWindow : public kroll::StaticBoundObject {
	public:
		UserWindow(kroll::Host *host, WindowConfig *config);
		~UserWindow(){};
		kroll::Host* GetHost() { return this->host; }
		SharedBoundObject CreateWindow(WindowConfig *config);
		static std::vector<UserWindow*>& GetWindows();
		void UpdateWindowForURL(std::string url);

	private:
		void _Hide(const kroll::ValueList&, kroll::SharedValue);
		void _Show(const kroll::ValueList&, kroll::SharedValue);
		void _Focus(const kroll::ValueList&, kroll::SharedValue);
		void _Unfocus(const kroll::ValueList&, kroll::SharedValue);
		void _IsUsingChrome(const kroll::ValueList&, kroll::SharedValue);
		void _SetUsingChrome(const kroll::ValueList&, kroll::SharedValue);
		void _IsUsingScrollbars(const kroll::ValueList&, kroll::SharedValue);
		void _IsFullScreen(const kroll::ValueList&, kroll::SharedValue);
		void _SetFullScreen(const kroll::ValueList&, kroll::SharedValue);
		void _GetId(const kroll::ValueList&, kroll::SharedValue);

		void _Open(const kroll::ValueList&, kroll::SharedValue);
		void _Close(const kroll::ValueList&, kroll::SharedValue);

		void _GetX(const kroll::ValueList&, kroll::SharedValue);
		void _SetX(const kroll::ValueList&, kroll::SharedValue);

		void _GetY(const kroll::ValueList&, kroll::SharedValue);
		void _SetY(const kroll::ValueList&, kroll::SharedValue);

		void _GetWidth(const kroll::ValueList&, kroll::SharedValue);
		void _SetWidth(const kroll::ValueList&, kroll::SharedValue);
		void _GetMaxWidth(const kroll::ValueList&, kroll::SharedValue);
		void _SetMaxWidth(const kroll::ValueList&, kroll::SharedValue);
		void _GetMinWidth(const kroll::ValueList&, kroll::SharedValue);
		void _SetMinWidth(const kroll::ValueList&, kroll::SharedValue);

		void _GetHeight(const kroll::ValueList&, kroll::SharedValue);
		void _SetHeight(const kroll::ValueList&, kroll::SharedValue);
		void _GetMaxHeight(const kroll::ValueList&, kroll::SharedValue);
		void _SetMaxHeight(const kroll::ValueList&, kroll::SharedValue);
		void _GetMinHeight(const kroll::ValueList&, kroll::SharedValue);
		void _SetMinHeight(const kroll::ValueList&, kroll::SharedValue);

		void _GetBounds(const kroll::ValueList&, kroll::SharedValue);
		void _SetBounds(const kroll::ValueList&, kroll::SharedValue);

		void _GetTitle(const kroll::ValueList&, kroll::SharedValue);
		void _SetTitle(const kroll::ValueList&, kroll::SharedValue);

		void _GetURL(const kroll::ValueList&, kroll::SharedValue);
		void _SetURL(const kroll::ValueList&, kroll::SharedValue);

		void _IsResizable(const kroll::ValueList&, kroll::SharedValue);
		void _SetResizable(const kroll::ValueList&, kroll::SharedValue);

		void _IsMaximizable(const kroll::ValueList&, kroll::SharedValue);
		void _SetMaximizable(const kroll::ValueList&, kroll::SharedValue);

		void _IsMinimizable(const kroll::ValueList&, kroll::SharedValue);
		void _SetMinimizable(const kroll::ValueList&, kroll::SharedValue);

		void _IsCloseable(const kroll::ValueList&, kroll::SharedValue);
		void _SetCloseable(const kroll::ValueList&, kroll::SharedValue);

		void _IsVisible(const kroll::ValueList&, kroll::SharedValue);
		void _SetVisible(const kroll::ValueList&, kroll::SharedValue);

		void _GetTransparency(const kroll::ValueList&, kroll::SharedValue);
		void _SetTransparency(const kroll::ValueList&, kroll::SharedValue);

		void _GetMenu(const kroll::ValueList&, kroll::SharedValue);
		void _SetMenu(const kroll::ValueList&, kroll::SharedValue);

		void _GetContextMenu(const kroll::ValueList&, kroll::SharedValue);
		void _SetContextMenu(const kroll::ValueList&, kroll::SharedValue);

		void _GetIcon(const kroll::ValueList&, kroll::SharedValue);
		void _SetIcon(const kroll::ValueList&, kroll::SharedValue);

		void _GetParent(const kroll::ValueList&, kroll::SharedValue);
		void _CreateWindow(const kroll::ValueList&, kroll::SharedValue);
		void _OpenFiles(const ValueList& args, SharedValue result);

		void _AddEventListener(const kroll::ValueList&, kroll::SharedValue);
		void _RemoveEventListener(const kroll::ValueList&, kroll::SharedValue);

		void _IsTopMost(const kroll::ValueList&, kroll::SharedValue);
		void _SetTopMost(const kroll::ValueList&, kroll::SharedValue);

		struct Listener {
			SharedBoundMethod callback;
			long id;
		};
		std::vector<Listener> listeners;
		long next_listener_id;

	public:
		virtual UserWindow* WindowFactory(Host*, WindowConfig*) = 0;

		virtual void OpenFiles(
			SharedBoundMethod callback,
			bool multiple,
			bool files,
			bool directories,
			std::string& path,
			std::string& file,
			std::vector<std::string>& types) = 0;

		virtual void Hide() = 0;
		virtual void Show() = 0;
		virtual void Focus() = 0;
		virtual void Unfocus() = 0;
		virtual bool IsUsingChrome() = 0;
		virtual bool IsUsingScrollbars() = 0;
		virtual bool IsFullScreen() = 0;
		virtual std::string GetId() = 0;
		virtual void Open() = 0;
		virtual void Close();

		virtual double GetX() = 0;
		virtual void SetX(double x) = 0;
		virtual double GetY() = 0;
		virtual void SetY(double y) = 0;

		virtual double GetWidth() = 0;
		virtual void SetWidth(double width) = 0;
		virtual double GetMaxWidth() = 0;
		virtual void SetMaxWidth(double width) = 0;
		virtual double GetMinWidth() = 0;
		virtual void SetMinWidth(double width) = 0;

		virtual double GetHeight() = 0;
		virtual void SetHeight(double height) = 0;
		virtual double GetMaxHeight() = 0;
		virtual void SetMaxHeight(double height) = 0;
		virtual double GetMinHeight() = 0;
		virtual void SetMinHeight(double height) = 0;

		virtual Bounds GetBounds() = 0;
		virtual void SetBounds(Bounds bounds) = 0;
		virtual std::string GetTitle() = 0;
		virtual void SetTitle(std::string& title) = 0;
		virtual std::string GetURL() = 0;
		virtual void SetURL(std::string &url) = 0;
		virtual bool IsResizable() = 0;
		virtual void SetResizable(bool resizable) = 0;
		virtual bool IsMaximizable() = 0;
		virtual void SetMaximizable(bool maximizable) = 0;
		virtual bool IsMinimizable() = 0;
		virtual void SetMinimizable(bool minimizable) = 0;
		virtual bool IsCloseable() = 0;
		virtual void SetCloseable(bool closeable) = 0;
		virtual bool IsVisible() = 0;
		virtual void SetVisible(bool visible) = 0;
		virtual double GetTransparency() = 0;
		virtual void SetTransparency(double transparency) = 0;
		virtual void SetFullScreen(bool fullscreen) = 0;
		virtual void SetUsingChrome(bool chrome) = 0;
		virtual void SetMenu(SharedPtr<MenuItem> menu) = 0;
		virtual SharedPtr<MenuItem> GetMenu() = 0;
		virtual void SetContextMenu(SharedPtr<MenuItem> menu) = 0;
		virtual SharedPtr<MenuItem> GetContextMenu() = 0;
		virtual void SetIcon(SharedString icon_path) = 0;
		virtual SharedString GetIcon() = 0;
		virtual bool IsTopMost() = 0;
		virtual void SetTopMost(bool topmost) = 0;

		virtual void FireEvent(UserWindowEvent event);

	protected:
		kroll::Host *host;
		WindowConfig *config;
		UserWindow *parent;

		virtual UserWindow* GetParent();
		virtual void SetParent(UserWindow *parent);
		virtual void AddChild(UserWindow *window);
		virtual void RemoveChild(UserWindow *window);

		static std::vector<UserWindow*> windows;
		static std::map<UserWindow*, std::vector<UserWindow*> > windowsMap;

		static void Open(UserWindow *);
		static void AddChild(UserWindow *parent, UserWindow *child);
		static void RemoveChild(UserWindow *parent, UserWindow *child);

	private:
		DISALLOW_EVIL_CONSTRUCTORS(UserWindow);
};

}
#endif
