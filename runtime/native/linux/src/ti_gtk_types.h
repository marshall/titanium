
#ifndef __TI_GTK_TYPES_H
#define __TI_GTK_TYPES_H
#include "ti_types.h"

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <webkit/webkit.h>

class TiGtkUserWindow : public TiUserWindow {

    public:
        TiGtkUserWindow();
        void setup_decorations();

        void hide();
        void show();
        bool is_using_chrome();
        bool is_using_scrollbars();
        bool is_full_screen();
        char* get_id();
        void open();
        void close();
        double get_x();
        void set_x(double x);
        double get_y();
        void set_y(double y);
        double get_width();
        void set_width(double width);
        double get_height();
        void set_height(double height);
        TiBounds get_bounds();
        void set_bounds(TiBounds bounds);
        char* get_title();
        void set_title(char* title);
        char* get_url();
        void set_url(char* url);
        bool is_resizable();
        void set_resizable(bool resizable);
        bool is_maximizable();
        void set_maximizable(bool maximizable);
        bool is_minimizable();
        void set_minimizable(bool minimizable);
        bool is_closeable();
        void set_closeable(bool closeable);
        bool is_visible();
        void set_visible(bool visible);
        double get_transparency();
        void set_transparency(double transparency);

    protected:
        GtkWindow* gtk_window;
        WebKitWebView* web_view;

        gchar* window_title;
        gchar* uri;

        bool showing;
        bool full_screen;
        bool using_scrollbars;
        char* id;
        double transparency;

        bool resizable;
        bool chrome;
        bool minimizable;
        bool maximizable;
        bool closeable;
};

#endif
