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
#include "ti_chrome_window.h"
#include "ti_utils.h"
#include "ti_app_arguments.h"
//#include "third_party/WebKit/WebCore/platform/graphics/IntSize.h"
//#include "third_party/WebKit/WebCore/platform/DragImage.h"
//#include "Frame.h"
//#include "FrameLoader.h"
#include "Resource.h"
#include <shellapi.h>
	
std::vector<WebFrame*> TiWebViewDelegate::initializedFrames = std::vector<WebFrame*>();

TiWebViewDelegate::~TiWebViewDelegate() {
}

WebWidgetHost* TiWebViewDelegate::GetHostForWidget(WebWidget* webwidget) {
  if (webwidget == window->getHost()->webwidget())
    return window->getHost();
  if (webwidget == window->getPopupHost()->webwidget())
    return window->getPopupHost();
  return NULL;
}

gfx::ViewHandle TiWebViewDelegate::GetContainingWindow(WebWidget* webwidget) {
	if (WebWidgetHost* host = GetHostForWidget(webwidget))
		return host->window_handle();

	return NULL;
}

// Called when a region of the WebWidget needs to be re-painted.
void TiWebViewDelegate::DidInvalidateRect(WebWidget* webwidget, const gfx::Rect& rect) {
	if (WebWidgetHost* host = GetHostForWidget(webwidget))
		host->DidInvalidateRect(rect);
}

// Called when a region of the WebWidget, given by clip_rect, should be
// scrolled by the specified dx and dy amounts.
void TiWebViewDelegate::DidScrollRect(WebWidget* webwidget, int dx, int dy,
	const gfx::Rect& clip_rect) {
	if (WebWidgetHost* host = GetHostForWidget(webwidget))
		return host->DidScrollRect(dx, dy, clip_rect);
}

// This method is called to instruct the window containing the WebWidget to
// show itself as the topmost window.  This method is only used after a
// successful call to CreateWebWidget.  |disposition| indicates how this new
// window should be displayed, but generally only means something for WebViews.
void TiWebViewDelegate::Show(WebWidget* webwidget, WindowOpenDisposition disposition) {
	if (webwidget == window->getHost()->webwidget()) {
		//ShowWindow(window->getWindowHandle(), SW_SHOW);
		// don't show here, wait until we're finished loading?

		UpdateWindow(window->getWindowHandle());
		
	} else if (webwidget == window->getPopupHost()->webwidget()) {
		ShowWindow(window->getPopupWindowHandle(), SW_SHOW);
		UpdateWindow(window->getPopupWindowHandle());
	}
}

// This method is called to instruct the window containing the WebWidget to
// close.  Note: This method should just be the trigger that causes the
// WebWidget to eventually close.  It should not actually be destroyed until
// after this call returns.
void TiWebViewDelegate::CloseWidgetSoon(WebWidget* webwidget) {
	if (webwidget == window->getHost()->webwidget()) {
		PostMessage(window->getWindowHandle(), WM_CLOSE, 0, 0);
	} else if (webwidget == window->getPopupHost()->webwidget()) {
		window->closePopup();
	}
	
}

// This method is called to focus the window containing the WebWidget so
// that it receives keyboard events.
void TiWebViewDelegate::Focus(WebWidget* webwidget) {
	if (WebWidgetHost *host = GetHostForWidget(webwidget))
		host->webwidget()->SetFocus(true);
}

// This method is called to unfocus the window containing the WebWidget so that
// it no longer receives keyboard events.
void TiWebViewDelegate::Blur(WebWidget* webwidget) {
	if (WebWidgetHost *host = GetHostForWidget(webwidget))
		host->webwidget()->SetFocus(false);
}

void TiWebViewDelegate::SetCursor(WebWidget* webwidget, 
	const WebCursor& cursor) {
	if (WebWidgetHost* host = GetHostForWidget(webwidget)) {
		if (customCursor) {
			DestroyIcon(customCursor);
			customCursor = NULL;
		}
		if (cursor.IsCustom()) {
			customCursor = cursor.GetCustomCursor();
			host->SetCursor(customCursor);
		} else {
			HINSTANCE mod_handle = GetModuleHandle(NULL);
			host->SetCursor(cursor.GetCursor(mod_handle));
		}
	}
}

// Returns the rectangle of the WebWidget in screen coordinates.
void TiWebViewDelegate::GetWindowRect(WebWidget* webwidget, gfx::Rect* out_rect) {
	if (WebWidgetHost* host = GetHostForWidget(webwidget)) {
		RECT rect;
		::GetWindowRect(host->window_handle(), &rect);
		*out_rect = gfx::Rect(rect);
	}
}


// This method is called to re-position the WebWidget on the screen.  The given
// rect is in screen coordinates.  The implementation may choose to ignore
// this call or modify the given rect.  This method may be called before Show
// has been called.
// TODO(darin): this is more of a request; does this need to take effect
// synchronously?
void TiWebViewDelegate::SetWindowRect(WebWidget* webwidget, const gfx::Rect& rect) {
	if (webwidget == window->getHost()->webwidget()) {
		// Update our window model, then actually do the move
		window->getTiWindowConfig()->setX(rect.x());
		window->getTiWindowConfig()->setY(rect.y());
		window->getTiWindowConfig()->setWidth(rect.width());
		window->getTiWindowConfig()->setHeight(rect.height());

		MoveWindow(window->getWindowHandle(), rect.x(), rect.y(), rect.width(), rect.height(), FALSE);
	} else if (webwidget == window->getPopupHost()->webwidget()) {
		MoveWindow(window->getPopupWindowHandle(),
			rect.x(), rect.y(), rect.width(), rect.height(), FALSE);
	}
}

// Returns the rectangle of the window in which this WebWidget is embeded in.
void TiWebViewDelegate::GetRootWindowRect(WebWidget* webwidget, gfx::Rect* out_rect) {
	if (WebWidgetHost* host = GetHostForWidget(webwidget)) {
		RECT rect;
		HWND root_window = ::GetAncestor(host->window_handle(), GA_ROOT);
		::GetWindowRect(root_window, &rect);
		*out_rect = gfx::Rect(rect);
	}
}

// Keeps track of the necessary window move for a plugin window that resulted
// from a scroll operation.  That way, all plugin windows can be moved at the
// same time as each other and the page.
void TiWebViewDelegate::DidMove(WebWidget* webwidget, const WebPluginGeometry& move) {
	WebPluginDelegateImpl::MoveWindow(
		move.window, move.window_rect, move.clip_rect, move.cutout_rects,
		move.visible);
}

// Suppress input events to other windows, and do not return until the widget
// is closed.  This is used to support |window.showModalDialog|.
void TiWebViewDelegate::RunModal(WebWidget* webwidget){

}

const std::string kCustomTargetParam = "ti_Target_Win32";

bool TiWebViewDelegate::ExecuteCustomTarget(std::string &customTarget, std::string newURL)
{
	if (LowerCaseEqualsASCII(customTarget, "systembrowser"))
	{
		ShellExecute(NULL, L"open", UTF8ToWide(newURL).c_str(), NULL, NULL, SW_SHOWNORMAL);

		return true;
	}
	return false;
}

WindowOpenDisposition TiWebViewDelegate::DispositionForNavigationAction(WebView *webView,
											WebFrame *frame,
											const WebRequest *request,
											WebNavigationType type,
											WindowOpenDisposition disposition,
											bool is_redirect)
{
	GURL url = request->GetURL();
	if (url.has_query()) {
		std::string queryString = url.query();
		size_t pos;
		if ((pos = queryString.find(kCustomTargetParam+"=")) != std::string::npos) {
			
			size_t nextAmperstand = queryString.find("&", pos);
			std::string newQueryString = queryString;
			std::string customTarget = "";

			customTarget = queryString.substr(pos+kCustomTargetParam.length()+1, nextAmperstand);
			newQueryString = newQueryString.replace(pos, pos+kCustomTargetParam.length()+1, "");

			if (customTarget.length() > 0) {
				newQueryString = newQueryString.replace(0, newQueryString.find("&"), "");
				std::string newURL = url.host() + url.path();
				if (newQueryString.length() > 0) {
					newURL += "?";
					newURL += newQueryString;
				}

				if (ExecuteCustomTarget(customTarget, newURL)) {
					return IGNORE_ACTION;
				}
			}
		}
	}

	return WebViewDelegate::DispositionForNavigationAction(webView, frame, request, type, disposition, is_redirect);
}

WebView* TiWebViewDelegate::CreateWebView(WebView* webview, bool user_gesture)
{
	TiChromeWindow *window = new TiChromeWindow(TiChromeWindow::getMainWindow()->getInstanceHandle(), "");

	return window->getHost()->webview();
}

void TiWebViewDelegate::DidCommitLoadForFrame(WebView* webview, WebFrame* frame, bool is_new_navigation) {
	std::vector<WebFrame*>::iterator iter;

	iter = std::find(initializedFrames.begin(), initializedFrames.end(), frame);
	if (iter != initializedFrames.end()) {
		initializedFrames.erase(iter);
	}

	TiWindowConfig *matchedWindow = NULL;
	TiWindowConfigList::iterator cIter = TiAppConfig::instance()->getWindows().begin();
	for (; cIter != TiAppConfig::instance()->getWindows().end() ; cIter++)
	{
		TiWindowConfig *window = (*cIter);
		if (TiURL::urlMatchesPattern(static_cast<GURL>(webview->GetMainFrame()->GetURL()), window->getURL())) {
			matchedWindow = window;
			break;
		}
	}

	window->showWindow(SW_HIDE);

	if (matchedWindow != NULL) {
		window->setTiWindowConfig(matchedWindow);
	}
}

void TiWebViewDelegate::DidFinishLoadForFrame(WebView* webview, WebFrame* frame) {
	initRuntime(frame);

	if (frame == webview->GetMainFrame() && window->isOpenOnLoad()) {
		window->open();
	}
}

void TiWebViewDelegate::DidStopLoading(WebView* webview)
{
	
}

// Owners depend on the delegates living as long as they do, so we ref them.
void TiWebViewDelegate::AddRef() {
	//base::RefCounted<TiWebViewDelegate>::AddRef();
}

void TiWebViewDelegate::Release() {
	//base::RefCounted<TiWebViewDelegate>::Release();
}

// Returns true if the widget is in a background tab.
bool TiWebViewDelegate::IsHidden() {
	return false;
}

void TiWebViewDelegate::initRuntime(WebFrame *frame)
{
	if (std::find(initializedFrames.begin(), initializedFrames.end(), frame) == initializedFrames.end())
	{
		TiRuntime *tiRuntime = new TiRuntime(window, frame);

		// do this first, BindToJavascript causes a 2nd callback to windowObjectCleared... strange.
		initializedFrames.push_back(frame);
		
		tiRuntime->BindToJavascript(frame, L"tiRuntime");
		std::string titanium_js = "ti://titanium.js";
		if (TiAppArguments::isDebugMode)
		{
			titanium_js = "ti://titanium-debug.js";
		}

		window->include(frame, titanium_js);
		
		if (TiAppArguments::openInspector && !TiChromeWindow::isInspectorOpen) {
			// disable the inspector for now, we need to figure out the best
			// way to distribute it.. probably as part of the DLL
			//window->openInspectorWindow();
		}
	}
}

// Basically the "firstDataReady" event initially calls us, but when we go to BindToJavascript,
// v8 recalls windowObject cleared, which is really weird.
void TiWebViewDelegate::WindowObjectCleared(WebFrame *webFrame)
{
	initRuntime(webFrame);
}

WebPluginDelegate* TiWebViewDelegate::CreatePluginDelegate(
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

void TiWebViewDelegate::AddMessageToConsole(WebView* webview,
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

// Displays a JavaScript alert panel associated with the given view. Clients
// should visually indicate that this panel comes from JavaScript. The panel
// should have a single OK button.
void TiWebViewDelegate::RunJavaScriptAlert(WebView* webview, const std::wstring& message) {
	MessageBox(window->getWindowHandle(), (LPCTSTR) message.c_str(), L"Alert", MB_OK | MB_ICONQUESTION);
}

// Displays a JavaScript confirm panel associated with the given view.
// Clients should visually indicate that this panel comes
// from JavaScript. The panel should have two buttons, e.g. "OK" and
// "Cancel". Returns true if the user hit OK, or false if the user hit Cancel.
bool TiWebViewDelegate::RunJavaScriptConfirm(WebView* webview, const std::wstring& message) {
	int result = MessageBox(window->getWindowHandle(), (LPCTSTR) message.c_str(), L"Confirm", MB_YESNO | MB_ICONEXCLAMATION);

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
bool TiWebViewDelegate::RunJavaScriptPrompt(WebView* webview,
                               const std::wstring& message,
                               const std::wstring& default_value,
                               std::wstring* result) {

	jsPromptLabel = message;
	jsPromptDefaultText = default_value;

	// TODO - center dialog box
	INT_PTR r = DialogBox(window->getInstanceHandle(), MAKEINTRESOURCE(IDD_JSPROMPT),
		window->getWindowHandle(), reinterpret_cast<DLGPROC>(JsPromptDlgProc));

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
bool TiWebViewDelegate::RunBeforeUnloadConfirm(WebView* webview,
                                  const std::wstring& message) {
	return true;  // OK, continue to navigate away
}

WebWidget* TiWebViewDelegate::CreatePopupWidget(WebView* webview, bool focus_on_show) {
  
	return window->createPopupWidget();
}

void TiWebViewDelegate::StartDragging(WebView *webView, const WebDropData &drop_data)
{
	webView->DragSourceSystemDragEnded();
}
