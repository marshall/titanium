/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "webkit_frame_load_delegate.h"
#include "win32_user_window.h"
#include "../ui_module.h"
#include "../../../kroll/modules/javascript/javascript_module.h"

using namespace ti;
using namespace kroll;

Win32WebKitFrameLoadDelegate::Win32WebKitFrameLoadDelegate(Win32UserWindow *window_) : window(window_), ref_count(1) {
	// TODO Auto-generated constructor stub

}

HRESULT STDMETHODCALLTYPE
Win32WebKitFrameLoadDelegate::didFinishLoadForFrame(IWebView *webView, IWebFrame *frame)
{
	window->FrameLoaded();
	return S_OK;
}

HRESULT STDMETHODCALLTYPE
Win32WebKitFrameLoadDelegate::windowScriptObjectAvailable (
		IWebView *webView, JSContextRef context, JSObjectRef windowScriptObject)
{
	JSObjectRef global_object = JSContextGetGlobalObject(context);
	KJSUtil::RegisterGlobalContext(global_object, (JSGlobalContextRef) context);

	Win32UserWindow* user_window = this->window;
	Host* tihost = window->GetHost();

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

		// Place currentWindow.openFiles in the delegate.
		SharedValue open_files_value = user_window->Get("openFiles");
		delegate_ui_api->Set("openFiles", open_files_value);

		ti_object->Set("UI", Value::NewObject(delegate_ui_api));
	}
	else
	{
		std::cerr << "Could not find UI API point!" << std::endl;
	}

	// Get the global object into a KJSBoundObject
	BoundObject *global_bound_object = new KJSBoundObject(context, global_object);

	// Copy the document and window properties to the Titanium object
	SharedValue doc_value = global_bound_object->Get("document");
	ti_object->Set("document", doc_value);
	SharedValue window_value = global_bound_object->Get("window");
	ti_object->Set("window", window_value);

	// Place the Titanium object into the window's global object
	SharedValue ti_object_value = Value::NewObject(shared_ti_obj);
	global_bound_object->Set(GLOBAL_NS_VARNAME, ti_object_value);


	/*

	// Set user window into the Titanium object
	SharedBoundObject* shared_user_window = new SharedBoundObject(window);
	SharedValue user_window_val = Value::NewObject(*shared_user_window);
	BoundObject *current_window = new StaticBoundObject();
	SharedBoundObject shared_current_window(current_window);
	shared_current_window->Set("window", user_window_val);
	*/

	return S_OK;
}

HRESULT STDMETHODCALLTYPE
Win32WebKitFrameLoadDelegate::QueryInterface(REFIID riid, void **ppvObject)
{
	*ppvObject = 0;
	if (IsEqualGUID(riid, IID_IUnknown)) {
		*ppvObject = static_cast<IWebFrameLoadDelegate*>(this);
	}
	else if (IsEqualGUID(riid, IID_IWebFrameLoadDelegate)) {
		*ppvObject = static_cast<IWebFrameLoadDelegate*>(this);
	}
	else {
		return E_NOINTERFACE;
	}
	return S_OK;
}

ULONG STDMETHODCALLTYPE
Win32WebKitFrameLoadDelegate::AddRef()
{
	return ++ref_count;
}

ULONG STDMETHODCALLTYPE
Win32WebKitFrameLoadDelegate::Release()
{
	ULONG new_count = --ref_count;
	if (!new_count) delete(this);

	return new_count;
}
