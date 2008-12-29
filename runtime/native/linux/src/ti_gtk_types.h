
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
        void SetupDecorations();

        void Hide();
        void Show();
        bool IsUsingChrome();
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
        TiBounds GetBounds();
        void SetBounds(TiBounds bounds);
        std::string GetTitle();
        void SetTitle(std::string title);
        std::string GetUrl();
        void SetUrl(std::string url);
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

    protected:
        GtkWindow* gtk_window;
        WebKitWebView* web_view;

        std::string window_title;
        std::string uri;

        bool showing;
        bool full_screen;
        bool using_scrollbars;
        std::string id;
        double transparency;

        bool resizable;
        bool chrome;
        bool minimizable;
        bool maximizable;
        bool closeable;
};

#endif
