/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

/* This object is that represents Titanium.UI.currentWindow */

#include "ui_module.h"
#include <stdlib.h>

#ifdef OS_WIN32
	#include "win32/win32_user_window.h"
#elif OS_OSX
	#include "osx/osx_user_window.h"
#elif OS_LINUX
	#include "gtk/gtk_user_window.h"
#endif


using namespace ti;
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

// Initialize our constants here
int UserWindow::CENTERED = WindowConfig::DEFAULT_POSITION;

UserWindow::UserWindow(SharedUIBinding binding, WindowConfig *config, SharedUserWindow& parent) :
	kroll::StaticBoundObject(),
	binding(binding),
	host(binding->GetHost()),
	config(config),
	parent(parent),
	next_listener_id(0),
	closed(false)
{
	this->shared_this = this;

	this->Set("CENTERED", Value::NewInt(UserWindow::CENTERED));
	this->SetMethod("hide", &UserWindow::_Hide);
	this->SetMethod("show", &UserWindow::_Show);
	this->SetMethod("focus", &UserWindow::_Focus);
	this->SetMethod("unfocus", &UserWindow::_Unfocus);
	this->SetMethod("isUsingChrome", &UserWindow::_IsUsingChrome);
	this->SetMethod("setUsingChrome", &UserWindow::_SetUsingChrome);
	this->SetMethod("isFullScreen", &UserWindow::_IsFullScreen);
	this->SetMethod("setFullScreen", &UserWindow::_SetFullScreen);
	this->SetMethod("getID", &UserWindow::_GetId);
	this->SetMethod("open", &UserWindow::_Open);
	this->SetMethod("close", &UserWindow::_Close);
	this->SetMethod("getX", &UserWindow::_GetX);
	this->SetMethod("setX", &UserWindow::_SetX);
	this->SetMethod("getY", &UserWindow::_GetY);
	this->SetMethod("setY", &UserWindow::_SetY);
	this->SetMethod("getWidth", &UserWindow::_GetWidth);
	this->SetMethod("setWidth", &UserWindow::_SetWidth);
	this->SetMethod("getMaxWidth", &UserWindow::_GetMaxWidth);
	this->SetMethod("setMaxWidth", &UserWindow::_SetMaxWidth);
	this->SetMethod("getMinWidth", &UserWindow::_GetMinWidth);
	this->SetMethod("setMinWidth", &UserWindow::_SetMinWidth);
	this->SetMethod("getHeight", &UserWindow::_GetHeight);
	this->SetMethod("setHeight", &UserWindow::_SetHeight);
	this->SetMethod("getMaxHeight", &UserWindow::_GetMaxHeight);
	this->SetMethod("setMaxHeight", &UserWindow::_SetMaxHeight);
	this->SetMethod("getMinHeight", &UserWindow::_GetMinHeight);
	this->SetMethod("setMinHeight", &UserWindow::_SetMinHeight);
	this->SetMethod("getBounds", &UserWindow::_GetBounds);
	this->SetMethod("setBounds", &UserWindow::_SetBounds);
	this->SetMethod("getTitle", &UserWindow::_GetTitle);
	this->SetMethod("setTitle", &UserWindow::_SetTitle);
	this->SetMethod("getURL", &UserWindow::_GetURL);
	this->SetMethod("setURL", &UserWindow::_SetURL);
	this->SetMethod("isResizable", &UserWindow::_IsResizable);
	this->SetMethod("setResizable", &UserWindow::_SetResizable);
	this->SetMethod("isMaximizable", &UserWindow::_IsMaximizable);
	this->SetMethod("setMaximizable", &UserWindow::_SetMaximizable);
	this->SetMethod("isMinimizable", &UserWindow::_IsMinimizable);
	this->SetMethod("setMinimizable", &UserWindow::_SetMinimizable);
	this->SetMethod("isCloseable", &UserWindow::_IsCloseable);
	this->SetMethod("setCloseable", &UserWindow::_SetCloseable);
	this->SetMethod("isVisible", &UserWindow::_IsVisible);
	this->SetMethod("setVisible", &UserWindow::_SetVisible);
	this->SetMethod("getTransparency", &UserWindow::_GetTransparency);
	this->SetMethod("setTransparency", &UserWindow::_SetTransparency);
	this->SetMethod("getTransparencyColor", &UserWindow::_GetTransparencyColor);
	this->SetMethod("setMenu", &UserWindow::_SetMenu);
	this->SetMethod("getMenu", &UserWindow::_GetMenu);
	this->SetMethod("setContextMenu", &UserWindow::_SetContextMenu);
	this->SetMethod("getContextMenu", &UserWindow::_GetContextMenu);
	this->SetMethod("setIcon", &UserWindow::_SetIcon);
	this->SetMethod("getIcon", &UserWindow::_GetIcon);
	this->SetMethod("setTopMost", &UserWindow::_SetTopMost);
	this->SetMethod("isTopMost", &UserWindow::_IsTopMost);
	this->SetMethod("createWindow", &UserWindow::_CreateWindow);
	this->SetMethod("openFiles", &UserWindow::_OpenFiles);
	this->SetMethod("getParent", &UserWindow::_GetParent);
	this->SetMethod("addEventListener", &UserWindow::_AddEventListener);
	this->SetMethod("removeEventListener", &UserWindow::_RemoveEventListener);

	this->api = host->GetGlobalObject()->GetNS("API.fire")->ToMethod();
	this->FireEvent(CREATE);
}

UserWindow::~UserWindow()
{
}

SharedUserWindow UserWindow::GetSharedPtr()
{
	return this->shared_this;
}

Host* UserWindow::GetHost()
{
	return this->host;
}

SharedUIBinding UserWindow::GetBinding()
{
	return this->binding;
}

void UserWindow::Open()
{
	this->FireEvent(OPEN);

	// We are now in the UI binding's open window list
	this->binding->AddToOpenWindows(this->shared_this);

	// Tell the parent window that we are open for business
	if (!parent.isNull())
		parent->AddChild(this->shared_this);
}

void UserWindow::Close()
{
	if (this->closed)
		return;
	else
		this->closed = true;

	SharedUserWindow shthis = this->shared_this;
	this->FireEvent(CLOSE);

	// Close all children and cleanup
	std::vector<SharedUserWindow>::iterator iter = this->children.begin();
	while (iter != this->children.end())
	{
		// Save a pointer to the child here, because it may
		// be freed by the SharedPtr otherwise and that will
		// make this iterator seriously, seriously unhappy.
		SharedUserWindow child = (*iter);
		iter = children.erase(iter);
		child->Close();
	}

	// Tell our parent that we are now closed
	if (!this->parent.isNull())
	{
		this->parent->RemoveChild(this->shared_this);
		this->parent->Focus(); // Focus the parent
	}

	// Tell the UIBinding that we are closed
	this->binding->RemoveFromOpenWindows(this->shared_this);

	// When we have no more open windows, we exit...
	std::vector<SharedUserWindow> windows = this->binding->GetOpenWindows();
	if (windows.size() == 0)
		this->host->Exit(0);
	else
		windows.at(0)->Focus();

	// This should be the last reference to this window
	// after all external references are destroyed.
	this->shared_this = NULL;
}


void UserWindow::_Hide(const kroll::ValueList& args, kroll::SharedValue result)
{
	this->Hide();
}

void UserWindow::_Show(const kroll::ValueList& args, kroll::SharedValue result)
{
	this->Show();
}

void UserWindow::_Focus(const kroll::ValueList& args, kroll::SharedValue result)
{
	this->Focus();
}

void UserWindow::_Unfocus(const kroll::ValueList& args, kroll::SharedValue result)
{
	this->Unfocus();
}

void UserWindow::_IsUsingChrome(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetBool(this->IsUsingChrome());
}

void UserWindow::_SetTopMost(const kroll::ValueList& args, kroll::SharedValue result)
{
	TI_SET_BOOL(SetTopMost, 0);
}

void UserWindow::_IsTopMost(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetBool(this->IsTopMost());
}

void UserWindow::_SetUsingChrome(const kroll::ValueList& args, kroll::SharedValue result)
{
	TI_SET_BOOL(SetUsingChrome, 0);
}

void UserWindow::_IsUsingScrollbars(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetBool(this->IsUsingScrollbars());
}

void UserWindow::_IsFullScreen(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetBool(this->IsFullScreen());
}

void UserWindow::_SetFullScreen(const kroll::ValueList& args, kroll::SharedValue result)
{
	TI_SET_BOOL(SetFullScreen, 0);
}

void UserWindow::_GetId(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetString(this->GetId().c_str());
}

void UserWindow::_Open(const kroll::ValueList& args, kroll::SharedValue result)
{
	this->Open();
}

void UserWindow::_Close(const kroll::ValueList& args, kroll::SharedValue result)
{
	this->Close();
}

void UserWindow::_GetX(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetDouble(this->GetX());
}

void UserWindow::_SetX(const kroll::ValueList& args, kroll::SharedValue result)
{
	if (args.size() > 0) {
		this->SetX(args.at(0)->ToDouble());
	}
}

void UserWindow::_GetY(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetDouble(this->GetY());
}

void UserWindow::_SetY(const kroll::ValueList& args, kroll::SharedValue result)
{
	if (args.size() > 0) {
		this->SetY(args.at(0)->ToDouble());
	}
}

void UserWindow::_GetWidth(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetDouble(this->GetWidth());
}

void UserWindow::_SetWidth(const kroll::ValueList& args, kroll::SharedValue result)
{
	if (args.size() > 0) {
		this->SetWidth(args.at(0)->ToDouble());
	}
}

void UserWindow::_GetMinWidth(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetDouble(this->GetMinWidth());
}

void UserWindow::_SetMinWidth(const kroll::ValueList& args, kroll::SharedValue result)
{
	if (args.size() > 0) {
		this->SetMinWidth(args.at(0)->ToDouble());
	}
}

void UserWindow::_GetMaxWidth(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetDouble(this->GetMaxWidth());
}

void UserWindow::_SetMaxWidth(const kroll::ValueList& args, kroll::SharedValue result)
{
	if (args.size() > 0) {
		this->SetMaxWidth(args.at(0)->ToDouble());
	}
}

void UserWindow::_GetHeight(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetDouble(this->GetHeight());
}

void UserWindow::_SetHeight(const kroll::ValueList& args, kroll::SharedValue result)
{
	if (args.size() > 0) {
		this->SetHeight(args.at(0)->ToDouble());
	}
}

void UserWindow::_GetMinHeight(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetDouble(this->GetMinHeight());
}

void UserWindow::_SetMinHeight(const kroll::ValueList& args, kroll::SharedValue result)
{
	if (args.size() > 0) {
		this->SetMinHeight(args.at(0)->ToDouble());
	}
}

void UserWindow::_GetMaxHeight(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetDouble(this->GetMaxHeight());
}

void UserWindow::_SetMaxHeight(const kroll::ValueList& args, kroll::SharedValue result)
{
	if (args.size() > 0) {
		this->SetMaxHeight(args.at(0)->ToDouble());
	}
}

void UserWindow::_GetBounds(const kroll::ValueList& args, kroll::SharedValue result)
{
	Bounds bounds = this->GetBounds();
	kroll::StaticBoundObject *b = new kroll::StaticBoundObject();
	b->Set("x", kroll::Value::NewInt(bounds.x));
	b->Set("y", kroll::Value::NewInt(bounds.y));
	b->Set("width", kroll::Value::NewInt(bounds.width));
	b->Set("height", kroll::Value::NewInt(bounds.height));
	result->SetObject(b);
}

void UserWindow::_SetBounds(const kroll::ValueList& args, kroll::SharedValue result)
{
	if (args.size() > 0 && args.at(0)->IsObject())
	{
		Bounds bounds;

		kroll::BoundObject* o = args[0]->ToObject();
		bounds.x = o->Get("x")->ToInt();
		bounds.y = o->Get("y")->ToInt();
		bounds.width = o->Get("width")->ToInt();
		bounds.height = o->Get("height")->ToInt();
		this->SetBounds(bounds);
	}
}

void UserWindow::_GetTitle(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetString(this->GetTitle().c_str());
}

void UserWindow::_SetTitle(const kroll::ValueList& args, kroll::SharedValue result)
{
	if (args.size() > 0) {
		std::string title = args.at(0)->ToString();
		this->SetTitle(title);
	}
}

void UserWindow::_GetURL(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetString(this->GetURL().c_str());
}

void UserWindow::_SetURL(const kroll::ValueList& args, kroll::SharedValue result)
{
	if (args.size() > 0 && args.at(0)->IsString()) {
		std::string url = args.at(0)->ToString();
		url = AppConfig::Instance()->InsertAppIDIntoURL(url);
		this->SetURL(url);
	}
}

void UserWindow::_IsResizable(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetBool(this->IsResizable());
}

void UserWindow::_SetResizable(const kroll::ValueList& args, kroll::SharedValue result)
{
	TI_SET_BOOL(SetResizable, 0);
}

void UserWindow::_IsMaximizable(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetBool(this->IsMaximizable());
}

void UserWindow::_SetMaximizable(const kroll::ValueList& args, kroll::SharedValue result)
{
	TI_SET_BOOL(SetMaximizable, 0);
}

void UserWindow::_IsMinimizable(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetBool(this->IsMinimizable());
}

void UserWindow::_SetMinimizable(const kroll::ValueList& args, kroll::SharedValue result)
{
	TI_SET_BOOL(SetMinimizable, 0);
}

void UserWindow::_IsCloseable(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetBool(this->IsCloseable());
}

void UserWindow::_SetCloseable(const kroll::ValueList& args, kroll::SharedValue result)
{
	TI_SET_BOOL(SetCloseable, 0);
}

void UserWindow::_IsVisible(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetBool(this->IsVisible());
}

void UserWindow::_SetVisible(const kroll::ValueList& args, kroll::SharedValue result)
{
	TI_SET_BOOL(SetVisible, 0);
}

void UserWindow::_GetTransparency(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetDouble(this->GetTransparency());
}

void UserWindow::_SetTransparency(const kroll::ValueList& args, kroll::SharedValue result)
{
	if (args.size() > 0) {
		this->SetTransparency(args.at(0)->ToDouble());
	}
}

void UserWindow::_GetTransparencyColor(const kroll::ValueList& args, kroll::SharedValue result)
{
	std::string color = this->GetTransparencyColor();
	result->SetString(color);
}

void UserWindow::_SetMenu(const kroll::ValueList& args, kroll::SharedValue result)
{
	SharedPtr<MenuItem> menu = NULL; // A NULL value is an unset
	if (args.size() > 0 && args.at(0)->IsList())
	{
		menu = args.at(0)->ToList().cast<MenuItem>();
	}
	this->SetMenu(menu);
}

void UserWindow::_GetMenu(const kroll::ValueList& args, kroll::SharedValue result)
{
	SharedBoundList menu = this->GetMenu();
	if (!menu.isNull())
	{
		result->SetList(menu);
	}
	else
	{
		result->SetUndefined();
	}
}

void UserWindow::_SetContextMenu(const kroll::ValueList& args, kroll::SharedValue result)
{
	SharedPtr<MenuItem> menu = NULL; // A NULL value is an unset
	if (args.size() > 0 && args.at(0)->IsList())
	{
		menu = args.at(0)->ToList().cast<MenuItem>();
	}
	this->SetContextMenu(menu);
}

void UserWindow::_GetContextMenu(const kroll::ValueList& args, kroll::SharedValue result)
{
	SharedBoundList menu = this->GetContextMenu();
	if (!menu.isNull())
	{
		result->SetList(menu);
	}
	else
	{
		result->SetUndefined();
	}
}

void UserWindow::_SetIcon(const kroll::ValueList& args, kroll::SharedValue result)
{
	SharedString icon_path = NULL; // a NULL value is an unset
	if (args.size() > 0 && args.at(0)->IsString())
	{
		const char *icon_url = args.at(0)->ToString();
		icon_path = UIModule::GetResourcePath(icon_url);
	}
	this->SetIcon(icon_path);
}

void UserWindow::_GetIcon(const kroll::ValueList& args, kroll::SharedValue result)
{
	result->SetString(this->GetIcon()->c_str());
}

void UserWindow::_GetParent(const kroll::ValueList& args, kroll::SharedValue result)
{
	if (this->parent.isNull())
	{
		result->SetNull();
	}
	else
	{
		result->SetObject(this->parent);
	}
}

void UserWindow::_CreateWindow(const ValueList& args, SharedValue result)
{
	//TODO: wrap in sharedptr
	WindowConfig *config = NULL;

	if (args.size() > 0 && args.at(0)->IsObject())
	{
		SharedBoundObject props = SharedBoundObject(new StaticBoundObject());
		config = new WindowConfig();
		props = args.at(0)->ToObject();
		config->UseProperties(props);
	}
	else if (args.size() > 0 && args.at(0)->IsString())
	{
		// String might match a url spec
		std::string url = args.at(0)->ToString();
		WindowConfig* matchedConfig = AppConfig::Instance()->GetWindowByURL(url);

		url = AppConfig::Instance()->InsertAppIDIntoURL(url);
		config = new WindowConfig(matchedConfig, url);
	}
	else
	{
		config = new WindowConfig();
	}

	SharedUserWindow new_window = this->binding->CreateWindow(config, shared_this);
	result->SetObject(new_window);
}

void UserWindow::UpdateWindowForURL(std::string url)
{
	WindowConfig* config = AppConfig::Instance()->GetWindowByURL(url);
	if (!config)
	{
		// no need to update window
		return;
	}

	// copy the config object
	config = new WindowConfig(config, url);

	Bounds b;
	b.x = config->GetX();
	b.y = config->GetY();
	b.width = config->GetWidth();
	b.height = config->GetHeight();

	this->SetBounds(b);

	this->SetMinimizable(config->IsMinimizable());
	this->SetMaximizable(config->IsMaximizable());
	this->SetCloseable(config->IsCloseable());
}

void UserWindow::_OpenFiles(const ValueList& args, SharedValue result)
{
	// pass in a set of properties with each key being
	// the name of the property and a boolean for its setting
	// example:
	//
	// var selected = Titanium.Desktop.openFiles({
	//    multiple:true,
	//    files:false,
	//    directories:true,
	//    types:['js','html']
	// });
	//
	//
	SharedBoundMethod callback;
	if (args.size() < 1 || !args.at(0)->IsMethod())
	{
		throw ValueException::FromString("openFiles expects first argument to be a callback");
	}
	callback = args.at(0)->ToMethod();

	SharedBoundObject props;
	if (args.size() < 2 || !args.at(1)->IsObject())
	{
		props = new StaticBoundObject();
	}
	else
	{
		props = args.at(1)->ToObject();
	}

	bool files = props->GetBool("files", true);
	bool multiple = props->GetBool("multiple", false);
	bool directories = props->GetBool("directories", false);
	std::string path = props->GetString("path", "");
	std::string file = props->GetString("file", "");

	std::vector<std::string> types;
	if (props->Get("types")->IsList())
	{
		SharedBoundList l = props->Get("types")->ToList();
		for (unsigned int i = 0; i < l->Size(); i++)
		{
			if (l->At(i)->IsString())
			{
				types.push_back(l->At(i)->ToString());
			}
		}
	}

	this->OpenFiles(callback, multiple, files, directories, path, file, types);
}

void UserWindow::_AddEventListener(const ValueList& args, SharedValue result)
{
	ArgUtils::VerifyArgsException("addEventListener", args, "m");

	SharedBoundMethod target = args.at(0)->ToMethod();
	Listener listener = Listener();
	listener.id = this->next_listener_id++;
	listener.callback = target;
	this->listeners.push_back(listener);

	result->SetInt(listener.id);
}

void UserWindow::_RemoveEventListener(const ValueList& args, SharedValue result)
{
	if (args.size() != 1 || !args.at(0)->IsNumber())
	{
		throw ValueException::FromString("invalid argument");
	}
	int id = args.at(0)->ToInt();

	std::vector<Listener>::iterator it = this->listeners.begin();
	while (it != this->listeners.end())
	{
		if ((*it).id == id)
		{
			this->listeners.erase(it);
			result->SetBool(true);
			return;
		}
		it++;
	}
	result->SetBool(false);
}

void UserWindow::FireEvent(UserWindowEvent event_type, SharedKObject event)
{
	std::string name;
	switch (event_type)
	{
		case FOCUSED:
		{
			name = "focused";
			break;
		}
		case UNFOCUSED:
		{
			name = "unfocused";
			break;
		}
		case OPEN:
		{
			name = "open";
			break;
		}
		case OPENED:
		{
			name = "opened";
			break;
		}
		case CLOSE:
		{
			name = "close";
			break;
		}
		case CLOSED:
		{
			name = "closed";
			break;
		}
		case HIDDEN:
		{
			name = "hidden";
			break;
		}
		case SHOWN:
		{
			name = "shown";
			break;
		}
		case FULLSCREENED:
		{
			name = "fullscreened";
			break;
		}
		case UNFULLSCREENED:
		{
			name = "unfullscreened";
			break;
		}
		case MAXIMIZED:
		{
			name = "maximized";
			break;
		}
		case MINIMIZED:
		{
			name = "minimized";
			break;
		}
		case RESIZED:
		{
			name = "resized";
			break;
		}
		case MOVED:
		{
			name = "moved";
			break;
		}
		case INIT:
		{
			name = "page.init";
			break;
		}
		case LOAD:
		{
			name = "page.load";
			break;
		}
		case CREATE:
		{
			name = "window.create";
			break;
		}
	}

	std::string en("ti.UI.window.");
	en+=name;

	if (event.isNull())
		event = new StaticBoundObject();

	event->Set("window", Value::NewObject(this->shared_this));
	this->api->Call(en.c_str(), Value::NewObject(event));

	// If we don't have listeners here, we can just bail.
	if (this->listeners.size() == 0)
		return;

	ValueList args;
	args.push_back(Value::NewString(name));
	std::vector<Listener>::iterator it = this->listeners.begin();
	while (it != this->listeners.end())
	{
		SharedBoundMethod callback = (*it).callback;
		try
		{
			this->host->InvokeMethodOnMainThread(callback,args);
		}
		catch(std::exception &e)
		{
			std::cerr << "Caught exception dispatching window event callback: " << event << ", Error: " << e.what() << std::endl;
		}
		it++;
	}
}

SharedUserWindow UserWindow::GetParent()
{
	return this->parent;
}

void UserWindow::AddChild(SharedUserWindow child)
{
	this->children.push_back(child);
}

void UserWindow::RemoveChild(SharedUserWindow child)
{
	std::vector<SharedUserWindow>::iterator iter = this->children.begin();
	while (iter != this->children.end())
	{
		if ((*iter).get() == child.get())
			iter = children.erase(iter);
		else
			iter++;
	}
}

void UserWindow::ContextBound(SharedBoundObject global_bound_object)
{
	SharedBoundObject event = new StaticBoundObject();
	event->Set("scope", Value::NewObject(global_bound_object));
	this->FireEvent(INIT, event);
}

void UserWindow::PageLoaded(SharedBoundObject global_bound_object, std::string &url)
{
	SharedBoundObject event = new StaticBoundObject();
	event->Set("scope", Value::NewObject(global_bound_object));
	event->Set("url", Value::NewString(url.c_str()));
	this->FireEvent(LOAD, event);
}
