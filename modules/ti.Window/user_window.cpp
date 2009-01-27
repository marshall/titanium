/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "window_module.h"
#include <stdlib.h>

using namespace ti;

#define STUB printf("__FILE__:__LINE__: called a stub method!\n")

#define TI_SET_BOOL(method, index) \
{ \
	SharedValue arg = args.at(index);\
	if (arg->IsBool()) \
	{ \
		this->method(arg->ToBool()); \
	}\
	else if (arg->IsInt()) \
	{ \
		this->method(arg->ToInt()==1); \
	} \
	else if (arg->IsString()) \
	{ \
		std::string yn(arg->ToString()); \
		if (yn=="yes" || yn=="true" || yn=="1") \
		{ \
			this->method(true); \
		} \
		else \
		{ \
			this->method(false); \
		} \
	} \
}


std::vector<UserWindow *> UserWindow::windows;
std::map<UserWindow*, std::vector<UserWindow*> > UserWindow::windowsMap;

void UserWindow::Open(UserWindow *window)
{
	// Don't do any refcounting here,
	// since we are holding onto our copy
	windows.push_back(window);
}
void UserWindow::Close(UserWindow *window)
{
	// check to see if we have a parent, and if so,
	// remove us from the parent list
	UserWindow *parent = window->get_parent();
	if (parent!=NULL)
	{
		UserWindow::RemoveChild(parent,window);
	}

	// see if we have any open child windows
	std::vector<UserWindow*> children = windowsMap[window];
	std::vector<UserWindow*>::iterator iter;
	for (iter = children.begin(); iter != children.end(); iter++)
	{
		UserWindow *child = (*iter);
		child->Close();
	}
	children.clear();
	windowsMap.erase(window);


	// delete from vector
	for (iter = windows.begin(); iter != windows.end(); iter++)
	{
		if ((*iter) == window)
		{
			break;
		}
	}
	if (iter != windows.end())
	{
		windows.erase(iter);
	}

	// when we have no more windows, we exit ...
	if (windows.size()==0)
	{
#if defined(OS_OSX)
		[NSApp terminate:nil];
#elif defined(OS_WIN32)
		ExitProcess(0);
#else
		//TODO: in Win32, do we exit some other way??
		exit(0);
#endif
	}
}
void UserWindow::AddChild(UserWindow *parent, UserWindow *child)
{
	std::vector<UserWindow*> children = windowsMap[parent];
	children.push_back(child);
	windowsMap[parent]=children;
}
void UserWindow::RemoveChild(UserWindow *parent, UserWindow *child)
{
	std::vector<UserWindow*> children = windowsMap[parent];
	std::vector<UserWindow*>::iterator iter;
	for (iter = children.begin(); iter != children.end(); iter++) {
		if ((*iter) == child) {
			break;
		}
	}
	if (iter != children.end()) {
		children.erase(iter);
	}
}

UserWindow::UserWindow(kroll::Host *host, WindowConfig *config) :
	kroll::StaticBoundObject(), parent(0)
{

	this->host = host;
	this->config = config;

	/* this object is accessed by Titanium.Window.currentWindow */
	this->SetMethod("hide", &UserWindow::hide_cb);
	this->SetMethod("show", &UserWindow::show_cb);
	this->SetMethod("isUsingChrome", &UserWindow::is_using_chrome_cb);
	this->SetMethod("setUsingChrome", &UserWindow::set_using_chrome_cb);
	this->SetMethod("isFullscreen", &UserWindow::is_full_screen_cb);
	this->SetMethod("setFullscreen", &UserWindow::set_full_screen_cb);
	this->SetMethod("getID", &UserWindow::get_id_cb);
	this->SetMethod("open", &UserWindow::open_cb);
	this->SetMethod("close", &UserWindow::close_cb);
	this->SetMethod("getX", &UserWindow::get_x_cb);
	this->SetMethod("setX", &UserWindow::set_x_cb);
	this->SetMethod("getY", &UserWindow::get_y_cb);
	this->SetMethod("setY", &UserWindow::set_y_cb);
	this->SetMethod("getWidth", &UserWindow::get_width_cb);
	this->SetMethod("setWidth", &UserWindow::set_width_cb);
	this->SetMethod("getHeight", &UserWindow::get_height_cb);
	this->SetMethod("setHeight", &UserWindow::set_height_cb);
	this->SetMethod("getBounds", &UserWindow::get_bounds_cb);
	this->SetMethod("setBounds", &UserWindow::set_bounds_cb);
	this->SetMethod("getTitle", &UserWindow::get_title_cb);
	this->SetMethod("setTitle", &UserWindow::set_title_cb);
	this->SetMethod("getURL", &UserWindow::get_url_cb);
	this->SetMethod("setURL", &UserWindow::set_url_cb);
	this->SetMethod("isResizable", &UserWindow::is_resizable_cb);
	this->SetMethod("setResizable", &UserWindow::set_resizable_cb);
	this->SetMethod("isMaximizable", &UserWindow::is_maximizable_cb);
	this->SetMethod("setMaimizable", &UserWindow::set_maximizable_cb);
	this->SetMethod("isMinimizable", &UserWindow::is_minimizable_cb);
	this->SetMethod("setMinizable", &UserWindow::set_minimizable_cb);
	this->SetMethod("isCloseable", &UserWindow::is_closeable_cb);
	this->SetMethod("setCloseable", &UserWindow::set_closeable_cb);
	this->SetMethod("isVisible", &UserWindow::is_visible_cb);
	this->SetMethod("setVisible", &UserWindow::set_visible_cb);
	this->SetMethod("getTransparency", &UserWindow::get_transparency_cb);
	this->SetMethod("setTransparency", &UserWindow::set_transparency_cb);
	this->SetMethod("setMenu", &UserWindow::set_menu_cb);
	this->SetMethod("getParent", &UserWindow::get_parent_cb);
}

void UserWindow::hide_cb(const kroll::ValueList& args, kroll::SharedValue result)
{
	this->Hide();
}

void UserWindow::show_cb(const kroll::ValueList& args, kroll::SharedValue result)
{
	this->Show();
}

void UserWindow::is_using_chrome_cb(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetBool(this->IsUsingChrome());
}

void UserWindow::set_using_chrome_cb(const kroll::ValueList& args, kroll::SharedValue result)
{
	TI_SET_BOOL(SetUsingChrome, 0);
}

void UserWindow::is_using_scrollbars_cb(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetBool(this->IsUsingScrollbars());
}

void UserWindow::is_full_screen_cb(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetBool(this->IsFullScreen());
}

void UserWindow::set_full_screen_cb(const kroll::ValueList& args, kroll::SharedValue result)
{
	TI_SET_BOOL(SetFullScreen, 0);
}

void UserWindow::get_id_cb(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetString(this->GetId().c_str());
}

void UserWindow::open_cb(const kroll::ValueList& args, kroll::SharedValue result)
{
	this->Open();
}

void UserWindow::close_cb(const kroll::ValueList& args, kroll::SharedValue result)
{
	this->Close();
}

void UserWindow::get_x_cb(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetDouble(this->GetX());
}

void UserWindow::set_x_cb(const kroll::ValueList& args, kroll::SharedValue result)
{
	if (args.size() > 0) {
		this->SetX(args.at(0)->ToDouble());
	}
}

void UserWindow::get_y_cb(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetDouble(this->GetY());
}

void UserWindow::set_y_cb(const kroll::ValueList& args, kroll::SharedValue result)
{
	if (args.size() > 0) {
		this->SetY(args.at(0)->ToDouble());
	}
}

void UserWindow::get_width_cb(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetDouble(this->GetWidth());
}

void UserWindow::set_width_cb(const kroll::ValueList& args, kroll::SharedValue result)
{
	if (args.size() > 0) {
		this->SetWidth(args.at(0)->ToDouble());
	}
}

void UserWindow::get_height_cb(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetDouble(this->GetHeight());
}

void UserWindow::set_height_cb(const kroll::ValueList& args, kroll::SharedValue result)
{
	if (args.size() > 0) {
		this->SetHeight(args.at(0)->ToDouble());
	}
}

void UserWindow::get_bounds_cb(const kroll::ValueList& args, kroll::SharedValue result)
{
	Bounds bounds = this->GetBounds();
	kroll::StaticBoundObject *b = new kroll::StaticBoundObject();
	b->Set("x", kroll::Value::NewInt(bounds.x));
	b->Set("y", kroll::Value::NewInt(bounds.y));
	b->Set("width", kroll::Value::NewInt(bounds.width));
	b->Set("height", kroll::Value::NewInt(bounds.height));
	result->SetObject(b);
}

void UserWindow::set_bounds_cb(const kroll::ValueList& args, kroll::SharedValue result)
{
	if (args.size() > 0 && args.at(0)->IsObject())
	{
		Bounds bounds;
		kroll::BoundObject* o = args[0]->ToObject();

		kroll::Value* x = o->Get("x");
		kroll::Value* y = o->Get("y");
		kroll::Value* width = o->Get("width");
		kroll::Value* height = o->Get("height");

		bounds.x = x->ToInt();
		bounds.y = y->ToInt();
		bounds.width = width->ToInt();
		bounds.height = height->ToInt();

		this->SetBounds(bounds);
	}
}

void UserWindow::get_title_cb(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetString(this->GetTitle().c_str());
}

void UserWindow::set_title_cb(const kroll::ValueList& args, kroll::SharedValue result)
{
	if (args.size() > 0) {
		std::string title = args.at(0)->ToString();
		this->SetTitle(title);
	}
}

void UserWindow::get_url_cb(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetString(this->GetURL().c_str());
}

void UserWindow::set_url_cb(const kroll::ValueList& args, kroll::SharedValue result)
{
	if (args.size() > 0) {
		std::string url = args.at(0)->ToString();
		this->SetURL(url);
	}
}

void UserWindow::is_resizable_cb(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetBool(this->IsResizable());
}

void UserWindow::set_resizable_cb(const kroll::ValueList& args, kroll::SharedValue result)
{
	TI_SET_BOOL(SetResizable, 0);
}

void UserWindow::is_maximizable_cb(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetBool(this->IsMaximizable());
}

void UserWindow::set_maximizable_cb(const kroll::ValueList& args, kroll::SharedValue result)
{
	TI_SET_BOOL(SetMaximizable, 0);
}

void UserWindow::is_minimizable_cb(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetBool(this->IsMinimizable());
}

void UserWindow::set_minimizable_cb(const kroll::ValueList& args, kroll::SharedValue result)
{
	TI_SET_BOOL(SetMinimizable, 0);
}

void UserWindow::is_closeable_cb(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetBool(this->IsCloseable());
}

void UserWindow::set_closeable_cb(const kroll::ValueList& args, kroll::SharedValue result)
{
	TI_SET_BOOL(SetCloseable, 0);
}

void UserWindow::is_visible_cb(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetBool(this->IsVisible());
}

void UserWindow::set_visible_cb(const kroll::ValueList& args, kroll::SharedValue result)
{
	TI_SET_BOOL(SetVisible, 0);
}

void UserWindow::get_transparency_cb(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetDouble(this->GetTransparency());
}

void UserWindow::set_transparency_cb(const kroll::ValueList& args, kroll::SharedValue result)
{
	if (args.size() > 0) {
		this->SetTransparency(args.at(0)->ToDouble());
	}
}

void UserWindow::set_menu_cb(const kroll::ValueList& args, kroll::SharedValue result)
{
	SharedBoundObject val = args.at(0)->ToObject();
	printf("%x\n", (int) val.get());
	if (args.size() > 0 && args.at(0)->IsList()) {
		SharedBoundList menu = args.at(0)->ToList();
		this->SetMenu(menu);
	}
}
void UserWindow::get_parent_cb(const kroll::ValueList& args, kroll::SharedValue result)
{
	if (this->parent==NULL)
	{
		result->SetNull();
		return;
	}
	SharedBoundObject o = this->get_parent();
	result->SetObject(o);
}
void UserWindow::set_parent(UserWindow *parent)
{
	this->parent = parent;
	parent->add_child(this);
}
UserWindow* UserWindow::get_parent()
{
	return this->parent;
}
void UserWindow::add_child(UserWindow *window)
{
	UserWindow::AddChild(this,window);
}
void UserWindow::remove_child(UserWindow *window)
{
	UserWindow::RemoveChild(this,window);
}
