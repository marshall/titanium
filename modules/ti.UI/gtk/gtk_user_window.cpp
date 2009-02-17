/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "../ui_module.h"
#include <iostream>

using namespace ti;

#define STUB() printf("Method is still a stub, %s:%i\n", __FILE__, __LINE__)

static void destroy_cb(
	GtkWidget* widget,
	gpointer data);
static void frame_event_cb(
	GtkWindow *window,
	GdkEvent *event,
	gpointer user_data);
static void window_object_cleared_cb(
	WebKitWebView*,
	WebKitWebFrame*,
	JSGlobalContextRef,
	JSObjectRef, gpointer);
static void populate_popup_cb(
	WebKitWebView *web_view,
	GtkMenu *menu,
	gpointer data);

GtkUserWindow::GtkUserWindow(Host *host, WindowConfig* config) : UserWindow(host, config)
{
	this->gtk_window = NULL;
	this->web_view = NULL,
	this->menu = NULL;
	this->menu_bar = NULL;
	this->gdk_width = -1;
	this->gdk_height = -1;
	this->gdk_x = -1;
	this->gdk_y = -1;

}

GtkUserWindow::~GtkUserWindow()
{
	this->Close();
}

UserWindow* GtkUserWindow::WindowFactory(Host *host, WindowConfig* config)
{
	return new GtkUserWindow(host, config);
}

void GtkUserWindow::Open()
{
	if (this->gtk_window == NULL)
	{
		/* web view */
		WebKitWebView* web_view = WEBKIT_WEB_VIEW (webkit_web_view_new ());
		g_signal_connect(G_OBJECT (web_view), "window-object-cleared",
		                 G_CALLBACK (window_object_cleared_cb),
		                 this);

		g_signal_connect(
			G_OBJECT (web_view),
			"populate-popup",
			G_CALLBACK (populate_popup_cb),
			this);

		GtkWidget* view_container = NULL;
		if (this->IsUsingScrollbars())
		{
			/* web view scroller */
			GtkWidget* scrolled_window = gtk_scrolled_window_new (NULL, NULL);
			gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
			                               GTK_POLICY_AUTOMATIC,
			                               GTK_POLICY_AUTOMATIC);
			gtk_container_add(GTK_CONTAINER (scrolled_window),
			                  GTK_WIDGET (web_view));
			view_container = scrolled_window;
		}
		else // No scrollin' fer ya.
		{
			view_container = GTK_WIDGET(web_view);
		}

		/* main window vbox */
		this->vbox = gtk_vbox_new(FALSE, 0);
		gtk_box_pack_start(GTK_BOX (vbox),
		                   GTK_WIDGET(view_container),
		                   TRUE, TRUE, 0);

		/* main window */
		GtkWidget* window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
		gtk_window_set_default_size(GTK_WINDOW (window),
		                            this->config->GetWidth(),
		                            this->config->GetHeight());
		gtk_widget_set_name(window, this->config->GetTitle().c_str());

		g_signal_connect(G_OBJECT(window), "destroy",
		                 G_CALLBACK(destroy_cb), this);
		g_signal_connect(G_OBJECT(window), "frame-event",
		                 G_CALLBACK(frame_event_cb), this);
		gtk_container_add(GTK_CONTAINER (window), vbox);

		this->gtk_window = GTK_WINDOW(window);
		this->web_view = web_view;
		//this->SetupTransparency();

		gtk_widget_realize(window);
		this->SetupDecorations();
		this->SetupSizeLimits();
		this->SetupPosition();
		this->SetupSize();
		this->SetupMenu();
		this->SetupIcon();

		webkit_web_view_open(web_view, this->config->GetURL().c_str());

		gtk_widget_grab_focus(GTK_WIDGET (web_view));

		if (this->IsVisible())
		{
			gtk_widget_show_all(window);
		}

		if (this->config->IsFullScreen())
		{
			gtk_window_fullscreen(this->gtk_window);
		}

		UserWindow::Open(this);
		this->FireEvent(OPENED);
	}
	else
	{
		this->Show();
	}
}

void GtkUserWindow::Close()
{
	if (this->gtk_window != NULL)
	{
		this->FireEvent(CLOSED);
		this->RemoveOldMenu();
		gtk_widget_destroy(GTK_WIDGET(gtk_window));
		this->gtk_window = NULL;
		this->web_view = NULL;
	}

	UserWindow::Close();
}

void GtkUserWindow::SetupTransparency()
{
	if (this->gtk_window != NULL)
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
}

void GtkUserWindow::SetupDecorations()
{
	if (this->gtk_window != NULL)
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
}


void GtkUserWindow::SetupSizeLimits()
{
	if (this->gtk_window != NULL)
	{
		GdkGeometry hints;
		hints.max_width = this->config->GetMaxWidth();
		hints.min_width = this->config->GetMinWidth();
		hints.max_height = this->config->GetMaxHeight();
		hints.min_height = this->config->GetMinHeight();
		GdkWindowHints mask = (GdkWindowHints) (GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE);
		gtk_window_set_geometry_hints(this->gtk_window, NULL, &hints, mask);
	}
}

void GtkUserWindow::SetupPosition()
{
	if (this->gtk_window != NULL)
	{
		gtk_window_move(this->gtk_window, this->GetX(), this->GetY());
	}
}

void GtkUserWindow::SetupSize()
{
	if (this->gtk_window != NULL)
	{
		gtk_window_resize(this->gtk_window,
		                  (int) this->GetWidth(),
		                  (int) this->GetHeight());
	}
}

void GtkUserWindow::SetupIcon()
{
	if (this->gtk_window == NULL)
		return;

	GdkPixbuf* icon = NULL; // NULL is an unset.
	SharedString icon_path = this->icon_path;
	if (icon_path.isNull() && !UIModule::GetIcon().isNull())
		icon_path = UIModule::GetIcon();

	if (!icon_path.isNull())
	{
		GError* error = NULL;
		icon = gdk_pixbuf_new_from_file(icon_path->c_str(), &error);

		if (icon == NULL && error != NULL)
		{
			std::cerr << "Could not load icon because: "
			          << error->message << std::endl;
			g_error_free(error);
		}

	}

	gtk_window_set_icon(this->gtk_window, icon);
}

static void destroy_cb(
	GtkWidget* widget,
	gpointer data)
{
	GtkUserWindow* user_window = (GtkUserWindow*) data;
	user_window->Close();
}

static void frame_event_cb(
	GtkWindow *w,
	GdkEvent *event,
	gpointer data)
{
	GtkUserWindow* window = (GtkUserWindow*) data;
	if (event->type == GDK_FOCUS_CHANGE)
	{
		GdkEventFocus* f = (GdkEventFocus*) event;
		if (f->in)
			window->FireEvent(FOCUSED);
		else
			window->FireEvent(UNFOCUSED);
	}
	else if (event->type == GDK_WINDOW_STATE)
	{
		GdkEventWindowState* f = (GdkEventWindowState*) event;
		if ((f->changed_mask & GDK_WINDOW_STATE_WITHDRAWN)
			&& (f->new_window_state & GDK_WINDOW_STATE_WITHDRAWN))
		{
			window->FireEvent(HIDDEN);
		}
		else if ((f->changed_mask & GDK_WINDOW_STATE_ICONIFIED)
			&& (f->new_window_state & GDK_WINDOW_STATE_ICONIFIED))
		{
			window->FireEvent(MINIMIZED);
		}
		else if (((f->changed_mask & GDK_WINDOW_STATE_FULLSCREEN)
			&& (f->new_window_state & GDK_WINDOW_STATE_FULLSCREEN)))
		{
			window->FireEvent(FULLSCREENED);
		}
		else if (f->changed_mask & GDK_WINDOW_STATE_FULLSCREEN)
		{
			window->FireEvent(UNFULLSCREENED);
		}
		else if (((f->changed_mask & GDK_WINDOW_STATE_MAXIMIZED)
			&& (f->new_window_state & GDK_WINDOW_STATE_MAXIMIZED)))
		{
			window->FireEvent(MAXIMIZED);
		}
	}
	else if (event->type == GDK_CONFIGURE)
	{
		GdkEventConfigure* c = (GdkEventConfigure*) event;
		if (c->x != window->gdk_x || c->y != window->gdk_y)
		{
			window->gdk_x = c->x;
			window->gdk_y = c->y;
			window->FireEvent(RESIZED);
		}

		if (c->width != window->gdk_width || c->height != window->gdk_height)
		{
			window->gdk_height = c->height;
			window->gdk_width = c->width;
			window->FireEvent(MOVED);
		}
	}
}

static void window_object_cleared_cb(
	WebKitWebView* web_view,
	WebKitWebFrame* web_frame,
	JSGlobalContextRef context,
	JSObjectRef window_object,
	gpointer data)
{
	JSContextGroupRef group = JSContextGetGroup(context);
	KJSUtil::RegisterGlobalContext(group, context);

	JSObjectRef global_object = JSContextGetGlobalObject(context);
	GtkUserWindow* user_window = (GtkUserWindow*) data;
	Host* tihost = user_window->GetHost();

	// Produce a delegating object to represent the top-level
	// Titanium object. When a property isn't found in this object
	// it will look for it in global_tibo.
	SharedBoundObject global_tibo = tihost->GetGlobalObject();
	BoundObject* ti_object = new DelegateStaticBoundObject(global_tibo);
	SharedBoundObject shared_ti_obj = SharedBoundObject(ti_object);

	SharedValue ui_api_value = ti_object->Get("UI");
	if (ui_api_value->IsObject())
	{
		// Create a delegate object for the UI API.
		SharedBoundObject ui_api = ui_api_value->ToObject();
		BoundObject* delegate_ui_api = new DelegateStaticBoundObject(ui_api);

		// Place currentWindow in the delegate.
		SharedBoundObject* shared_user_window = new SharedBoundObject(user_window);
		SharedValue user_window_val = Value::NewObject(*shared_user_window);
		delegate_ui_api->Set("currentWindow", user_window_val);

		// Place currentWindow.createWindow in the delegate.
		SharedValue create_window_value = user_window->Get("createWindow");
		delegate_ui_api->Set("createWindow", create_window_value);

		ti_object->Set("UI", Value::NewObject(delegate_ui_api));
	}
	else
	{
		std::cerr << "Could not find UI API point!" << std::endl;
	}

	// Place the Titanium object into the window's global object
	BoundObject *global_bound_object = new KJSBoundObject(context, global_object);
	SharedValue ti_object_value = Value::NewObject(shared_ti_obj);
	global_bound_object->Set(GLOBAL_NS_VARNAME, ti_object_value);

}

static void populate_popup_cb(WebKitWebView *web_view,
                               GtkMenu *menu,
                               gpointer data)
{
	GtkUserWindow* user_window = (GtkUserWindow*) data;
	SharedPtr<GtkMenuItemImpl> m =
		user_window->GetContextMenu().cast<GtkMenuItemImpl>();

	if (m.isNull())
		m = UIModule::GetContextMenu().cast<GtkMenuItemImpl>();

	GList* children = gtk_container_get_children(GTK_CONTAINER(menu));
	for (size_t i = 0; i < g_list_length(children); i++)
	{
		GtkWidget* w = (GtkWidget*) g_list_nth_data(children, i);
		gtk_container_remove(GTK_CONTAINER(menu), w);
	}

	if (m.isNull())
		return;

	m->AddChildrenTo(GTK_WIDGET(menu));
}


void GtkUserWindow::Hide() {
	this->config->SetVisible(false);

	if (this->gtk_window != NULL)
	{
		gtk_widget_hide_all(GTK_WIDGET(this->gtk_window));
	}
}

void GtkUserWindow::Show() {
	this->config->SetVisible(true);
	if (this->gtk_window != NULL)
	{
		gtk_widget_show_all(GTK_WIDGET(this->gtk_window));
	}
}

bool GtkUserWindow::IsUsingScrollbars() {
	return this->config->IsUsingScrollbars();
}

bool GtkUserWindow::IsFullScreen() {
	return this->config->IsFullScreen();
}

void GtkUserWindow::SetFullScreen(bool fullscreen)
{
	this->config->SetFullScreen(fullscreen);
	if (fullscreen && this->gtk_window != NULL)
	{
		gtk_window_fullscreen(this->gtk_window);
	}
	else if (this->gtk_window != NULL)
	{
		gtk_window_unfullscreen(this->gtk_window);
	}
}


std::string GtkUserWindow::GetId() {
	return this->config->GetID();
}


double GtkUserWindow::GetX() {
	return this->config->GetX();
}

void GtkUserWindow::SetX(double x) {
	this->config->SetX(x);
	this->SetupPosition();
}

double GtkUserWindow::GetY() {
	return this->config->GetY();
}

void GtkUserWindow::SetY(double y) {
	this->config->SetY(y);
	this->SetupPosition();
}

double GtkUserWindow::GetWidth() {
	return this->config->GetWidth();
}

void GtkUserWindow::SetWidth(double width) {
	this->config->SetWidth((int)width);
	this->SetupSize();
}

double GtkUserWindow::GetMaxWidth() {
	return this->config->GetMaxWidth();
}

void GtkUserWindow::SetMaxWidth(double width) {
	this->config->SetMaxWidth(width);
	this->SetupSizeLimits();
}

double GtkUserWindow::GetMinWidth() {
	return this->config->GetMinWidth();
}

void GtkUserWindow::SetMinWidth(double width) {
	this->config->SetMinWidth(width);
	this->SetupSizeLimits();
}

double GtkUserWindow::GetHeight() {
	return this->config->GetHeight();
}

void GtkUserWindow::SetHeight(double height) {
	this->config->SetHeight((int) height);
	this->SetupSize();
}

double GtkUserWindow::GetMaxHeight() {
	return this->config->GetMaxHeight();
}

void GtkUserWindow::SetMaxHeight(double height) {
	this->config->SetMaxHeight(height);
	this->SetupSizeLimits();
}

double GtkUserWindow::GetMinHeight() {
	return this->config->GetMinHeight();
}

void GtkUserWindow::SetMinHeight(double height) {
	this->config->SetMinHeight(height);
	this->SetupSizeLimits();
}

Bounds GtkUserWindow::GetBounds() {
	int width = this->GetWidth();
	int height = this->GetHeight();
	int x = this->GetX();
	int y = this->GetY();
	Bounds b = {x, y, width, height};
	return b;
}

void GtkUserWindow::SetBounds(Bounds b) {
	this->config->SetX(b.x);
	this->config->SetY(b.y);
	this->config->SetWidth(b.width);
	this->config->SetHeight(b.height);

	this->SetupSize();
	this->SetupPosition();
}

std::string GtkUserWindow::GetTitle() {
	return this->config->GetTitle();
}

void GtkUserWindow::SetTitle(std::string& title)
{
	if (this->gtk_window != NULL)
		gtk_window_set_title(this->gtk_window, "");

	this->config->SetTitle(title);

	if (this->gtk_window != NULL)
	{
		std::string& ntitle = this->config->GetTitle();
		gtk_window_set_title(this->gtk_window, ntitle.c_str());
	}
}

std::string GtkUserWindow::GetURL()
{
	return this->config->GetURL();
}

void GtkUserWindow::SetURL(std::string& uri)
{
	this->config->SetURL(uri);

	if (this->gtk_window != NULL && this->web_view != NULL)
		webkit_web_view_open(this->web_view, uri.c_str());
}

bool GtkUserWindow::IsUsingChrome() {
	return this->config->IsUsingChrome();
}

void GtkUserWindow::SetUsingChrome(bool chrome) {
	this->config->SetUsingChrome(chrome);

	if (this->gtk_window != NULL)
		gtk_window_set_decorated(this->gtk_window, chrome);
}

bool GtkUserWindow::IsResizable()
{
	return this->config->IsResizable();
}

void GtkUserWindow::SetResizable(bool resizable)
{
	this->config->SetResizable(resizable);

	if (this->gtk_window != NULL)
		gtk_window_set_resizable(this->gtk_window, resizable);
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
	return this->config->IsCloseable();
}
void GtkUserWindow::SetCloseable(bool closeable)
{
	this->config->SetCloseable(closeable);
	if (this->gtk_window != NULL)
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
	return this->config->GetTransparency();
}

void GtkUserWindow::SetTransparency(double alpha)
{
	this->config->SetTransparency(alpha);

	if (this->gtk_window != NULL)
		gtk_window_set_opacity(this->gtk_window, alpha);
}

bool GtkUserWindow::IsTopMost()
{
	return this->config->IsTopMost();
}

void GtkUserWindow::SetTopMost(bool topmost)
{
	this->config->SetTopMost(topmost);

	if (this->gtk_window != NULL)
	{
		guint topmost_i = topmost ? TRUE : FALSE;
		gtk_window_set_keep_above(this->gtk_window, topmost_i);
	}
}

void GtkUserWindow::SetMenu(SharedPtr<MenuItem> value)
{
	SharedPtr<GtkMenuItemImpl> menu = value.cast<GtkMenuItemImpl>();
	this->menu = menu;
	this->SetupMenu();
}

SharedPtr<MenuItem> GtkUserWindow::GetMenu()
{
	return this->menu;
}

void GtkUserWindow::SetContextMenu(SharedPtr<MenuItem> value)
{
	SharedPtr<GtkMenuItemImpl> menu = value.cast<GtkMenuItemImpl>();
	this->context_menu = menu;
}

SharedPtr<MenuItem> GtkUserWindow::GetContextMenu()
{
	return this->context_menu;
}


void GtkUserWindow::SetIcon(SharedString icon_path)
{
	this->icon_path = icon_path;
	this->SetupIcon();
}

SharedString GtkUserWindow::GetIcon()
{
	return this->icon_path;
}

void GtkUserWindow::RemoveOldMenu()
{
	// Check if we are already using a menu
	// and the window is initialized.
	if (this->gtk_window != NULL &&
	    !this->menu_in_use.isNull() &&
	    this->menu_bar != NULL)
	{
		this->menu_in_use->ClearRealization(this->menu_bar);
		gtk_container_remove(GTK_CONTAINER(this->vbox), this->menu_bar);
	}

	this->menu_in_use = NULL;
	this->menu_bar = NULL;
}

void GtkUserWindow::SetupMenu()
{
	SharedPtr<GtkMenuItemImpl> menu = this->menu;
	SharedPtr<MenuItem> app_menu = UIModule::GetMenu();

	// No window menu, try to use the application menu.
	if (menu.isNull() && !app_menu.isNull())
	{
		menu = app_menu.cast<GtkMenuItemImpl>();
	}

	// Only do this if the menu is actually changing.
	if (menu == this->menu_in_use)
		return;

	this->RemoveOldMenu();

	if (!menu.isNull() && this->gtk_window != NULL)
	{
		GtkWidget* menu_bar = menu->GetMenuBar();
		gtk_box_pack_start(GTK_BOX(this->vbox), menu_bar,
		                   FALSE, FALSE, 2);
		gtk_box_reorder_child(GTK_BOX(this->vbox), menu_bar, 0);
		gtk_widget_show(menu_bar);
		this->menu_bar = menu_bar;
	}

	this->menu_in_use = menu;

}

void GtkUserWindow::AppMenuChanged()
{
	if (this->menu.isNull())
	{
		this->SetupMenu();
	}
}

void GtkUserWindow::AppIconChanged()
{
	if (this->icon_path.isNull())
	{
		this->SetupIcon();
	}
}

