/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "windowing_plugin_linux.h"
#include <iostream>

static void destroy_cb (GtkWidget* widget, gpointer data);
static void window_object_cleared_cb (WebKitWebView*, WebKitWebFrame*, JSGlobalContextRef, JSObjectRef, gpointer);

TiGtkUserWindow::TiGtkUserWindow(TiHost *host, TiWindowConfig* config) : TiUserWindow(host, config)
{
	this->gtk_window = NULL;
	this->web_view = NULL;
}

TiGtkUserWindow::~TiGtkUserWindow()
{
	if (this->gtk_window != NULL)
	{
		gtk_widget_destroy(GTK_WIDGET(this->gtk_window));
		this->gtk_window = NULL;
		this->web_view = NULL;
	}
}

void TiGtkUserWindow::Open() {

	if (this->gtk_window == NULL)
	{
		/* web view */
		WebKitWebView* web_view = WEBKIT_WEB_VIEW (webkit_web_view_new ());
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
		gtk_window_set_default_size (GTK_WINDOW (window), this->config->GetWidth(), this->config->GetHeight());
		gtk_widget_set_name (window, this->config->GetTitle().c_str());

		g_signal_connect (G_OBJECT (window), "destroy", G_CALLBACK (destroy_cb), NULL);
		gtk_container_add (GTK_CONTAINER (window), vbox);

		this->gtk_window = GTK_WINDOW(window);
		this->web_view = web_view;

		gtk_widget_realize(window);
		this->SetupDecorations();

		webkit_web_view_open (web_view, this->config->GetURL().c_str());

		gtk_widget_grab_focus(GTK_WIDGET (web_view));

		if (this->config->IsVisible())
		{
			gtk_widget_show_all(window);
		}
		TiUserWindow::Open(this);
	}
	else
	{
		this->Show();
	}
}

void TiGtkUserWindow::Close()
{
	TiUserWindow::Close(this);
}

static void destroy_cb (GtkWidget* widget, gpointer data) {
}

static void window_object_cleared_cb (WebKitWebView* web_view,
                                      WebKitWebFrame* web_frame,
                                      JSGlobalContextRef context,
                                      JSObjectRef window_object,
                                      gpointer data) {

	JSObjectRef global_object = JSContextGetGlobalObject(context);

	TiGtkUserWindow* user_window = (TiGtkUserWindow*) data;

	TiHost* tihost = user_window->GetHost();
	TiBoundObject* global_tibo = (TiStaticBoundObject*) tihost->GetGlobalObject();

	// place window into the context local for currentWindow
	TiStaticBoundObject *context_local = GetContextLocal(context);

	// set user window into the context
	TiValue *user_window_val = new TiValue(user_window);
	context_local->Set("currentWindow", user_window_val);
	TI_DECREF(user_window_val);

	// Bind all child objects to global context
	std::vector<std::string> prop_names = global_tibo->GetPropertyNames();
	for (size_t i = 0; i < prop_names.size(); i++)
	{
		const char *name = prop_names.at(i).c_str();
		TiValue* value = global_tibo->Get(name, context_local);

		JSValueRef js_value = TiValueToJSValue(context, value);
		BindPropertyToJSObject(context, global_object, name, js_value);
	}

	JSStringRef script;
	char code[1024];
	sprintf(code, "alert(typeof(foo.list.push));");
	//sprintf(code, "var o = Object(); o.prop = 'one'; foo.prop = 'two'; var f = function() {alert(this.prop);}; o.f = f; o.f(); foo.f = o.f; foo.f();");
	script = JSStringCreateWithUTF8CString(code);
	if(JSCheckScriptSyntax(context, script, NULL, 0, NULL))
		JSEvaluateScript(context, script, window_object, NULL, 1, NULL);
	JSStringRelease(script);

}

void TiGtkUserWindow::Hide() {
	gtk_widget_hide_all(GTK_WIDGET(this->gtk_window));
	this->config->SetVisible(false);
}

void TiGtkUserWindow::Show() {
	gtk_widget_show_all(GTK_WIDGET(this->gtk_window));
	this->config->SetVisible(true);
}

bool TiGtkUserWindow::IsUsingChrome() {
	return gtk_window_get_decorated(this->gtk_window);
}

bool TiGtkUserWindow::IsUsingScrollbars() {
	return this->config->IsUsingScrollbars();
}

bool TiGtkUserWindow::IsFullScreen() {
	return this->config->IsFullscreen();
}

std::string TiGtkUserWindow::GetId() {
	return this->config->GetID();
}


double TiGtkUserWindow::GetX() {
	int x, y;
	gtk_window_get_position(this->gtk_window, &x, &y);
	return x;
}

void TiGtkUserWindow::SetX(double x) {
	int y = GetY();
	gtk_window_move(this->gtk_window, int(x), y);
	this->config->SetX(x);
}

double TiGtkUserWindow::GetY() {
	int x, y;
	gtk_window_get_position (this->gtk_window, &x, &y);
	return y;
}

void TiGtkUserWindow::SetY(double y) {
	int x = GetX();
	gtk_window_move(this->gtk_window, x, int(y));
	this->config->SetY(y);
}

double TiGtkUserWindow::GetWidth() {
	int width, height;
	gtk_window_get_size (this->gtk_window, &width, &height);
	return width;
}

void TiGtkUserWindow::SetWidth(double width) {
	int height = GetHeight();
	gtk_window_resize(this->gtk_window, int(width), height);
	this->config->SetWidth(width);
}

double TiGtkUserWindow::GetHeight() {
	int width, height;
	gtk_window_get_size (this->gtk_window, &width, &height);
	return width;
}

void TiGtkUserWindow::SetHeight(double height) {
	int width = GetWidth();
	gtk_window_resize(this->gtk_window, width, int(height));
	this->config->SetHeight(height);
}

TiBounds TiGtkUserWindow::GetBounds() {
	int width, height;
	int x, y;
	gtk_window_get_size(this->gtk_window, &width, &height);
	gtk_window_get_position(this->gtk_window, &x, &y);
	TiBounds b = { x, y, width, height };
	return b;
}

void TiGtkUserWindow::SetBounds(TiBounds b) {
	gtk_window_resize(this->gtk_window, int(b.width), int(b.height));
	gtk_window_move(this->gtk_window, int(b.x), int(b.y));

	this->config->SetX(b.x);
	this->config->SetY(b.y);
	this->config->SetWidth(b.width);
	this->config->SetHeight(b.height);
}

std::string TiGtkUserWindow::GetTitle() {
	return std::string(gtk_window_get_title(this->gtk_window));
}

void TiGtkUserWindow::SetTitle(std::string title) {
	gtk_window_set_title (this->gtk_window, title.c_str());
	this->config->SetTitle(title);
}

std::string TiGtkUserWindow::GetUrl() {
	return this->config->GetURL();
}

void TiGtkUserWindow::SetUrl(std::string uri) {
	webkit_web_view_open (this->web_view, uri.c_str());
	this->config->SetURL(uri);
}

void TiGtkUserWindow::SetupDecorations() {
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

	gdk_window_set_decorations(gdk_window, (GdkWMDecoration) d);
}

bool TiGtkUserWindow::IsResizable() {
	return gtk_window_get_resizable(this->gtk_window);
}

void TiGtkUserWindow::SetResizable(bool resizable) {
	gtk_window_set_resizable(this->gtk_window, resizable);
	this->config->SetResizable(resizable);
}

bool TiGtkUserWindow::IsMaximizable() {
	return this->config->IsMaximizable();
}

void TiGtkUserWindow::SetMaximizable(bool maximizable) {
	this->config->SetMaximizable(maximizable);
	this->SetupDecorations();
}

bool TiGtkUserWindow::IsMinimizable() {
	return this->config->IsMinimizable();
}

void TiGtkUserWindow::SetMinimizable(bool minimizable) {
	this->config->SetMinimizable(minimizable);
	this->SetupDecorations();
}

bool TiGtkUserWindow::IsCloseable() {
	return gtk_window_get_deletable(this->gtk_window);
}
void TiGtkUserWindow::SetCloseable(bool closeable) {
	this->config->SetCloseable(closeable);
	gtk_window_set_deletable(this->gtk_window, closeable);
}

bool TiGtkUserWindow::IsVisible() {
	return this->config->IsVisible();
}

void TiGtkUserWindow::SetVisible(bool visible) {
	if (visible) {
		this->Show();
	} else {
		this->Hide();
	}
}

double TiGtkUserWindow::GetTransparency() {
	return gtk_window_get_opacity(this->gtk_window);
}

void TiGtkUserWindow::SetTransparency(double alpha) {
	return gtk_window_set_opacity(this->gtk_window, alpha);
	this->config->SetTransparency(alpha);
}
