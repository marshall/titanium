/**
 * Appcelerator Titanium - licensed under the Apache Public License 2
 * see LICENSE in the root folder for details on the license.
 * Copyright (c) 2008 Appcelerator, Inc. All Rights Reserved.
 */

#include "frame_load_delegate.h"
#include "win32_user_window.h"
#include "../binding/kjs.h"

using namespace ti;

Win32FrameLoadDelegate::Win32FrameLoadDelegate(Win32UserWindow *window_) : window(window_), ref_count(1) {
	// TODO Auto-generated constructor stub

}

HRESULT STDMETHODCALLTYPE
Win32FrameLoadDelegate::windowScriptObjectAvailable (
		IWebView *webView, JSContextRef context, JSObjectRef windowScriptObject)
{
	JSObjectRef global_object = JSContextGetGlobalObject(context);
	kroll::Host* tihost = window->GetHost();
	kroll::BoundObject* global_tibo = (kroll::StaticBoundObject*) tihost->GetGlobalObject();

	// place window into the context local for currentWindow
	kroll::StaticBoundObject *context_local = GetContextLocal(context);

	// set user window into the context
	kroll::Value *user_window_val = new kroll::Value(window);
	context_local->Set("currentWindow", user_window_val);
	KR_DECREF(user_window_val);

	// Bind all child objects to global context
	std::vector<std::string> prop_names = global_tibo->GetPropertyNames();
	for (size_t i = 0; i < prop_names.size(); i++)
	{
		const char *name = prop_names.at(i).c_str();
		kroll::Value* value = global_tibo->Get(name, context_local);

		JSValueRef js_value = KrollValueToJSValue(context, value);
		BindPropertyToJSObject(context, global_object, name, js_value);
	}

	/*
	JSStringRef script;
	char code[1024];
	sprintf(code, "var o = Object(); foo.o = o; alert(foo.o); foo.b = foo.o; alert(foo.b);");
	//sprintf(code, "var o = Object(); o.prop = 'one'; foo.prop = 'two'; var f = function() {alert(this.prop);}; o.f = f; o.f(); foo.f = o.f; foo.f();");
	script = JSStringCreateWithUTF8CString(code);
	if(JSCheckScriptSyntax(context, script, NULL, 0, NULL))
		JSEvaluateScript(context, script, window_object, NULL, 1, NULL);
	JSStringRelease(script);*/

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
