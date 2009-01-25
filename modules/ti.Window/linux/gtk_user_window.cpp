/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "window_module_linux.h"
#include <iostream>

using namespace ti;

static void destroy_cb (GtkWidget* widget, gpointer data);
static void window_object_cleared_cb (WebKitWebView*,
                                      WebKitWebFrame*,
                                      JSGlobalContextRef,
                                      JSObjectRef, gpointer);

GtkUserWindow::GtkUserWindow(Host *host, WindowConfig* config) : UserWindow(host, config)
{
	this->gtk_window = NULL;
	this->web_view = NULL;
	this->menu_wrapper = NULL;
}

GtkUserWindow::~GtkUserWindow()
{
	if (this->gtk_window != NULL)
	{
		gtk_widget_destroy(GTK_WIDGET(this->gtk_window));
		this->gtk_window = NULL;
		this->web_view = NULL;
	}
}

void GtkUserWindow::Open() {

	if (this->gtk_window == NULL)
	{
		/* web view */
		WebKitWebView* web_view = WEBKIT_WEB_VIEW (webkit_web_view_new ());
		g_signal_connect(G_OBJECT (web_view), "window-object-cleared",
		                 G_CALLBACK (window_object_cleared_cb),
		                 this);

		/* web view scroller */
		GtkWidget* scrolled_window = gtk_scrolled_window_new (NULL, NULL);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW (scrolled_window),
		                               GTK_POLICY_AUTOMATIC,
		                               GTK_POLICY_AUTOMATIC);
		gtk_container_add(GTK_CONTAINER (scrolled_window),
		                  GTK_WIDGET (web_view));

		/* main window vbox */
		this->vbox = gtk_vbox_new(FALSE, 0);
		gtk_box_pack_start(GTK_BOX (vbox),
		                   GTK_WIDGET(scrolled_window),
		                   TRUE, TRUE, 0);

		/* main window */
		GtkWidget* window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
		gtk_window_set_default_size(GTK_WINDOW (window),
		                            this->config->GetWidth(),
		                            this->config->GetHeight());
		gtk_widget_set_name(window, this->config->GetTitle().c_str());

		g_signal_connect(G_OBJECT (window), "destroy",
		                 G_CALLBACK (destroy_cb), NULL);
		gtk_container_add(GTK_CONTAINER (window), vbox);

		this->gtk_window = GTK_WINDOW(window);
		this->web_view = web_view;
		//this->SetupTransparency();

		gtk_widget_realize(window);
		this->SetupDecorations();

		webkit_web_view_open(web_view, this->config->GetURL().c_str());

		gtk_widget_grab_focus(GTK_WIDGET (web_view));

		if (this->config->IsVisible())
		{
			gtk_widget_show_all(window);
		}

		if (this->config->IsFullscreen())
		{
			gtk_window_fullscreen(this->gtk_window);
		}

		UserWindow::Open(this);
	}
	else
	{
		this->Show();
	}
}

void GtkUserWindow::SetupTransparency()
{
	GValue val = {0,};
	g_value_init(&val, G_TYPE_BOOLEAN);
	g_value_set_boolean(&val, 1);
	g_object_set_property(G_OBJECT (this->web_view), "transparent", &val); 

	GdkScreen* screen = gtk_widget_get_screen(GTK_WIDGET(this->gtk_window));
	GdkColormap* colormap = gdk_screen_get_rgba_colormap(screen);
	if (!colormap) {
		std::cerr << "Could not use ARGB colormap. "
		          << "True transparency not available." << std::endl;
		colormap = gdk_screen_get_rgb_colormap(screen);
	}
	gtk_widget_set_colormap(GTK_WIDGET(this->gtk_window), colormap);
}

void GtkUserWindow::Close()
{
	UserWindow::Close(this);
}

static void destroy_cb (GtkWidget* widget, gpointer data) {
}

static void window_object_cleared_cb (WebKitWebView* web_view,
                                      WebKitWebFrame* web_frame,
                                      JSGlobalContextRef context,
                                      JSObjectRef window_object,
                                      gpointer data) {

	JSObjectRef global_object = JSContextGetGlobalObject(context);
	GtkUserWindow* user_window = (GtkUserWindow*) data;
	Host* tihost = user_window->GetHost();

	// Produce a delegating object to represent the top-level
	// Titanium object. When a property isn't found in this object
	// it will look for it in global_tibo.
	SharedBoundObject global_tibo = tihost->GetGlobalObject();
	BoundObject* ti_object = new DelegateStaticBoundObject(global_tibo);
	SharedBoundObject shared_ti_obj = SharedBoundObject(ti_object);

	// Set user window into the Titanium object
	SharedBoundObject* shared_user_window = new SharedBoundObject(user_window);
	SharedValue user_window_val = Value::NewObject(*shared_user_window);
	ti_object->Set("currentWindow", user_window_val);

	// Place the Titanium object into the window's global object
	BoundObject *global_bound_object = new KJSBoundObject(context, global_object);
	SharedValue ti_object_value = Value::NewObject(shared_ti_obj);
	global_bound_object->Set(GLOBAL_NS_VARNAME, ti_object_value);

	//FIXME: evaluate TI_WINDOW_BINDING_JS_CODE (as javascript) in the
	//JS global context so that Titanium.Window.createWindow function
	//is correctly defined to pass in parent  (see define in window_binding.h)
}

void GtkUserWindow::Hide() {
	gtk_widget_hide_all(GTK_WIDGET(this->gtk_window));
	this->config->SetVisible(false);
}

void GtkUserWindow::Show() {
	gtk_widget_show_all(GTK_WIDGET(this->gtk_window));
	this->config->SetVisible(true);
}

bool GtkUserWindow::IsUsingScrollbars() {
	return this->config->IsUsingScrollbars();
}

bool GtkUserWindow::IsFullScreen() {
	return this->config->IsFullscreen();
}

std::string GtkUserWindow::GetId() {
	return this->config->GetID();
}


double GtkUserWindow::GetX() {
	int x, y;
	gtk_window_get_position(this->gtk_window, &x, &y);
	return x;
}

void GtkUserWindow::SetX(double x) {
	int y = GetY();
	gtk_window_move(this->gtk_window, int(x), y);
	this->config->SetX(x);
}

double GtkUserWindow::GetY() {
	int x, y;
	gtk_window_get_position (this->gtk_window, &x, &y);
	return y;
}

void GtkUserWindow::SetY(double y) {
	int x = GetX();
	gtk_window_move(this->gtk_window, x, int(y));
	this->config->SetY(y);
}

double GtkUserWindow::GetWidth() {
	int width, height;
	gtk_window_get_size (this->gtk_window, &width, &height);
	return width;
}

void GtkUserWindow::SetWidth(double width) {
	int height = GetHeight();
	gtk_window_resize(this->gtk_window, int(width), height);
	this->config->SetWidth(width);
}

double GtkUserWindow::GetHeight() {
	int width, height;
	gtk_window_get_size (this->gtk_window, &width, &height);
	return width;
}

void GtkUserWindow::SetHeight(double height) {
	int width = GetWidth();
	gtk_window_resize(this->gtk_window, width, int(height));
	this->config->SetHeight(height);
}

Bounds GtkUserWindow::GetBounds() {
	int width, height;
	int x, y;
	gtk_window_get_size(this->gtk_window, &width, &height);
	gtk_window_get_position(this->gtk_window, &x, &y);
	Bounds b = { x, y, width, height };
	return b;
}

void GtkUserWindow::SetBounds(Bounds b) {
	gtk_window_resize(this->gtk_window, int(b.width), int(b.height));
	gtk_window_move(this->gtk_window, int(b.x), int(b.y));

	this->config->SetX(b.x);
	this->config->SetY(b.y);
	this->config->SetWidth(b.width);
	this->config->SetHeight(b.height);
}

std::string GtkUserWindow::GetTitle() {
	return std::string(gtk_window_get_title(this->gtk_window));
}

void GtkUserWindow::SetTitle(std::string& title)
{
	gtk_window_set_title (this->gtk_window, title.c_str());
	this->config->SetTitle(title);
}

std::string GtkUserWindow::GetURL()
{
	return this->config->GetURL();
}

void GtkUserWindow::SetURL(std::string& uri)
{
	webkit_web_view_open (this->web_view, uri.c_str());
	this->config->SetURL(uri);
}

void GtkUserWindow::SetupDecorations()
{
	GdkWindow *gdk_window = gtk_widget_get_window(GTK_WIDGET(this->gtk_window));
	int d = 0;

	if (this->config->IsUsingChrome())
		d = d | GDK_DECOR_BORDER | GDK_DECOR_TITLE | GDK_DECOR_MENU;

	if (this->config->IsResizable())
		d = d | GDK_DECOR_RESIZEH;

	if (this->config->IsMinimizable())
		d = d | GDK_DECOR_MINIMIZE;

	if (this->config->IsMaximizable())
		d = d | GDK_DECOR_MAXIMIZE;

	this->SetTransparency(config->GetTransparency());

	gdk_window_set_decorations(gdk_window, (GdkWMDecoration) d);
}

bool GtkUserWindow::IsUsingChrome() {
	return gtk_window_get_decorated(this->gtk_window);
}

void GtkUserWindow::SetUsingChrome(bool chrome) {
	//TODO: implement
}

bool GtkUserWindow::IsResizable()
{
	return gtk_window_get_resizable(this->gtk_window);
}

void GtkUserWindow::SetResizable(bool resizable)
{
	gtk_window_set_resizable(this->gtk_window, resizable);
	this->config->SetResizable(resizable);
}

bool GtkUserWindow::IsMaximizable()
{
	return this->config->IsMaximizable();
}

void GtkUserWindow::SetMaximizable(bool maximizable)
{
	this->config->SetMaximizable(maximizable);
	this->SetupDecorations();
}

bool GtkUserWindow::IsMinimizable()
{
	return this->config->IsMinimizable();
}

void GtkUserWindow::SetMinimizable(bool minimizable)
{
	this->config->SetMinimizable(minimizable);
	this->SetupDecorations();
}

bool GtkUserWindow::IsCloseable()
{
	return gtk_window_get_deletable(this->gtk_window);
}
void GtkUserWindow::SetCloseable(bool closeable)
{
	this->config->SetCloseable(closeable);
	gtk_window_set_deletable(this->gtk_window, closeable);
}

bool GtkUserWindow::IsVisible()
{
	return this->config->IsVisible();
}

void GtkUserWindow::SetVisible(bool visible)
{
	if (visible) {
		this->Show();
	} else {
		this->Hide();
	}
}

double GtkUserWindow::GetTransparency()
{
	return gtk_window_get_opacity(this->gtk_window);
}

void GtkUserWindow::SetTransparency(double alpha)
{
	return gtk_window_set_opacity(this->gtk_window, alpha);
	this->config->SetTransparency(alpha);
}

void GtkUserWindow::SetFullScreen(bool fullscreen)
{
	if (fullscreen)
	{
		gtk_window_fullscreen(this->gtk_window);
	}
	else
	{
		gtk_window_unfullscreen(this->gtk_window);
	}
}

void GtkUserWindow::SetMenu(SharedBoundList menu)
{
	if (this->menu_wrapper != NULL)
	{
		GtkWidget* old_menu_bar = this->menu_wrapper->GetMenuBar();
		gtk_container_remove(GTK_CONTAINER(this->vbox), old_menu_bar);
		delete this->menu_wrapper;
	}

	this->menu_wrapper = new GtkMenuWrapper(menu);
	GtkWidget* gtk_menu_bar = this->menu_wrapper->GetMenuBar();
	gtk_box_pack_start(GTK_BOX(this->vbox), gtk_menu_bar, FALSE, FALSE, 2);
	gtk_box_reorder_child(GTK_BOX(this->vbox), gtk_menu_bar, 0);
	gtk_widget_show(gtk_menu_bar);

}

