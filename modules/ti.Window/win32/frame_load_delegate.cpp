/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "frame_load_delegate.h"
#include "win32_user_window.h"
#include "../window_module.h"
#include "../../../kroll/modules/javascript/javascript_module.h"

using namespace ti;
using namespace kroll;

Win32FrameLoadDelegate::Win32FrameLoadDelegate(Win32UserWindow *window_) : window(window_), ref_count(1) {
	// TODO Auto-generated constructor stub

}

HRESULT STDMETHODCALLTYPE
Win32FrameLoadDelegate::windowScriptObjectAvailable (
		IWebView *webView, JSContextRef context, JSObjectRef windowScriptObject)
{
	JSObjectRef global_object = JSContextGetGlobalObject(context);
	Host* tihost = window->GetHost();

	// Produce a delegating object to represent the top-level
	// Titanium object. When a property isn't found in this object
	// it will look for it in global_tibo.
	SharedBoundObject global_tibo = tihost->GetGlobalObject();
	BoundObject* ti_object = new DelegateStaticBoundObject(global_tibo);
	SharedBoundObject shared_ti_obj = SharedBoundObject(ti_object);

	// Set user window into the Titanium object
	SharedBoundObject* shared_user_window = new SharedBoundObject(window);
	SharedValue user_window_val = Value::NewObject(*shared_user_window);
	BoundObject *current_window = new StaticBoundObject();
	SharedBoundObject shared_current_window(current_window);
	shared_current_window->Set("window", user_window_val);

	SharedValue current_window_val = Value::NewObject(shared_current_window);
	ti_object->Set("currentWindow", current_window_val);

	// Place the Titanium object into the window's global object
	BoundObject *global_bound_object = new KJSBoundObject(context, global_object);
	SharedValue ti_object_value = Value::NewObject(shared_ti_obj);
	global_bound_object->Set(GLOBAL_NS_VARNAME, ti_object_value);

	return S_OK;
}

HRESULT STDMETHODCALLTYPE
Win32FrameLoadDelegate::QueryInterface(REFIID riid, void **ppvObject)
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
Win32FrameLoadDelegate::AddRef()
{
	return ++ref_count;
}

ULONG STDMETHODCALLTYPE
Win32FrameLoadDelegate::Release()
{
	ULONG new_count = --ref_count;
	if (!new_count) delete(this);

	return new_count;
}
