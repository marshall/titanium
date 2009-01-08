/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef __TI_USER_WINDOW_H__
#define __TI_USER_WINDOW_H__

#include <string>
#include <vector>

typedef struct {
	double x;
	double y;
	double height;
	double width;
} TiBounds;

class TiUserWindow : public TiStaticBoundObject {
	public:
		TiUserWindow(TiHost *host, TiWindowConfig *config);
		TiHost* GetHost() { return this->host; }

		void hide_cb(const TiValueList&, TiValue *, TiBoundObject*);
		void show_cb(const TiValueList&, TiValue *, TiBoundObject*);
		void is_using_chrome_cb(const TiValueList&, TiValue *, TiBoundObject*);
		void is_using_scrollbars_cb(const TiValueList&, TiValue *, TiBoundObject*);
		void is_full_screen_cb(const TiValueList&, TiValue *, TiBoundObject*);
		void get_id_cb(const TiValueList&, TiValue *, TiBoundObject*);
		void open_cb(const TiValueList&, TiValue *, TiBoundObject*);
		void close_cb(const TiValueList&, TiValue *, TiBoundObject*);
		void get_x_cb(const TiValueList&, TiValue *, TiBoundObject*);
		void set_x_cb(const TiValueList&, TiValue *, TiBoundObject*);
		void get_y_cb(const TiValueList&, TiValue *, TiBoundObject*);
		void set_y_cb(const TiValueList&, TiValue *, TiBoundObject*);
		void get_width_cb(const TiValueList&, TiValue *, TiBoundObject*);
		void set_width_cb(const TiValueList&, TiValue *, TiBoundObject*);
		void get_height_cb(const TiValueList&, TiValue *, TiBoundObject*);
		void set_height_cb(const TiValueList&, TiValue *, TiBoundObject*);
		void get_bounds_cb(const TiValueList&, TiValue *, TiBoundObject*);
		void set_bounds_cb(const TiValueList&, TiValue *, TiBoundObject*);
		void get_title_cb(const TiValueList&, TiValue *, TiBoundObject*);
		void set_title_cb(const TiValueList&, TiValue *, TiBoundObject*);
		void get_url_cb(const TiValueList&, TiValue *, TiBoundObject*);
		void set_url_cb(const TiValueList&, TiValue *, TiBoundObject*);
		void is_resizable_cb(const TiValueList&, TiValue *, TiBoundObject*);
		void set_resizable_cb(const TiValueList&, TiValue *, TiBoundObject*);
		void is_maximizable_cb(const TiValueList&, TiValue *, TiBoundObject*);
		void set_maximizable_cb(const TiValueList&, TiValue *, TiBoundObject*);
		void is_minimizable_cb(const TiValueList&, TiValue *, TiBoundObject*);
		void set_minimizable_cb(const TiValueList&, TiValue *, TiBoundObject*);
		void is_closeable_cb(const TiValueList&, TiValue *, TiBoundObject*);
		void set_closeable_cb(const TiValueList&, TiValue *, TiBoundObject*);
		void is_visible_cb(const TiValueList&, TiValue *, TiBoundObject*);
		void set_visible_cb(const TiValueList&, TiValue *, TiBoundObject*);
		void get_transparency_cb(const TiValueList&, TiValue *, TiBoundObject*);
		void set_transparency_cb(const TiValueList&, TiValue *, TiBoundObject*);

		virtual void Hide() = 0;
		virtual void Show() = 0;
		virtual bool IsUsingChrome() = 0;
		virtual bool IsUsingScrollbars() = 0;
		virtual bool IsFullScreen() = 0;
		virtual std::string GetId() = 0;
		virtual void Open() = 0;
		virtual void Close() = 0;
		virtual double GetX() = 0;
		virtual void SetX(double x) = 0;
		virtual double GetY() = 0;
		virtual void SetY(double y) = 0;
		virtual double GetWidth() = 0;
		virtual void SetWidth(double width) = 0;
		virtual double GetHeight() = 0;
		virtual void SetHeight(double height) = 0;
		virtual TiBounds GetBounds() = 0;
		virtual void SetBounds(TiBounds bounds) = 0;
		virtual std::string GetTitle() = 0;
		virtual void SetTitle(std::string title) = 0;
		virtual std::string GetUrl() = 0;
		virtual void SetUrl(std::string url) = 0;
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

	protected:
		TiHost *host;
		TiWindowConfig *config;

		static std::vector<TiUserWindow*> windows;
		static void Open(TiUserWindow *);
		static void Close(TiUserWindow *);
};

#endif

