/*
 * web_wit_policy_delegate.cpp
 *
 *  Created on: Feb 16, 2009
 *      Author: jorge
 */

#include "webkit_policy_delegate.h"

#include <string>

namespace ti {
	Win32WebKitPolicyDelegate::Win32WebKitPolicyDelegate(Win32UserWindow *window_)
		: window(window_), m_refCount(1), m_permissiveDelegate(false)
	{
	}

	// IUnknown
	HRESULT STDMETHODCALLTYPE Win32WebKitPolicyDelegate::QueryInterface(REFIID riid, void** ppvObject)
	{
		*ppvObject = 0;
		if (IsEqualGUID(riid, IID_IUnknown))
			*ppvObject = static_cast<IWebPolicyDelegate*>(this);
		else if (IsEqualGUID(riid, IID_IWebPolicyDelegate))
			*ppvObject = static_cast<IWebPolicyDelegate*>(this);
		else
			return E_NOINTERFACE;

		AddRef();
		return S_OK;
	}

	ULONG STDMETHODCALLTYPE Win32WebKitPolicyDelegate::AddRef(void)
	{
		return ++m_refCount;
	}

	ULONG STDMETHODCALLTYPE Win32WebKitPolicyDelegate::Release(void)
	{
		ULONG newRef = --m_refCount;
		if (!newRef)
			delete this;

		return newRef;
	}

	HRESULT STDMETHODCALLTYPE Win32WebKitPolicyDelegate::decidePolicyForNavigationAction(
		/*[in]*/ IWebView* /*webView*/,
		/*[in]*/ IPropertyBag* actionInformation,
		/*[in]*/ IWebURLRequest* request,
		/*[in]*/ IWebFrame* frame,
		/*[in]*/ IWebPolicyDecisionListener* listener)
	{
		std::cout << "ppppppppppppp  decidePolicyForNavigationAction() called" << std::endl;

		/*
		BSTR url;
		request->URL(&url);

		int navType = 0;
		VARIANT var;
		if (SUCCEEDED(actionInformation->Read(WebActionNavigationTypeKey, &var, 0))) {
			V_VT(&var) = VT_I4;
			navType = V_I4(&var);
		}

		const char* typeDescription;
		switch (navType) {
			case WebNavigationTypeLinkClicked:
				typeDescription = "link clicked";
				break;
			case WebNavigationTypeFormSubmitted:
				typeDescription = "form submitted";
				break;
			case WebNavigationTypeBackForward:
				typeDescription = "back/forward";
				break;
			case WebNavigationTypeReload:
				typeDescription = "reload";
				break;
			case WebNavigationTypeFormResubmitted:
				typeDescription = "form resubmitted";
				break;
			case WebNavigationTypeOther:
				typeDescription = "other";
				break;
			default:
				typeDescription = "illegal value";
		}

		printf("Policy delegate: attempt to load %S with navigation type '%s'\n", url ? url : TEXT(""), typeDescription);

		SysFreeString(url);

		if (m_permissiveDelegate)
			listener->use();
		else
			listener->ignore();

		return S_OK;
		*/
		return E_NOTIMPL;
	}

    HRESULT STDMETHODCALLTYPE Win32WebKitPolicyDelegate::decidePolicyForNewWindowAction(
        /* [in] */ IWebView *webView,
        /* [in] */ IPropertyBag *actionInformation,
        /* [in] */ IWebURLRequest *request,
        /* [in] */ BSTR frameName,
        /* [in] */ IWebPolicyDecisionListener *listener)
    {
		std::wstring frame(frameName);
		transform(frame.begin(), frame.end(), frame.begin(), tolower);

		if(frame == L"ti::systembrowser")
		{
			BSTR u;
			request->URL(&u);
			std::wstring url(u);

			ShellExecuteW(NULL, L"open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
			listener->ignore();
		}
		else
		{
			listener->use();
		}

		return S_OK;
    }

	HRESULT STDMETHODCALLTYPE Win32WebKitPolicyDelegate::decidePolicyForMIMEType(
		/* [in] */ IWebView *webView,
		/* [in] */ BSTR type,
		/* [in] */ IWebURLRequest *request,
		/* [in] */ IWebFrame *frame,
		/* [in] */ IWebPolicyDecisionListener *listener)
	{
		std::cout << "ppppppppppppp  decidePolicyForMIMEType() called" << std::endl;
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE Win32WebKitPolicyDelegate::unableToImplementPolicyWithError(
		/* [in] */ IWebView *webView,
		/* [in] */ IWebError *error,
		/* [in] */ IWebFrame *frame)
	{
		std::cout << "ppppppppppppp  unableToImplementPolicyWithError() called" << std::endl;
		return E_NOTIMPL;
	}

}
