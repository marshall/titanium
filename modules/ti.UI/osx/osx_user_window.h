/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#import "preinclude.h"
#import <WebKit/WebKit.h>
#import "../menu_item.h"
#import "../user_window.h"
#import "native_window.h"

namespace ti
{
	class OSXUserWindow : public UserWindow
	{
		public:
			OSXUserWindow(Host *host, WindowConfig *config);
			~OSXUserWindow();
		public:
			UserWindow* WindowFactory(Host*, WindowConfig*);
			void OpenFiles(
				SharedBoundMethod callback,
				bool multiple,
				bool files,
				bool directories,
				std::string& path,
				std::string& file,
				std::vector<std::string>& types);
			void Hide();
			void Show();
			bool IsUsingChrome();
			void SetUsingChrome(bool chrome);
			bool IsUsingScrollbars();
			bool IsFullScreen();
			std::string GetId();
			void Open();
			void Close();
			double GetX();
			void SetX(double x);
			double GetY();
			void SetY(double y);
			double GetWidth();
			void SetWidth(double width);
			double GetMaxWidth();
			void SetMaxWidth(double width);
			double GetMinWidth();
			void SetMinWidth(double width);
			double GetHeight();
			void SetHeight(double height);
			double GetMaxHeight();
			void SetMaxHeight(double height);
			void ReconfigureWindowConstraints();
			double GetMinHeight();
			void SetMinHeight(double height);
			Bounds GetBounds();
			void SetBounds(Bounds bounds);
			std::string GetTitle();
			void SetTitle(std::string& title);
			std::string GetURL();
			void SetURL(std::string& url);
			bool IsResizable();
			void SetResizable(bool resizable);
			bool IsMaximizable();
			void SetMaximizable(bool maximizable);
			bool IsMinimizable();
			void SetMinimizable(bool minimizable);
			bool IsCloseable();
			void SetCloseable(bool closeable);
			bool IsVisible();
			void SetVisible(bool visible);
			double GetTransparency();
			void SetTransparency(double transparency);
			void SetFullScreen(bool fullscreen);
			bool IsTopMost();
			void SetTopMost(bool topmost);

			void SetMenu(SharedPtr<MenuItem> menu);
			SharedPtr<MenuItem> GetMenu();
			void SetContextMenu(SharedPtr<MenuItem> menu);
			SharedPtr<MenuItem> GetContextMenu();
			void SetIcon(SharedString icon_path);
			SharedString GetIcon();

			NativeWindow* GetNative() { return window; }
			void Focused();
			void Unfocused();

		private:
			NativeWindow *window;
			bool opened;
			bool closed;
			bool topmost;
			SharedPtr<MenuItem> menu;
			SharedPtr<MenuItem> context_menu;
			NSMenu *native_menu;

			DISALLOW_EVIL_CONSTRUCTORS(OSXUserWindow);
	};
}
