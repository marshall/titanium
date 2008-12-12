#include "ti_gtk_types.h"

static void destroy_cb (GtkWidget* widget, gpointer data);
static void window_object_cleared_cb (WebKitWebView* web_view,
                                      WebKitWebFrame* web_frame,
                                      JSGlobalContextRef context,
                                      JSObjectRef window_object,
                                      gpointer data);

static void title_change_cb (WebKitWebView* web_view,
                             WebKitWebFrame* web_frame,
                             const gchar* title,
                             gpointer data);

TiGtkUserWindow::TiGtkUserWindow() {

    /* web view */
    WebKitWebView* web_view = WEBKIT_WEB_VIEW (webkit_web_view_new ());
    //g_signal_connect (G_OBJECT (web_view), "load-progress-changed", G_CALLBACK (progress_change_cb), web_view);
    //g_signal_connect (G_OBJECT (web_view), "load-committed", G_CALLBACK (load_commit_cb), web_view);
    //g_signal_connect (G_OBJECT (web_view), "hovering-over-link", G_CALLBACK (link_hover_cb), web_view);
    g_signal_connect (G_OBJECT (web_view), "title-changed", G_CALLBACK (title_change_cb), this);
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

    gchar* uri = "http://www.google.com/";
    webkit_web_view_open (web_view, uri);

    gtk_widget_grab_focus(GTK_WIDGET (web_view));
    gtk_widget_show_all(window);
}

void TiGtkUserWindow::change_title(const gchar* title) {

    if (this->window_title)
        g_free (this->window_title);

    this->window_title = g_strdup (title);
    gtk_window_set_title (this->gtk_window, this->window_title);
}

void TiGtkUserWindow::hide(bool animate) {
    gtk_widget_hide_all(GTK_WIDGET(this->gtk_window));
}

void TiGtkUserWindow::show(bool animate) {
    gtk_widget_show_all(GTK_WIDGET(this->gtk_window));
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
    sprintf(code, "tiRuntime.Window.currentWindow.hide();");
    script = JSStringCreateWithUTF8CString(code);
    if(JSCheckScriptSyntax(context, script, NULL, 0, NULL))
        JSEvaluateScript(context, script, window_object, NULL, 1, NULL);
    JSStringRelease(script);

}

static void title_change_cb (WebKitWebView* web_view,
                             WebKitWebFrame* web_frame,
                             const gchar* title,
                             gpointer data) {

    //TiGtkUserWindow* ti_gtk_window = (TiGtkUserWindow*) data;
    //ti_gtk_window->change_title(title);

}

