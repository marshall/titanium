/*
 * Win32UIDelegate.cpp
 *
 *  Created on: Jan 22, 2009
 *      Author: jorge
 */

#include "ui_delegate.h"

#include <comdef.h>
#include "win32_user_window.h"

using namespace ti;

Win32UIDelegate::Win32UIDelegate(Win32UserWindow *window_) : window(window_), ref_count(1) {

}

HRESULT STDMETHODCALLTYPE
Win32UIDelegate::QueryInterface(REFIID riid, void **ppvObject)
{
	*ppvObject = 0;

    if (IsEqualGUID(riid, IID_IUnknown))
        *ppvObject = static_cast<IWebUIDelegate*>(this);
    else if (IsEqualGUID(riid, IID_IWebUIDelegate))
        *ppvObject = static_cast<IWebUIDelegate*>(this);
        /*
    else if (IsEqualGUID(riid, IID_IWebUIDelegatePrivate))
        *ppvObject = static_cast<IWebUIDelegatePrivate*>(this);
    else if (IsEqualGUID(riid, IID_IWebUIDelegatePrivate2))
        *ppvObject = static_cast<IWebUIDelegatePrivate2*>(this);
    else if (IsEqualGUID(riid, IID_IWebUIDelegatePrivate3))
        *ppvObject = static_cast<IWebUIDelegatePrivate3*>(this);
		*/
    else
        return E_NOINTERFACE;

	return S_OK;
}

ULONG STDMETHODCALLTYPE
Win32UIDelegate::AddRef()
{
	return ++ref_count;
}

ULONG STDMETHODCALLTYPE
Win32UIDelegate::Release()
{
	ULONG new_count = --ref_count;
	if (!new_count) delete(this);

	return new_count;
}

HRESULT STDMETHODCALLTYPE
Win32UIDelegate::createWebViewWithRequest(
	/* [in] */ IWebView *sender,
	/* [in] */ IWebURLRequest *request,
	/* [retval][out] */ IWebView **newWebView)
{
	std::cout << "&&&&&&&&&&&&&&  createWebViewWithRequest() called" << std::endl;
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE
Win32UIDelegate::webViewClose(
	/* [in] */ IWebView *sender)
{
	std::cout << "&&&&&&&&&&&&&&  webViewClose() called" << std::endl;
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE
Win32UIDelegate::webViewFocus(
	/* [in] */ IWebView *sender)
{
	std::cout << "&&&&&&&&&&&&&&  webViewFocus() called" << std::endl;
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE
Win32UIDelegate::webViewUnfocus(
	/* [in] */ IWebView *sender)
{
	std::cout << "&&&&&&&&&&&&&&  webViewUnfocus() called" << std::endl;
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE
Win32UIDelegate::setStatusText(
	/* [in] */ IWebView *sender,
	/* [in] */ BSTR text)
{
	std::string s;

	if(text)
	{
		s.append(_bstr_t(text));
	}
	std::cout << "&&&&&&&&&&&&&&  setStatusText() called '" << s << "'" << std::endl;
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE
Win32UIDelegate::setFrame(
	/* [in] */ IWebView *sender,
	/* [in] */ RECT *frame)
{
	std::cout << "&&&&&&&&&&&&&&  setFrame() called" << std::endl;
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE
Win32UIDelegate::webViewFrame(
	/* [in] */ IWebView *sender,
	/* [retval][out] */ RECT *frame)
{
	std::cout << "&&&&&&&&&&&&&&  webViewFrame() called" << std::endl;
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE
Win32UIDelegate::runJavaScriptAlertPanelWithMessage(
	/* [in] */ IWebView *sender,
	/* [in] */ BSTR message)
{
	HWND handle = window->GetWindowHandle();
	std::string msg;
	std::string title("JavaScript Alert");

	if(message)
	{
		msg.append(_bstr_t(message));
	}

	MessageBox(handle, (LPCTSTR) msg.c_str(), (LPCTSTR) title.c_str(), MB_OK | MB_ICONQUESTION);

	return S_OK;
}

HRESULT STDMETHODCALLTYPE
Win32UIDelegate::runJavaScriptConfirmPanelWithMessage(
	/* [in] */ IWebView *sender,
	/* [in] */ BSTR message,
	/* [retval][out] */ BOOL *result)
{
	HWND handle = window->GetWindowHandle();
	std::string title("JavaScript Confirm");
	std::string msg;

	if(message)
	{
		msg.append(bstr_t(message));
	}

	int r = MessageBox(handle, (LPCTSTR) msg.c_str(), (LPCTSTR) title.c_str(), MB_YESNO | MB_ICONEXCLAMATION);
	*result = (r == IDYES);

	return S_OK;
}

HRESULT STDMETHODCALLTYPE
Win32UIDelegate::runJavaScriptTextInputPanelWithPrompt(
	/* [in] */ IWebView *sender,
	/* [in] */ BSTR message,
	/* [in] */ BSTR defaultText,
	/* [retval][out] */ BSTR *result)
{
	HWND handle = window->GetWindowHandle();
	std::string title("JavaScript Prompt");
	std::string msg = _bstr_t(message);
	std::string def;

	if(defaultText)
	{
		def.append(_bstr_t(defaultText));
	}

	std::cout << "runJavaScriptTextInputPanelWithPrompt() called '" << msg << "' , '" << def << "'" << std::endl;

	Win32PopupDialog popupDialog(handle);
	popupDialog.SetTitle(title);
	popupDialog.SetMessage(msg);
	popupDialog.SetShowInputText(true);
	popupDialog.SetInputText(def);
	int r = popupDialog.Show();

	if(r == IDYES)
	{
		_bstr_t bstr1(popupDialog.GetInputText().c_str());
		*result = bstr1.copy();
	}

	return S_OK;
}

HRESULT STDMETHODCALLTYPE
Win32UIDelegate::runBeforeUnloadConfirmPanelWithMessage(
	/* [in] */ IWebView *sender,
	/* [in] */ BSTR message,
	/* [in] */ IWebFrame *initiatedByFrame,
	/* [retval][out] */ BOOL *result)
{
	std::cout << "&&&&&&&&&&&&&&  runBeforeUnloadConfirmPanelWithMessage() called" << std::endl;
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE
Win32UIDelegate::hasCustomMenuImplementation(
	/* [retval][out] */ BOOL *hasCustomMenus)
{
	std::cout << "&&&&&&&&&&&&&&  hasCustomMenuImplementation() called" << std::endl;
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE
Win32UIDelegate::trackCustomPopupMenu(
	/* [in] */ IWebView *sender,
	/* [in] */ OLE_HANDLE menu,
	/* [in] */ LPPOINT point)
{
	std::cout << "&&&&&&&&&&&&&&  trackCustomPopupMenu() called" << std::endl;
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE
Win32UIDelegate::registerUndoWithTarget(
	/* [in] */ IWebUndoTarget *target,
	/* [in] */ BSTR actionName,
	/* [in] */ IUnknown *actionArg)
{
	std::cout << "&&&&&&&&&&&&&&  registerUndoWithTarget() called" << std::endl;
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE
Win32UIDelegate::removeAllActionsWithTarget(
	/* [in] */ IWebUndoTarget *target)
{
	std::cout << "&&&&&&&&&&&&&&  removeAllActionsWithTarget() called " << (int)target << std::endl;
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE
Win32UIDelegate::setActionTitle(
	/* [in] */ BSTR actionTitle)
{
	std::cout << "&&&&&&&&&&&&&&  setActionTitle() called" << std::endl;
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE
Win32UIDelegate::undo()
{
	std::cout << "&&&&&&&&&&&&&&  undo() called" << std::endl;
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE
Win32UIDelegate::redo()
{
	std::cout << "&&&&&&&&&&&&&&  redo() called" << std::endl;
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE
Win32UIDelegate::canUndo(
	/* [retval][out] */ BOOL *result)
{
	std::cout << "&&&&&&&&&&&&&&  canUndo() called" << std::endl;
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE
Win32UIDelegate::canRedo(
	/* [retval][out] */ BOOL *result)
{
	std::cout << "&&&&&&&&&&&&&&  canRedo() called" << std::endl;
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE
Win32UIDelegate::webViewAddMessageToConsole(
	/* [in] */ IWebView *sender,
	/* [in] */ BSTR message,
	/* [in] */ int lineNumber,
	/* [in] */ BSTR url,
	/* [in] */ BOOL isError)
{
	std::cout << "&&&&&&&&&&&&&&  webViewAddMesageToConsole() called" << std::endl;
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE
Win32UIDelegate::doDragDrop(
	/* [in] */ IWebView *sender,
	/* [in] */ IDataObject *dataObject,
	/* [in] */ IDropSource *dropSource,
	/* [in] */ DWORD okEffect,
	/* [retval][out] */ DWORD *performedEffect)
{
	std::cout << "&&&&&&&&&&&&&&  doDragDrop() called" << std::endl;
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE
Win32UIDelegate::webViewGetDlgCode(
	/* [in] */ IWebView *sender,
	/* [in] */ UINT keyCode,
	/* [retval][out] */ LONG_PTR *code)
{
	std::cout << "&&&&&&&&&&&&&&  webViewGetDlgCode() called" << std::endl;
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE
Win32UIDelegate::webViewPainted(
	/* [in] */ IWebView *sender)
{
	std::cout << "&&&&&&&&&&&&&&  webViewPainted() called" << std::endl;
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE
Win32UIDelegate::exceededDatabaseQuota(
	/* [in] */ IWebView *sender,
	/* [in] */ IWebFrame *frame,
	/* [in] */ IWebSecurityOrigin *origin,
	/* [in] */ BSTR databaseIdentifier)
{
	std::cout << "&&&&&&&&&&&&&&  exceededDatabaseQuota() called" << std::endl;

    static const unsigned long long defaultQuota = 100 * 1024 * 1024;	// 100MB
    origin->setQuota(defaultQuota);

	return E_NOTIMPL;
}
