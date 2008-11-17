/*
* Copyright 2006-2008 Appcelerator, Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "ti_web_view_delegate.h"
#include "ti_web_shell.h"
#include "ti_utils.h"

#include "Resource.h"

TIWebViewDelegate::TIWebViewDelegate(TIWebShell *ti_web_shell) : bootstrapTitanium(false) {
	this->ti_web_shell = ti_web_shell;
}

TIWebViewDelegate::~TIWebViewDelegate() {
}

void TIWebViewDelegate::setHost(WebViewHost* host) {
	this->host = host;
}

void TIWebViewDelegate::setHWND(HWND hWnd) {
	this->hWnd = hWnd;
}

gfx::ViewHandle TIWebViewDelegate::GetContainingWindow(WebWidget* webwidget) {
	ti_debug("::: GetContainingWindow");
	if (host != NULL) return host->window_handle();
	return NULL;
}

// Called when a region of the WebWidget needs to be re-painted.
void TIWebViewDelegate::DidInvalidateRect(WebWidget* webwidget, const gfx::Rect& rect) {

	ti_debug("::: Did Invalidate Rect");
	if (host != NULL) host->DidInvalidateRect(rect);

}

// Called when a region of the WebWidget, given by clip_rect, should be
// scrolled by the specified dx and dy amounts.
void TIWebViewDelegate::DidScrollRect(WebWidget* webwidget, int dx, int dy,
	const gfx::Rect& clip_rect) {

		ti_debug(":::Did Scroll Rect");
		host->DidScrollRect(dx, dy, clip_rect);
}

// This method is called to instruct the window containing the WebWidget to
// show itself as the topmost window.  This method is only used after a
// successful call to CreateWebWidget.  |disposition| indicates how this new
// window should be displayed, but generally only means something for WebViews.
void TIWebViewDelegate::Show(WebWidget* webwidget, WindowOpenDisposition disposition) {
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
	ti_debug("::::::: SHOW WINDOW");
}

// This method is called to instruct the window containing the WebWidget to
// close.  Note: This method should just be the trigger that causes the
// WebWidget to eventually close.  It should not actually be destroyed until
// after this call returns.
void TIWebViewDelegate::CloseWidgetSoon(WebWidget* webwidget) {
	PostMessage(hWnd, WM_CLOSE, 0, 0);
}

// This method is called to focus the window containing the WebWidget so
// that it receives keyboard events.
void TIWebViewDelegate::Focus(WebWidget* webwidget) {
	SetFocus(hWnd);

	ti_debug("::::::::::::FOCUS WINDOW");
}

// This method is called to unfocus the window containing the WebWidget so that
// it no longer receives keyboard events.
void TIWebViewDelegate::Blur(WebWidget* webwidget) {
	if (::GetFocus() == hWnd) { SetFocus(NULL); }
}

void TIWebViewDelegate::SetCursor(WebWidget* webwidget, 
	const WebCursor& cursor) {

}

// Returns the rectangle of the WebWidget in screen coordinates.
void TIWebViewDelegate::GetWindowRect(WebWidget* webwidget, gfx::Rect* out_rect) {
	RECT rect;
	::GetWindowRect(host->window_handle(), &rect);
	*out_rect = gfx::Rect(rect);

	ti_debug("::::::::: GET WINDOW RECT");
}


// This method is called to re-position the WebWidget on the screen.  The given
// rect is in screen coordinates.  The implementation may choose to ignore
// this call or modify the given rect.  This method may be called before Show
// has been called.
// TODO(darin): this is more of a request; does this need to take effect
// synchronously?
void TIWebViewDelegate::SetWindowRect(WebWidget* webwidget, const gfx::Rect& rect) {

}

// Returns the rectangle of the window in which this WebWidget is embeded in.
void TIWebViewDelegate::GetRootWindowRect(WebWidget* webwidget, gfx::Rect* out_rect) {
	ti_debug("::: Get Root Window Rect");
	RECT rect;
	HWND root_window = ::GetAncestor(host->window_handle(), GA_ROOT);
	::GetWindowRect(root_window, &rect);
	*out_rect = gfx::Rect(rect);
}

// Keeps track of the necessary window move for a plugin window that resulted
// from a scroll operation.  That way, all plugin windows can be moved at the
// same time as each other and the page.
void TIWebViewDelegate::DidMove(WebWidget* webwidget, const WebPluginGeometry& move) {
	WebPluginDelegateImpl::MoveWindow(
		move.window, move.window_rect, move.clip_rect, move.cutout_rects,
		move.visible);
}

// Suppress input events to other windows, and do not return until the widget
// is closed.  This is used to support |window.showModalDialog|.
void TIWebViewDelegate::RunModal(WebWidget* webwidget){

}

// Owners depend on the delegates living as long as they do, so we ref them.
void TIWebViewDelegate::AddRef() {
	//base::RefCounted<TIWebViewDelegate>::AddRef();
}

void TIWebViewDelegate::Release() {
	//base::RefCounted<TIWebViewDelegate>::Release();
}

// Returns true if the widget is in a background tab.
bool TIWebViewDelegate::IsHidden() {
	return false;
}

void TIWebViewDelegate::WindowObjectCleared(WebFrame *webFrame)
{
	if (bootstrapTitanium) {
		bootstrapTitanium = false;

		ti_native = new TiNative(ti_web_shell);
		ti_native->BindToJavascript(webFrame, L"TiNative");
		std::string titanium_js = "ti:///titanium.js";
		ti_web_shell->include(titanium_js);
	}
}

WebPluginDelegate* TIWebViewDelegate::CreatePluginDelegate(
		WebView *webview, const GURL &url,
		const std::string &mime_type, const std::string &clsid,
		std::string *actual_mime_type)
{
	bool allow_wildcard = true;
	WebPluginInfo info;
	if (!NPAPI::PluginList::Singleton()->GetPluginInfo(url, mime_type, clsid,
		allow_wildcard, &info,
		actual_mime_type))
		return NULL;

	if (actual_mime_type && !actual_mime_type->empty())
		return WebPluginDelegateImpl::Create(info.file, *actual_mime_type, host->window_handle());
	else {
		return WebPluginDelegateImpl::Create(info.file, mime_type, host->window_handle());
	}
}

void TIWebViewDelegate::AddMessageToConsole(WebView* webview,
	const std::wstring& message,
	unsigned int line_no,
	const std::wstring& source_id) {

	if (source_id.size() == 0 && line_no == 0) {
		printf("[ti:info] %ls\n", message.c_str());
	}
	else {
		printf("[ti:error] %ls:%d Line %ls\n", source_id.c_str(), line_no, message.c_str());
	}
	
}

void TIWebViewDelegate::DidFinishLoadForFrame(WebView* webview, WebFrame* frame) {

}

// Displays a JavaScript alert panel associated with the given view. Clients
// should visually indicate that this panel comes from JavaScript. The panel
// should have a single OK button.
void TIWebViewDelegate::RunJavaScriptAlert(WebView* webview, const std::wstring& message) {
	MessageBox(this->hWnd, (LPCTSTR) message.c_str(), L"Alert", MB_OK | MB_ICONQUESTION);
}

// Displays a JavaScript confirm panel associated with the given view.
// Clients should visually indicate that this panel comes
// from JavaScript. The panel should have two buttons, e.g. "OK" and
// "Cancel". Returns true if the user hit OK, or false if the user hit Cancel.
bool TIWebViewDelegate::RunJavaScriptConfirm(WebView* webview, const std::wstring& message) {
	int result = MessageBox(this->hWnd, (LPCTSTR) message.c_str(), L"Confirm", MB_YESNO | MB_ICONEXCLAMATION);

	return (result == IDYES);
}

std::wstring jsPromptLabel;
std::wstring jsPromptDefaultText;
std::wstring jsPromptText;

void SaveJSPromptText(HWND hWndDlg) {
	int len = GetWindowTextLength(GetDlgItem(hWndDlg, JSPROMPTTEXT));

	if(len > 0) {
		wchar_t buffer[2049];
		GetDlgItemText(hWndDlg, JSPROMPTTEXT, (LPWSTR) buffer, len + 1);
		jsPromptText.assign(buffer);
	}
	else {
		jsPromptText.clear();
	}
}


LRESULT CALLBACK JsPromptDlgProc(HWND hWndDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch(Msg)
	{
	case WM_INITDIALOG:
		{
			SetDlgItemText(hWndDlg, JSPROMPTLABEL, (LPCWSTR)jsPromptLabel.c_str());
			SetDlgItemText(hWndDlg, JSPROMPTTEXT, (LPCWSTR)jsPromptDefaultText.c_str());

			return TRUE;
		}

	case WM_COMMAND:
		switch(wParam)
		{
		case JSPROMPTIDOK:
			SaveJSPromptText(hWndDlg);
			EndDialog(hWndDlg, JSPROMPTIDOK);
			return TRUE;
		case JSPROMPTIDCANCEL:
			EndDialog(hWndDlg, JSPROMPTIDCANCEL);
			return TRUE;
		case WM_DESTROY:
			EndDialog(hWndDlg, JSPROMPTIDCANCEL);
			return TRUE;
		default:
			printf("wParam = %d\n", wParam);
		}
		break;
	}

	return FALSE;
}


// Displays a JavaScript text input panel associated with the given view.
// Clients should visually indicate that this panel comes from JavaScript.
// The panel should have two buttons, e.g. "OK" and "Cancel", and an area to
// type text. The default_value should appear as the initial text in the
// panel when it is shown. If the user hit OK, returns true and fills result
// with the text in the box.  The value of result is undefined if the user
// hit Cancel.
bool TIWebViewDelegate::RunJavaScriptPrompt(WebView* webview,
                               const std::wstring& message,
                               const std::wstring& default_value,
                               std::wstring* result) {

	jsPromptLabel = message;
	jsPromptDefaultText = default_value;

	INT_PTR r = DialogBox(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_JSPROMPT),
		this->hWnd, reinterpret_cast<DLGPROC>(JsPromptDlgProc));

	if(r == JSPROMPTIDOK) {
		result->assign(jsPromptText);

		return true;
	}
	else {
		return false;
	}
}

// Displays a "before unload" confirm panel associated with the given view.
// The panel should have two buttons, e.g. "OK" and "Cancel", where OK means
// that the navigation should continue, and Cancel means that the navigation
// should be cancelled, leaving the user on the current page.  Returns true
// if the user hit OK, or false if the user hit Cancel.
bool TIWebViewDelegate::RunBeforeUnloadConfirm(WebView* webview,
                                  const std::wstring& message) {
	return true;  // OK, continue to navigate away
}