
#ifndef __TI_GTK_TYPES_H
#define __TI_GTK_TYPES_H
#include "ti_types.h"

class TiGtkUserWindow : public TiUserWindow {

    public:
        TiGtkUserWindow();
        void change_title(const gchar*);

        void hide(bool animate);
        void show(bool animate);
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
        bool get_transparency();
        void set_transparency(bool transparency);

    protected:
        GtkWindow* gtk_window;
        gchar* window_title;
        bool full_screen;
        bool using_scrollbars;
        char* id;
};

#endif
