/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */
#ifndef _GTK_USER_WINDOW_H_
#define _GTK_USER_WINDOW_H_

#include "../ui_module.h"

namespace ti
{
	class GtkUserWindow : public UserWindow {
	public:
		GtkUserWindow(Host *host, ti::WindowConfig *config);
		~GtkUserWindow();
		void SetupDecorations();
		void SetupTransparency();

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
		double GetHeight();
		void SetHeight(double height);
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
		void SetMenu(SharedBoundList menu);


	protected:
		GtkWindow* gtk_window;
		GtkWidget* vbox;
		WebKitWebView* web_view;
		GtkMenuWrapper* menu_wrapper;
	};
}


#endif
