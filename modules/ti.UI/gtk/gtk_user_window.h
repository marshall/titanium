/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _GTK_USER_WINDOW_H_
#define _GTK_USER_WINDOW_H_

namespace ti
{
	class GtkUserWindow : public UserWindow
	{

		public:
		GtkUserWindow(SharedUIBinding, WindowConfig*, SharedUserWindow&);
		virtual ~GtkUserWindow();
		void Destroyed();
		void SetupDecorations();
		void SetupTransparency();
		void SetupSizeLimits();
		void SetupSize();
		void SetupPosition();
		void SetupMenu();
		void SetupIcon();
		void AppMenuChanged();
		void AppIconChanged();
		void RemoveOldMenu();

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
		void Focus();
		void Unfocus();
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
		void SetWidth(double width) ;
		double GetMaxWidth();
		void SetMaxWidth(double width);
		double GetMinWidth();
		void SetMinWidth(double width);

		double GetHeight();
		void SetHeight(double height);
		double GetMaxHeight();
		void SetMaxHeight(double height);
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

		int gdk_width;
		int gdk_height;
		int gdk_x;
		int gdk_y;

	protected:
		GtkWindow* gtk_window;
		GtkWidget* vbox;
		WebKitWebView* web_view;
		bool topmost;
		gulong destroy_cb_id;

		// The window-specific menu.
		SharedPtr<GtkMenuItemImpl> menu;
		/*
		 * The menu this window is using. This
		 * might just be a copy of the app menu.
		 */
		SharedPtr<GtkMenuItemImpl> menu_in_use;
		// The widget this window uses for a menu.
		GtkWidget* menu_bar;
		// The path to this window's icon
		SharedString icon_path;
		// The widget this window uses for a context menu.
		SharedPtr<GtkMenuItemImpl> context_menu;

		void _OpenFilesWork(const ValueList&, SharedValue);
		static std::string openFilesDirectory;

	};

}


#endif
