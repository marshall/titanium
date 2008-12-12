#ifndef __TI_TYPES_H
#define __TI_TYPES_H

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

        void window_object_cleared(JSContextRef, JSObjectRef);

        virtual void hide();
        TiValue hide_cb(size_t num_args, TiValue args[]);

        virtual void show();
        TiValue show_cb(size_t num_args, TiValue args[]);

        virtual bool is_using_chrome();
        TiValue is_using_chrome_cb(size_t num_args, TiValue args[]);

        virtual bool is_using_scrollbars();
        TiValue is_using_scrollbars_cb(size_t num_args, TiValue args[]);

        virtual bool is_full_screen();
        TiValue is_full_screen_cb(size_t num_args, TiValue args[]);

        virtual char* get_id();
        TiValue get_id_cb(size_t num_args, TiValue args[]);

        virtual void open();
        TiValue open_cb(size_t num_args, TiValue args[]);
        virtual void close();
        TiValue close_cb(size_t num_args, TiValue args[]);

        virtual double get_x();
        TiValue get_x_cb(size_t num_args, TiValue args[]);
        virtual void set_x(double x);
        TiValue set_x_cb(size_t num_args, TiValue args[]);

        virtual double get_y();
        TiValue get_y_cb(size_t num_args, TiValue args[]);
        virtual void set_y(double y);
        TiValue set_y_cb(size_t num_args, TiValue args[]);

        virtual double get_width();
        TiValue get_width_cb(size_t num_args, TiValue args[]);
        virtual void set_width(double width);
        TiValue set_width_cb(size_t num_args, TiValue args[]);

        virtual double get_height();
        TiValue get_height_cb(size_t num_args, TiValue args[]);
        virtual void set_height(double height);
        TiValue set_height_cb(size_t num_args, TiValue args[]);

        virtual TiBounds get_bounds();
        TiValue get_bounds_cb(size_t num_args, TiValue args[]);
        virtual void set_bounds(TiBounds bounds);
        TiValue set_bounds_cb(size_t num_args, TiValue args[]);

        virtual char* get_title();
        TiValue get_title_cb(size_t num_args, TiValue args[]);
        virtual void set_title(char* title);
        TiValue set_title_cb(size_t num_args, TiValue args[]);

        virtual char* get_url();
        TiValue get_url_cb(size_t num_args, TiValue args[]);
        virtual void set_url(char* url);
        TiValue set_url_cb(size_t num_args, TiValue args[]);

        virtual bool is_resizable();
        TiValue is_resizable_cb(size_t num_args, TiValue args[]);
        virtual void set_resizable(bool resizable);
        TiValue set_resizable_cb(size_t num_args, TiValue args[]);

        virtual bool is_maximizable();
        TiValue is_maximizable_cb(size_t num_args, TiValue args[]);
        virtual void set_maximizable(bool maximizable);
        TiValue set_maximizable_cb(size_t num_args, TiValue args[]);

        virtual bool is_minimizable();
        TiValue is_minimizable_cb(size_t num_args, TiValue args[]);
        virtual void set_minimizable(bool minimizable);
        TiValue set_minimizable_cb(size_t num_args, TiValue args[]);

        virtual bool is_closeable();
        TiValue is_closeable_cb(size_t num_args, TiValue args[]);
        virtual void set_closeable(bool closeable);
        TiValue set_closeable_cb(size_t num_args, TiValue args[]);

        virtual bool is_visible();
        TiValue is_visible_cb(size_t num_args, TiValue args[]);
        virtual void set_visible(bool visible);
        TiValue set_visible_cb(size_t num_args, TiValue args[]);

        virtual double get_transparency();
        TiValue get_transparency_cb(size_t num_args, TiValue args[]);
        virtual void set_transparency(double transparency);
        TiValue set_transparency_cb(size_t num_args, TiValue args[]);

    protected:
        TiObject global;
};

class TiApiPoint : public TiObject {
    public:
        TiApiPoint(TiUserWindow* window);
        TiUserWindow* get_window();

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
        TiValue debug(size_t num_args, TiValue args[]);
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
