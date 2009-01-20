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
	kroll::Host* tihost = window->GetHost();

	// Bind all child objects to global context
	SharedPtr<StaticBoundObject> global_tibo = tihost->GetGlobalObject();
	SharedPtr<StaticBoundObject> tiObject = new StaticBoundObject();

	SharedStringList prop_names = global_tibo->GetPropertyNames();
	for (size_t i = 0; i < prop_names->size(); i++)
	{
		SharedString name = prop_names->at(i);
		SharedValue value = global_tibo->Get(name->c_str());
		tiObject->Set(name->c_str(), value);
	}

	// set user window into the Titanium object
	SharedBoundObject w = this->window;
	SharedValue user_window_val = Value::NewObject(w);
	SharedValue tiWindow = tiObject->Get("Window");

	if (!tiWindow.isNull() && tiWindow->IsObject())
		tiWindow->ToObject()->Set("currentWindow", user_window_val);

	// place Titanium object into the window's global object
	SharedBoundObject global_bound_object = new KJSBoundObject(context, global_object);
	SharedValue tiObjectValue = Value::NewObject(tiObject);
	global_bound_object->Set(PRODUCT_NAME, tiObjectValue);

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
