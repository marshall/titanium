/**
 * Appcelerator Kroll - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license. 
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */	
#include <kroll/kroll.h>
#include "window_binding.h"


#define TI_WIN_SET_STR(name,method,def) \
{\
	SharedValue v = properties->Get(#name); \
	if (v->IsString()) \
	{ \
		try  \
		{  \
			std::string value(v->ToString()); \
			config->method(value); \
		} \
		catch (...) \
		{ \
		} \
	} \
	else \
	{ \
		config->method(def); \
	} \
}

#define TI_WIN_SET_BOOL(name,method,def) \
{\
	SharedValue v = properties->Get(#name); \
	if (v->IsString()) \
	{ \
		try  \
		{  \
			std::string value(v->ToString()); \
			if (value=="yes" || value=="1" || value=="true" || value=="1") \
			{ \
				config->method(true); \
			}\
			else \
			{ \
				config->method(false); \
			} \
		} \
		catch (...) \
		{ \
		} \
	} \
	else if (v->IsInt()) \
	{ \
		config->method(v->ToInt()==1); \
	} \
	else if (v->IsBool()) \
	{ \
		config->method(v->ToBool()); \
	} \
	else \
	{ \
		config->method(def); \
	} \
}

#define TI_WIN_SET_INT(name,method,def) \
{\
	SharedValue v = properties->Get(#name); \
	if (v->IsInt()) \
	{ \
		config->method(v->ToInt()); \
	} \
	if (v->IsDouble()) \
	{ \
		config->method(v->ToDouble()); \
	} \
	else if (v->IsString()) \
	{ \
		try  \
		{  \
			int value = atoi(v->ToString()); \
			config->method(value); \
		} \
		catch (...) \
		{ \
		} \
	} \
	else \
	{ \
		config->method(def); \
	} \
}

#define TI_WIN_SET_DOUBLE(name,method,def) \
{\
	SharedValue v = properties->Get(#name); \
	if (v->IsInt()) \
	{ \
		config->method((double)v->ToInt()); \
	} \
	if (v->IsDouble()) \
	{ \
		config->method(v->ToDouble()); \
	} \
	else if (v->IsString()) \
	{ \
		try  \
		{  \
			float value = atof(v->ToString()); \
			config->method((double)value); \
		} \
		catch (...) \
		{ \
		} \
	} \
	else \
	{ \
		config->method(def); \
	} \
}

static int ti_window_count = 0;

namespace ti
{
	WindowBinding::WindowBinding(Host *host, SharedBoundObject global) : host(host),global(global)
	{
		KR_ADDREF(host);
		this->SetMethod("createWindow",&WindowBinding::CreateWindow);
	}
	WindowBinding::~WindowBinding()
	{
		KR_DECREF(host);
	}
	void WindowBinding::CreateWindow(const ValueList& args, SharedValue result)
	{
		SharedBoundObject properties = args.size()==1 && args.at(0)->IsObject() ? args.at(0)->ToObject() : new StaticBoundObject();
		result->SetObject(this->CreateWindow(properties));
	}
	SharedBoundObject WindowBinding::CreateWindow(SharedBoundObject properties)
	{
		//TODO: wrap in sharedptr
		WindowConfig *config = new WindowConfig();
		
		char winid[80];
		sprintf(winid,"win_%d",++ti_window_count);
		std::string id(winid);
		std::string url;
		std::string title;
		
		TI_WIN_SET_STR(id,SetID,id);
		TI_WIN_SET_STR(url,SetURL,url);
		TI_WIN_SET_STR(title,SetTitle,title);
		TI_WIN_SET_INT(x,SetX,0);
		TI_WIN_SET_INT(y,SetY,0);
		TI_WIN_SET_INT(width,SetWidth,500);
		TI_WIN_SET_INT(minWidth,SetMinWidth,0);
		TI_WIN_SET_INT(maxWidth,SetMaxWidth,9999);
		TI_WIN_SET_INT(height,SetHeight,600);
		TI_WIN_SET_INT(minHeight,SetMinHeight,0);
		TI_WIN_SET_INT(maxHeight,SetMaxHeight,9999);
		TI_WIN_SET_BOOL(visible,SetVisible,true);
		TI_WIN_SET_BOOL(maximizable,SetMaximizable,true);
		TI_WIN_SET_BOOL(minimizable,SetMinimizable,true);
		TI_WIN_SET_BOOL(closeable,SetCloseable,true);
		TI_WIN_SET_BOOL(resizable,SetResizable,true);
		TI_WIN_SET_BOOL(fullscreen,SetFullScreen,false);
		TI_WIN_SET_BOOL(usingChrome,SetUsingChrome,false);
		TI_WIN_SET_BOOL(usingScrollbars,SetUsingScrollbars,true);
		TI_WIN_SET_DOUBLE(transparency,SetTransparency,1.0);
		

#if defined(OS_LINUX)
		GtkUserWindow* window = new GtkUserWindow(this->host, config);
#elif defined(OS_OSX)
		OSXUserWindow* window = new OSXUserWindow(this->host, config);
#elif defined(OS_WIN32)
		Win32UserWindow* window = new Win32UserWindow(this->host, config);
#endif
		return window;
	}
}
