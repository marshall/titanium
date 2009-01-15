/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "window_module.h"
#include <stdlib.h>

using namespace ti;

#define STUB printf("__FILE__:__LINE__: called a stub method!\n")

std::vector<UserWindow *> UserWindow::windows;
void UserWindow::Open(UserWindow *window)
{
	// Don't do any refcounting here,
	// since we are holding onto our copy
	windows.push_back(window);
}
void UserWindow::Close(UserWindow *window)
{
	// delete from vector
	std::vector<UserWindow*>::iterator iter;
	for (iter = windows.begin(); iter != windows.end(); iter++) {
		if ((*iter) == window) {
			break;
		}
	}
	if (iter != windows.end()) {
		windows.erase(iter);
	}

	KR_DECREF(window);

}

UserWindow::UserWindow(kroll::Host *host, WindowConfig *config) : kroll::StaticBoundObject() {

	this->host = host;
	this->config = config;

	/* this object is accessed by tiRuntime.Window.currentWindow */
	this->SetMethod("hide", &UserWindow::hide_cb);
	this->SetMethod("show", &UserWindow::show_cb);
	this->SetMethod("isUsingChrome", &UserWindow::is_using_chrome_cb);
	this->SetMethod("isFullScreen", &UserWindow::is_full_screen_cb);
	this->SetMethod("setFullScreen", &UserWindow::set_full_screen_cb);
	this->SetMethod("getId", &UserWindow::get_id_cb);
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
	this->SetMethod("getUrl", &UserWindow::get_url_cb);
	this->SetMethod("setUrl", &UserWindow::set_url_cb);
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
}

void UserWindow::hide_cb(const kroll::ValueList& args, kroll::Value *result)
{
	this->Hide();
}

void UserWindow::show_cb(const kroll::ValueList& args, kroll::Value *result)
{
	this->Show();
}

void UserWindow::is_using_chrome_cb(const kroll::ValueList& args, kroll::Value *result)
{
	result->Set(this->IsUsingChrome());
}

void UserWindow::is_using_scrollbars_cb(const kroll::ValueList& args, kroll::Value *result)
{
	result->Set(this->IsUsingScrollbars());
}

void UserWindow::is_full_screen_cb(const kroll::ValueList& args, kroll::Value *result)
{
	result->Set(this->IsFullScreen());
}

void UserWindow::set_full_screen_cb(const kroll::ValueList& args, kroll::Value *result)
{
	this->SetFullScreen(args.at(0)->ToBool());
}

void UserWindow::get_id_cb(const kroll::ValueList& args, kroll::Value *result)
{
	result->Set(this->GetId());
}

void UserWindow::open_cb(const kroll::ValueList& args, kroll::Value *result)
{
	this->Open();
}

void UserWindow::close_cb(const kroll::ValueList& args, kroll::Value *result)
{
	this->Close();
}

void UserWindow::get_x_cb(const kroll::ValueList& args, kroll::Value *result)
{
	result->Set(this->GetX());
}

void UserWindow::set_x_cb(const kroll::ValueList& args, kroll::Value *result)
{
	if (args.size() > 0) {
		this->SetX(args.at(0)->ToDouble());
	}
}

void UserWindow::get_y_cb(const kroll::ValueList& args, kroll::Value *result)
{
	result->Set(this->GetY());
}

void UserWindow::set_y_cb(const kroll::ValueList& args, kroll::Value *result)
{
	if (args.size() > 0) {
		this->SetY(args.at(0)->ToDouble());
	}
}

void UserWindow::get_width_cb(const kroll::ValueList& args, kroll::Value *result)
{
	result->Set(this->GetWidth());
}

void UserWindow::set_width_cb(const kroll::ValueList& args, kroll::Value *result)
{
	if (args.size() > 0) {
		this->SetWidth(args.at(0)->ToDouble());
	}
}

void UserWindow::get_height_cb(const kroll::ValueList& args, kroll::Value *result)
{
	result->Set(this->GetHeight());
}

void UserWindow::set_height_cb(const kroll::ValueList& args, kroll::Value *result)
{
	if (args.size() > 0) {
		this->SetHeight(args.at(0)->ToDouble());
	}
}

void UserWindow::get_bounds_cb(const kroll::ValueList& args, kroll::Value *result)
{
	Bounds bounds = this->GetBounds();
	kroll::StaticBoundObject *b = new kroll::StaticBoundObject();
	b->Set("x", new kroll::Value(bounds.x));
	b->Set("y", new kroll::Value(bounds.y));
	b->Set("width", new kroll::Value(bounds.width));
	b->Set("height", new kroll::Value(bounds.height));
	result->Set(b);

	KR_DECREF(b);
}

void UserWindow::set_bounds_cb(const kroll::ValueList& args, kroll::Value *result)
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

		KR_DECREF(x);
		KR_DECREF(y);
		KR_DECREF(width);
		KR_DECREF(height);

		this->SetBounds(bounds);
	}
}

void UserWindow::get_title_cb(const kroll::ValueList& args, kroll::Value *result)
{
	result->Set(this->GetTitle());
}

void UserWindow::set_title_cb(const kroll::ValueList& args, kroll::Value *result)
{
	if (args.size() > 0) {
		std::string title = args.at(0)->ToString();
		this->SetTitle(title);
	}
}

void UserWindow::get_url_cb(const kroll::ValueList& args, kroll::Value *result)
{
	result->Set(this->GetUrl());
}

void UserWindow::set_url_cb(const kroll::ValueList& args, kroll::Value *result)
{
	if (args.size() > 0) {
		std::string url = args.at(0)->ToString();
		this->SetUrl(url);
	}
}

void UserWindow::is_resizable_cb(const kroll::ValueList& args, kroll::Value *result)
{
	result->Set(this->IsResizable());
}

void UserWindow::set_resizable_cb(const kroll::ValueList& args, kroll::Value *result)
{
	if (args.size() > 0) {
		this->SetResizable(args.at(0)->ToBool());
	}
}

void UserWindow::is_maximizable_cb(const kroll::ValueList& args, kroll::Value *result)
{
	result->Set(this->IsMaximizable());
}

void UserWindow::set_maximizable_cb(const kroll::ValueList& args, kroll::Value *result)
{
	if (args.size() > 0) {
		this->SetMaximizable(args.at(0)->ToBool());
	}
}

void UserWindow::is_minimizable_cb(const kroll::ValueList& args, kroll::Value *result)
{
	result->Set(this->IsMinimizable());
}

void UserWindow::set_minimizable_cb(const kroll::ValueList& args, kroll::Value *result)
{
	if (args.size() > 0) {
		this->SetMinimizable(args.at(0)->ToBool());
	}
}

void UserWindow::is_closeable_cb(const kroll::ValueList& args, kroll::Value *result)
{
	result->Set(this->IsCloseable());
}

void UserWindow::set_closeable_cb(const kroll::ValueList& args, kroll::Value *result)
{
	if (args.size() > 0) {
		this->SetCloseable(args.at(0)->ToBool());
	}
}

void UserWindow::is_visible_cb(const kroll::ValueList& args, kroll::Value *result)
{
	result->Set(this->IsVisible());
}

void UserWindow::set_visible_cb(const kroll::ValueList& args, kroll::Value *result)
{
	if (args.size() > 0) {
		this->SetVisible(args.at(0)->ToBool());
	}
}

void UserWindow::get_transparency_cb(const kroll::ValueList& args, kroll::Value *result)
{
	result->Set(this->GetTransparency());
}

void UserWindow::set_transparency_cb(const kroll::ValueList& args, kroll::Value *result)
{
	if (args.size() > 0) {
		this->SetTransparency(args.at(0)->ToDouble());
	}
}
