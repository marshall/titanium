#ifndef __TI_TYPES_H
#define __TI_TYPES_H

#include <string>
#include <JavaScriptCore/JSObjectRef.h>
#include <JavaScriptCore/JSStringRef.h>
#include <JavaScriptCore/JSContextRef.h>
#include "js_wrapper.h"


typedef struct {
    double x;
    double y;
    double height;
    double width;
} TiBounds;

class TiUserWindow : public TiObject {
    public:
        TiUserWindow() {}
        TiUserWindow(JSContextRef, JSObjectRef);

        void WindowObjectCleared(JSContextRef, JSObjectRef);

        virtual void Hide();
        TiValue hide_cb(size_t num_args, TiValue args[]);

        virtual void Show();
        TiValue show_cb(size_t num_args, TiValue args[]);

        virtual bool IsUsingChrome();
        TiValue is_using_chrome_cb(size_t num_args, TiValue args[]);

        virtual bool IsUsingScrollbars();
        TiValue is_using_scrollbars_cb(size_t num_args, TiValue args[]);

        virtual bool IsFullScreen();
        TiValue is_full_screen_cb(size_t num_args, TiValue args[]);

        virtual std::string GetId();
        TiValue get_id_cb(size_t num_args, TiValue args[]);

        virtual void Open();
        TiValue open_cb(size_t num_args, TiValue args[]);
        virtual void Close();
        TiValue close_cb(size_t num_args, TiValue args[]);

        virtual double GetX();
        TiValue get_x_cb(size_t num_args, TiValue args[]);
        virtual void SetX(double x);
        TiValue set_x_cb(size_t num_args, TiValue args[]);

        virtual double GetY();
        TiValue get_y_cb(size_t num_args, TiValue args[]);
        virtual void SetY(double y);
        TiValue set_y_cb(size_t num_args, TiValue args[]);

        virtual double GetWidth();
        TiValue get_width_cb(size_t num_args, TiValue args[]);
        virtual void SetWidth(double width);
        TiValue set_width_cb(size_t num_args, TiValue args[]);

        virtual double GetHeight();
        TiValue get_height_cb(size_t num_args, TiValue args[]);
        virtual void SetHeight(double height);
        TiValue set_height_cb(size_t num_args, TiValue args[]);

        virtual TiBounds GetBounds();
        TiValue get_bounds_cb(size_t num_args, TiValue args[]);
        virtual void SetBounds(TiBounds bounds);
        TiValue set_bounds_cb(size_t num_args, TiValue args[]);

        virtual std::string GetTitle();
        TiValue get_title_cb(size_t num_args, TiValue args[]);
        virtual void SetTitle(std::string title);
        TiValue set_title_cb(size_t num_args, TiValue args[]);

        virtual std::string GetUrl();
        TiValue get_url_cb(size_t num_args, TiValue args[]);
        virtual void SetUrl(std::string url);
        TiValue set_url_cb(size_t num_args, TiValue args[]);

        virtual bool IsResizable();
        TiValue is_resizable_cb(size_t num_args, TiValue args[]);
        virtual void SetResizable(bool resizable);
        TiValue set_resizable_cb(size_t num_args, TiValue args[]);

        virtual bool IsMaximizable();
        TiValue is_maximizable_cb(size_t num_args, TiValue args[]);
        virtual void SetMaximizable(bool maximizable);
        TiValue set_maximizable_cb(size_t num_args, TiValue args[]);

        virtual bool IsMinimizable();
        TiValue is_minimizable_cb(size_t num_args, TiValue args[]);
        virtual void SetMinimizable(bool minimizable);
        TiValue set_minimizable_cb(size_t num_args, TiValue args[]);

        virtual bool IsCloseable();
        TiValue is_closeable_cb(size_t num_args, TiValue args[]);
        virtual void SetCloseable(bool closeable);
        TiValue set_closeable_cb(size_t num_args, TiValue args[]);

        virtual bool IsVisible();
        TiValue is_visible_cb(size_t num_args, TiValue args[]);
        virtual void SetVisible(bool visible);
        TiValue set_visible_cb(size_t num_args, TiValue args[]);

        virtual double GetTransparency();
        TiValue get_transparency_cb(size_t num_args, TiValue args[]);
        virtual void SetTransparency(double transparency);
        TiValue set_transparency_cb(size_t num_args, TiValue args[]);

    protected:
        TiObject global;
};

class TiApiPoint : public TiObject {
    public:
        TiApiPoint(TiUserWindow* window);
        TiUserWindow* GetWindow();

    protected:
        TiUserWindow* window;

};

class TiRuntime : public TiApiPoint {
    public:
        TiRuntime(TiUserWindow* window);

    protected:
};

class TiApp : public TiApiPoint {
    public:
        TiApp(TiUserWindow* window);
        TiValue Debug(size_t num_args, TiValue args[]);
};

class TiFilesystem : public TiApiPoint {
    public:
        TiFilesystem(TiUserWindow* window);
};

class TiFile : public TiApiPoint {
    public:
        TiFile(TiUserWindow* window);
};

class TiDesktop : public TiApiPoint {
    public:
        TiDesktop(TiUserWindow* window);
};

class TiMenu : public TiApiPoint {
    public:
        TiMenu(TiUserWindow* window);
};

class TiWindow : public TiApiPoint {
    public:
        TiWindow(TiUserWindow* window);
};

#endif
