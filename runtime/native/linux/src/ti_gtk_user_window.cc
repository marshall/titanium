#include "ti_gtk_types.h"

#define STUB printf("__FILE__:__LINE__: called a stub method!\n")

static void destroy_cb (GtkWidget* widget, gpointer data);
static void window_object_cleared_cb (WebKitWebView* web_view,
                                      WebKitWebFrame* web_frame,
                                      JSGlobalContextRef context,
                                      JSObjectRef window_object,
                                      gpointer data);

//static void title_change_cb (WebKitWebView* web_view,
//                             WebKitWebFrame* web_frame,
//                             const gchar* title,
//                             gpointer data);
///void TiGtkUserWindow::change_title(const gchar* title) {
//    char* new_title = g_strdup(title);
//    gtk_window_set_title (this->gtk_window, new_title);
//
//    if (this->window_title)
//        g_free(this->window_title);
//
//    this->window_title = new_title;
//}

TiGtkUserWindow::TiGtkUserWindow() {

    /* web view */
    WebKitWebView* web_view = WEBKIT_WEB_VIEW (webkit_web_view_new ());
    //g_signal_connect (G_OBJECT (web_view), "load-progress-changed", G_CALLBACK (progress_change_cb), web_view);
    //g_signal_connect (G_OBJECT (web_view), "load-committed", G_CALLBACK (load_commit_cb), web_view);
    //g_signal_connect (G_OBJECT (web_view), "hovering-over-link", G_CALLBACK (link_hover_cb), web_view);
    //g_signal_connect (G_OBJECT (web_view), "title-changed", G_CALLBACK (title_change_cb), this);
    g_signal_connect (G_OBJECT (web_view), "window-object-cleared", G_CALLBACK (window_object_cleared_cb), this);

    /* web view scroller */
    GtkWidget* scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add (GTK_CONTAINER (scrolled_window), GTK_WIDGET (web_view));

    /* main window vbox */
    GtkWidget* vbox = gtk_vbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET(scrolled_window), TRUE, TRUE, 0);

    /* main window */
    GtkWidget* window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size (GTK_WINDOW (window), 800, 600);
    gtk_widget_set_name (window, "Titanium");
    g_signal_connect (G_OBJECT (window), "destroy", G_CALLBACK (destroy_cb), NULL);
    gtk_container_add (GTK_CONTAINER (window), vbox);

    this->gtk_window = GTK_WINDOW(window);
    this->web_view = web_view;

    this->uri = uri;
    this->window_title = "";

    this->showing = false;
    this->resizable = true;
    this->chrome = true;
    this->minimizable = true;
    this->maximizable = true;
    this->closeable = true;
    this->transparency = 1.0;
    this->full_screen = false;
    this->using_scrollbars = true;

    gtk_widget_realize(window);
    this->setup_decorations();

    gchar* uri = "http://www.google.com/";
    webkit_web_view_open (web_view, uri);

    gtk_widget_grab_focus(GTK_WIDGET (web_view));
    gtk_widget_show_all(window);
    this->showing = true;
}

static void destroy_cb (GtkWidget* widget, gpointer data) {
    gtk_main_quit ();
}

static void window_object_cleared_cb (WebKitWebView* web_view,
                                      WebKitWebFrame* web_frame,
                                      JSGlobalContextRef context,
                                      JSObjectRef window_object,
                                      gpointer data) {

    TiUserWindow* ti_gtk_window = (TiGtkUserWindow*) data;
    ti_gtk_window->window_object_cleared(context, window_object);

    JSStringRef script;
    char code[1024];
    sprintf(code, "tiRuntime.Window.currentWindow.setTitle('yes');tiRuntime.Window.currentWindow.setTitle('no');");
    script = JSStringCreateWithUTF8CString(code);
    if(JSCheckScriptSyntax(context, script, NULL, 0, NULL))
        JSEvaluateScript(context, script, window_object, NULL, 1, NULL);
    JSStringRelease(script);

}

//static void title_change_cb (WebKitWebView* web_view,
//                             WebKitWebFrame* web_frame,
//                             const gchar* title,
//                             gpointer data) {
//
//    //TiGtkUserWindow* ti_gtk_window = (TiGtkUserWindow*) data;
//    //ti_gtk_window->change_title(title);
//
//}

void TiGtkUserWindow::hide() {
    gtk_widget_hide_all(GTK_WIDGET(this->gtk_window));
    this->showing = false;
}

void TiGtkUserWindow::show() {
    gtk_widget_show_all(GTK_WIDGET(this->gtk_window));
    this->showing = true;
}

bool TiGtkUserWindow::is_using_chrome() {
    return gtk_window_get_decorated(this->gtk_window);
}

bool TiGtkUserWindow::is_using_scrollbars() { 
    return this->using_scrollbars;
}

bool TiGtkUserWindow::is_full_screen() { 
    return this->full_screen;
}

std::string TiGtkUserWindow::get_id() { 
    return this->id;
}

void TiGtkUserWindow::open() { STUB; }
void TiGtkUserWindow::close() { STUB; }

double TiGtkUserWindow::get_x() {
    int x, y;
    gtk_window_get_position(this->gtk_window, &x, &y);
    return x;
}

void TiGtkUserWindow::set_x(double x) { 
    int y = get_y();
    gtk_window_move(this->gtk_window, int(x), y);
}

double TiGtkUserWindow::get_y() {
    int x, y;
    gtk_window_get_position (this->gtk_window, &x, &y);
    return y;
}

void TiGtkUserWindow::set_y(double y) { 
    int x = get_x();
    gtk_window_move(this->gtk_window, x, int(y));
}

double TiGtkUserWindow::get_width() {
    int width, height;
    gtk_window_get_size (this->gtk_window, &width, &height);
    return width;
}

void TiGtkUserWindow::set_width(double width) {
    int height = get_height();
    gtk_window_resize(this->gtk_window, int(width), height);
}

double TiGtkUserWindow::get_height() {
    int width, height;
    gtk_window_get_size (this->gtk_window, &width, &height);
    return width;
}

void TiGtkUserWindow::set_height(double height) {
    int width = get_width();
    gtk_window_resize(this->gtk_window, width, int(height));
}

TiBounds TiGtkUserWindow::get_bounds() {
    int width, height;
    int x, y;
    gtk_window_get_size(this->gtk_window, &width, &height);
    gtk_window_get_position(this->gtk_window, &x, &y);
    TiBounds b = { x, y, width, height };
    return b;
}

void TiGtkUserWindow::set_bounds(TiBounds b) {
    gtk_window_resize(this->gtk_window, int(b.width), int(b.height));
    gtk_window_move(this->gtk_window, int(b.x), int(b.y));
}

std::string TiGtkUserWindow::get_title() {
    return std::string(gtk_window_get_title(this->gtk_window));
}

void TiGtkUserWindow::set_title(std::string title) {
    this->window_title = title;
    gtk_window_set_title (this->gtk_window, this->window_title.c_str());
}

std::string TiGtkUserWindow::get_url() {
    return uri;
}

void TiGtkUserWindow::set_url(std::string uri) {
    this->uri = uri;
    webkit_web_view_open (this->web_view, uri.c_str());
}

void TiGtkUserWindow::setup_decorations() {
    GdkWindow *gdk_window = gtk_widget_get_window(GTK_WIDGET(this->gtk_window));
    int d = 0;

    if (this->chrome)
        d = d | GDK_DECOR_BORDER | GDK_DECOR_TITLE | GDK_DECOR_MENU;
    if (this->resizable) 
        d = d | GDK_DECOR_RESIZEH;
    if (this->minimizable)
        d = d | GDK_DECOR_MINIMIZE;
    if (this->maximizable)
        d = d | GDK_DECOR_MAXIMIZE;

    gdk_window_set_decorations(gdk_window, (GdkWMDecoration) d);
}

bool TiGtkUserWindow::is_resizable() {
    return gtk_window_get_resizable(this->gtk_window);
}

void TiGtkUserWindow::set_resizable(bool resizable) {
    gtk_window_set_resizable(this->gtk_window, resizable);
}

bool TiGtkUserWindow::is_maximizable() { 
    return this->resizable;
}

void TiGtkUserWindow::set_maximizable(bool maximizable) {
    this->maximizable = maximizable;
}

bool TiGtkUserWindow::is_minimizable() {
    return this->minimizable;
} 

void TiGtkUserWindow::set_minimizable(bool minimizable) {
    this->minimizable = minimizable;
}

bool TiGtkUserWindow::is_closeable() { 
    //return this->closeable;
    return gtk_window_get_deletable(this->gtk_window);
}
void TiGtkUserWindow::set_closeable(bool closeable) {
    this->closeable = closeable;
    gtk_window_set_deletable(this->gtk_window, closeable);
}

bool TiGtkUserWindow::is_visible() {
    return this->showing;
}

void TiGtkUserWindow::set_visible(bool visible) { 
    if (visible) {
        this->show();
    } else {
        this->hide();
    }
}

/* Get the X Window between child and the root window.
 *    This should usually be the WM managed XID */
static Window get_topmost_window(Display *dpy, Window child) {
    Window root, parent;
    Window* children;
    unsigned int nchildren;

    XQueryTree(dpy, child, &root, &parent, &children, &nchildren);
    XFree(children);

    while (parent != root) {
        child = parent;
        XQueryTree(dpy, child, &root, &parent, &children, &nchildren);
        XFree(children);
    }

    return child;
}

double TiGtkUserWindow::get_transparency() {
    return this->transparency;
}

void TiGtkUserWindow::set_transparency(double alpha) { 
    GdkWindow *gdk_window = gtk_widget_get_window(GTK_WIDGET(this->gtk_window));

    if (gdk_window == NULL) {
        printf("__FILE__:__LINE__  Couldn't get GDK Window\n");
        return;
    }

    Display* dpy = GDK_WINDOW_XDISPLAY(gdk_window);
    Window win = get_topmost_window(dpy, GDK_WINDOW_XWINDOW(GDK_DRAWABLE(gdk_window)));

    unsigned int opacity = (unsigned int) (0xffffffff * alpha);
    if (alpha == 0xff)
        XDeleteProperty(dpy, win, XInternAtom(dpy, "_NET_WM_WINDOW_OPACITY", False));
    else
        XChangeProperty(dpy, win, XInternAtom(dpy, "_NET_WM_WINDOW_OPACITY", False),
                        XA_CARDINAL, 32, PropModeReplace,
                        (unsigned char *) &opacity, 1L);
    XSync(dpy, False);

    this->transparency = alpha;
}



