/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#ifndef _USER_WINDOW_H_
#define _USER_WINDOW_H_

#include <string>
#include <vector>
#include <kroll/kroll.h>

#include "../ti.App/app_config.h"

namespace ti {

typedef struct {
	double x;
	double y;
	double height;
	double width;
} Bounds;

class UserWindow : public kroll::StaticBoundObject {
	public:
		UserWindow(kroll::Host *host, WindowConfig *config);
	protected:
		~UserWindow(){};
	public:
		kroll::Host* GetHost() { return this->host; }

		void hide_cb(const kroll::ValueList&, kroll::Value *, kroll::BoundObject*);
		void show_cb(const kroll::ValueList&, kroll::Value *, kroll::BoundObject*);
		void is_using_chrome_cb(const kroll::ValueList&, kroll::Value *, kroll::BoundObject*);
		void is_using_scrollbars_cb(const kroll::ValueList&, kroll::Value *, kroll::BoundObject*);
		void is_full_screen_cb(const kroll::ValueList&, kroll::Value *, kroll::BoundObject*);
		void get_id_cb(const kroll::ValueList&, kroll::Value *, kroll::BoundObject*);
		void open_cb(const kroll::ValueList&, kroll::Value *, kroll::BoundObject*);
		void close_cb(const kroll::ValueList&, kroll::Value *, kroll::BoundObject*);
		void get_x_cb(const kroll::ValueList&, kroll::Value *, kroll::BoundObject*);
		void set_x_cb(const kroll::ValueList&, kroll::Value *, kroll::BoundObject*);
		void get_y_cb(const kroll::ValueList&, kroll::Value *, kroll::BoundObject*);
		void set_y_cb(const kroll::ValueList&, kroll::Value *, kroll::BoundObject*);
		void get_width_cb(const kroll::ValueList&, kroll::Value *, kroll::BoundObject*);
		void set_width_cb(const kroll::ValueList&, kroll::Value *, kroll::BoundObject*);
		void get_height_cb(const kroll::ValueList&, kroll::Value *, kroll::BoundObject*);
		void set_height_cb(const kroll::ValueList&, kroll::Value *, kroll::BoundObject*);
		void get_bounds_cb(const kroll::ValueList&, kroll::Value *, kroll::BoundObject*);
		void set_bounds_cb(const kroll::ValueList&, kroll::Value *, kroll::BoundObject*);
		void get_title_cb(const kroll::ValueList&, kroll::Value *, kroll::BoundObject*);
		void set_title_cb(const kroll::ValueList&, kroll::Value *, kroll::BoundObject*);
		void get_url_cb(const kroll::ValueList&, kroll::Value *, kroll::BoundObject*);
		void set_url_cb(const kroll::ValueList&, kroll::Value *, kroll::BoundObject*);
		void is_resizable_cb(const kroll::ValueList&, kroll::Value *, kroll::BoundObject*);
		void set_resizable_cb(const kroll::ValueList&, kroll::Value *, kroll::BoundObject*);
		void is_maximizable_cb(const kroll::ValueList&, kroll::Value *, kroll::BoundObject*);
		void set_maximizable_cb(const kroll::ValueList&, kroll::Value *, kroll::BoundObject*);
		void is_minimizable_cb(const kroll::ValueList&, kroll::Value *, kroll::BoundObject*);
		void set_minimizable_cb(const kroll::ValueList&, kroll::Value *, kroll::BoundObject*);
		void is_closeable_cb(const kroll::ValueList&, kroll::Value *, kroll::BoundObject*);
		void set_closeable_cb(const kroll::ValueList&, kroll::Value *, kroll::BoundObject*);
		void is_visible_cb(const kroll::ValueList&, kroll::Value *, kroll::BoundObject*);
		void set_visible_cb(const kroll::ValueList&, kroll::Value *, kroll::BoundObject*);
		void get_transparency_cb(const kroll::ValueList&, kroll::Value *, kroll::BoundObject*);
		void set_transparency_cb(const kroll::ValueList&, kroll::Value *, kroll::BoundObject*);

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
		virtual Bounds GetBounds() = 0;
		virtual void SetBounds(Bounds bounds) = 0;
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
		kroll::Host *host;
		WindowConfig *config;

		static std::vector<UserWindow*> windows;
		static void Open(UserWindow *);
		static void Close(UserWindow *);
		
	private:
		DISALLOW_EVIL_CONSTRUCTORS(UserWindow);
};

}
#endif

