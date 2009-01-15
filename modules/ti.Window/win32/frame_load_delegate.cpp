/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "frame_load_delegate.h"
#include "win32_user_window.h"
#include "window_module.h"

using namespace ti;

Win32FrameLoadDelegate::Win32FrameLoadDelegate(Win32UserWindow *window_) : window(window_), ref_count(1) {
	// TODO Auto-generated constructor stub

}

HRESULT STDMETHODCALLTYPE
Win32FrameLoadDelegate::windowScriptObjectAvailable (
		IWebView *webView, JSContextRef context, JSObjectRef windowScriptObject)
{

	// Bind all child objects to global context
	BoundObject* global_tibo = (StaticBoundObject*) tihost->GetGlobalObject();
	BoundObject* tiObject = new StaticBoundObject();

	std::vector<const char *> prop_names;
	global_tibo->GetPropertyNames(&prop_names);
	for (size_t i = 0; i < prop_names.size(); i++)
	{
		const char *name = prop_names.at(i);
		Value* value = global_tibo->Get(name);
		ScopedDereferencer s0(value);
		tiObject->Set(name, value);
	}

	// set user window into the Titanium object
	Value *user_window_val = new Value(this->window);
	ScopedDereferencer s1(user_window_val);
	tiObject->Set("currentWindow", user_window_val);
	KR_DECREF(user_window_val);

	// place Titanium object into the window's global object
	BoundObject *global_bound_object = new KJSBoundObject(context, global_object);
	Value *tiObjectValue = new Value(tiObject);
	ScopedDereferencer s2(tiObjectValue);
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
