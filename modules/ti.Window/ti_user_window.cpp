/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "windowing_plugin.h"
#include <stdlib.h>

#define STUB printf("__FILE__:__LINE__: called a stub method!\n")

std::vector<TiUserWindow *> TiUserWindow::windows;
void TiUserWindow::Open(TiUserWindow *window)
{
	// Don't do any refcounting here,
	// since we are holding onto our copy
	windows.push_back(window);
}
void TiUserWindow::Close(TiUserWindow *window)
{
	// delete from vector
	std::vector<TiUserWindow*>::iterator iter;
	for (iter = windows.begin(); iter != windows.end(); iter++) {
		if ((*iter) == window) {
			break;
		}
	}
	if (iter != windows.end()) {
		windows.erase(iter);
	}

	TI_DECREF(window);

}

TiUserWindow::TiUserWindow(TiHost *host, TiWindowConfig *config) : TiStaticBoundObject() {

	this->host = host;
	this->config = config;

	/* this object is accessed by tiRuntime.Window.currentWindow */
	this->SetMethod("hide", &TiUserWindow::hide_cb);
	this->SetMethod("show", &TiUserWindow::show_cb);
	this->SetMethod("isUsingChrome", &TiUserWindow::is_using_chrome_cb);
	this->SetMethod("isFullScreen", &TiUserWindow::is_full_screen_cb);
	this->SetMethod("getId", &TiUserWindow::get_id_cb);
	this->SetMethod("open", &TiUserWindow::open_cb);
	this->SetMethod("close", &TiUserWindow::close_cb);
	this->SetMethod("getX", &TiUserWindow::get_x_cb);
	this->SetMethod("setX", &TiUserWindow::set_x_cb);
	this->SetMethod("getY", &TiUserWindow::get_y_cb);
	this->SetMethod("setY", &TiUserWindow::set_y_cb);
	this->SetMethod("getWidth", &TiUserWindow::get_width_cb);
	this->SetMethod("setWidth", &TiUserWindow::set_width_cb);
	this->SetMethod("getHeight", &TiUserWindow::get_height_cb);
	this->SetMethod("setHeight", &TiUserWindow::set_height_cb);
	this->SetMethod("getBounds", &TiUserWindow::get_bounds_cb);
	this->SetMethod("setBounds", &TiUserWindow::set_bounds_cb);
	this->SetMethod("getTitle", &TiUserWindow::get_title_cb);
	this->SetMethod("setTitle", &TiUserWindow::set_title_cb);
	this->SetMethod("getUrl", &TiUserWindow::get_url_cb);
	this->SetMethod("setUrl", &TiUserWindow::set_url_cb);
	this->SetMethod("isResizable", &TiUserWindow::is_resizable_cb);
	this->SetMethod("setResizable", &TiUserWindow::set_resizable_cb);
	this->SetMethod("isMaximizable", &TiUserWindow::is_maximizable_cb);
	this->SetMethod("setMaimizable", &TiUserWindow::set_maximizable_cb);
	this->SetMethod("isMinimizable", &TiUserWindow::is_minimizable_cb);
	this->SetMethod("setMinizable", &TiUserWindow::set_minimizable_cb);
	this->SetMethod("isCloseable", &TiUserWindow::is_closeable_cb);
	this->SetMethod("setCloseable", &TiUserWindow::set_closeable_cb);
	this->SetMethod("isVisible", &TiUserWindow::is_visible_cb);
	this->SetMethod("setVisible", &TiUserWindow::set_visible_cb);
	this->SetMethod("getTransparency", &TiUserWindow::get_transparency_cb);
	this->SetMethod("setTransparency", &TiUserWindow::set_transparency_cb);
}

void TiUserWindow::hide_cb(const TiValueList& args,
                           TiValue *result,
                           TiBoundObject* context)
{
	this->Hide();
}

void TiUserWindow::show_cb(const TiValueList& args,
                           TiValue *result,
                           TiBoundObject* context)
{
	this->Show();
}

void TiUserWindow::is_using_chrome_cb(const TiValueList& args,
                                      TiValue *result,
                                      TiBoundObject* context)
{
	result->Set(this->IsUsingChrome());
}

void TiUserWindow::is_using_scrollbars_cb(const TiValueList& args,
                                          TiValue *result,
                                          TiBoundObject* context)
{
	result->Set(this->IsUsingScrollbars());
}

void TiUserWindow::is_full_screen_cb(const TiValueList& args,
                                     TiValue *result,
                                     TiBoundObject* context)
{
	result->Set(this->IsFullScreen());
}

void TiUserWindow::get_id_cb(const TiValueList& args,
                             TiValue *result,
                             TiBoundObject* context)
{
	result->Set(this->GetId());
}

void TiUserWindow::open_cb(const TiValueList& args,
                           TiValue *result,
                           TiBoundObject* context)
{
	this->Open();
}

void TiUserWindow::close_cb(const TiValueList& args,
                            TiValue *result,
                            TiBoundObject* context)
{
	this->Close();
}

void TiUserWindow::get_x_cb(const TiValueList& args,
                            TiValue *result,
                            TiBoundObject* context)
{
	result->Set(this->GetX());
}

void TiUserWindow::set_x_cb(const TiValueList& args,
                            TiValue *result,
                            TiBoundObject* context)
{
	if (args.size() > 0) {
		this->SetX(args.at(0)->ToDouble());
	}
}

void TiUserWindow::get_y_cb(const TiValueList& args,
                            TiValue *result,
                            TiBoundObject* context)
{
	result->Set(this->GetY());
}

void TiUserWindow::set_y_cb(const TiValueList& args,
                            TiValue *result,
                            TiBoundObject* context)
{
	if (args.size() > 0) {
		this->SetY(args.at(0)->ToDouble());
	}
}

void TiUserWindow::get_width_cb(const TiValueList& args,
                                TiValue *result,
                                TiBoundObject* context)
{
	result->Set(this->GetWidth());
}

void TiUserWindow::set_width_cb(const TiValueList& args,
                                TiValue *result,
                                TiBoundObject* context)
{
	if (args.size() > 0) {
		this->SetWidth(args.at(0)->ToDouble());
	}
}

void TiUserWindow::get_height_cb(const TiValueList& args, 
                                 TiValue *result,
                                 TiBoundObject* context)
{
	result->Set(this->GetHeight());
}

void TiUserWindow::set_height_cb(const TiValueList& args, 
                                 TiValue *result,
                                 TiBoundObject* context)
{
	if (args.size() > 0) {
		this->SetHeight(args.at(0)->ToDouble());
	}
}

void TiUserWindow::get_bounds_cb(const TiValueList& args, 
                                 TiValue *result,
                                 TiBoundObject* context)
{
	TiBounds bounds = this->GetBounds();
	TiStaticBoundObject *b = new TiStaticBoundObject();
	b->Set("x", new TiValue(bounds.x));
	b->Set("y", new TiValue(bounds.y));
	b->Set("width", new TiValue(bounds.width));
	b->Set("height", new TiValue(bounds.height));
	result->Set(b);

	TI_DECREF(b);
}

void TiUserWindow::set_bounds_cb(const TiValueList& args, 
                                 TiValue *result,
                                 TiBoundObject* context)
{
	if (args.size() > 0 && args.at(0)->IsObject())
	{
		TiBounds bounds;
		TiBoundObject* o = args[0]->ToObject();

		TiValue* x = o->Get("x", context);
		TiValue* y = o->Get("y", context);
		TiValue* width = o->Get("width", context);
		TiValue* height = o->Get("height", context);

		bounds.x = x->ToInt();
		bounds.y = y->ToInt();
		bounds.width = width->ToInt();
		bounds.height = height->ToInt();

		TI_DECREF(x);
		TI_DECREF(y);
		TI_DECREF(width);
		TI_DECREF(height);

		this->SetBounds(bounds);
	}
}

void TiUserWindow::get_title_cb(const TiValueList& args, 
                                TiValue *result,
                                TiBoundObject* context)
{
	result->Set(this->GetTitle());
}

void TiUserWindow::set_title_cb(const TiValueList& args, 
                                TiValue *result,
                                TiBoundObject* context)
{
	if (args.size() > 0) {
		this->SetTitle(args.at(0)->ToString());
	}
}

void TiUserWindow::get_url_cb(const TiValueList& args, 
                              TiValue *result,
                              TiBoundObject* context)
{
	result->Set(this->GetUrl());
}

void TiUserWindow::set_url_cb(const TiValueList& args, 
                              TiValue *result,
                              TiBoundObject* context)
{
	if (args.size() > 0) {
		this->SetUrl(args.at(0)->ToString());
	}
}

void TiUserWindow::is_resizable_cb(const TiValueList& args, 
                                   TiValue *result,
                                   TiBoundObject* context)
{
	result->Set(this->IsResizable());
}

void TiUserWindow::set_resizable_cb(const TiValueList& args, 
                                    TiValue *result,
                                    TiBoundObject* context)
{
	if (args.size() > 0) {
		this->SetResizable(args.at(0)->ToBool());
	}
}

void TiUserWindow::is_maximizable_cb(const TiValueList& args, 
                                     TiValue *result,
                                     TiBoundObject* context)
{
	result->Set(this->IsMaximizable());
}

void TiUserWindow::set_maximizable_cb(const TiValueList& args, 
                                      TiValue *result,
                                      TiBoundObject* context)
{
	if (args.size() > 0) {
		this->SetMaximizable(args.at(0)->ToBool());
	}
}

void TiUserWindow::is_minimizable_cb(const TiValueList& args, 
                                     TiValue *result,
                                     TiBoundObject* context)
{
	result->Set(this->IsMinimizable());
}

void TiUserWindow::set_minimizable_cb(const TiValueList& args,
                                      TiValue *result,
                                      TiBoundObject* context)
{
	if (args.size() > 0) {
		this->SetMinimizable(args.at(0)->ToBool());
	}
}

void TiUserWindow::is_closeable_cb(const TiValueList& args,
                                   TiValue *result,
                                   TiBoundObject* context)
{
	result->Set(this->IsCloseable());
}

void TiUserWindow::set_closeable_cb(const TiValueList& args, 
                                    TiValue *result,
                                    TiBoundObject* context)
{
	if (args.size() > 0) {
		this->SetCloseable(args.at(0)->ToBool());
	}
}

void TiUserWindow::is_visible_cb(const TiValueList& args, 
                                 TiValue *result,
                                 TiBoundObject* context)
{
	result->Set(this->IsVisible());
}

void TiUserWindow::set_visible_cb(const TiValueList& args, 
                                  TiValue *result,
                                  TiBoundObject* context)
{
	if (args.size() > 0) {
		this->SetVisible(args.at(0)->ToBool());
	}
}

void TiUserWindow::get_transparency_cb(const TiValueList& args, 
                                       TiValue *result,
                                       TiBoundObject* context)
{
	result->Set(this->GetTransparency());
}

void TiUserWindow::set_transparency_cb(const TiValueList& args, 
                                       TiValue *result,
                                       TiBoundObject* context)
{
	if (args.size() > 0) {
		this->SetTransparency(args.at(0)->ToDouble());
	}
}
