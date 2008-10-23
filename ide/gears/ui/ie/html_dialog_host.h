// Copyright 2006, Google Inc.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//  3. Neither the name of Google Inc. nor the names of its contributors may be
//     used to endorse or promote products derived from this software without
//     specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
// EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// This class is branched from googleclient/bar/common/htmldialog.h. Some
// changes have been made to make it work with Gears:
// - The powerful HtmlExternal interface was replaced with simpler
//   dialogArguments/dialogResult string properties since Gears interacts with
//   HTML dialogs purely through JSON strings.
// - Some macros were defined locally since they don't exist in Gears.
// - Pure virtual methods were removed since in Gears this class is not used as
//   a base class.
// - The title is now automatically set based on the title in the HTML for
//   compatibility with the Firefox implementation.
// - Support for specifying a parent window was removed, since specifying the
//   parent window in a cross-platform way would be challenging. Instead the
//   active window of the calling thread/context is always used.

#ifndef GEARS_UI_IE_HTML_DIALOG_HOST_H__
#define GEARS_UI_IE_HTML_DIALOG_HOST_H__

#include <exdispid.h>
#include <mshtmhst.h>  // for IHTMLOMWindowServices

#include "gears/base/common/common.h"
#include "gears/base/ie/atl_headers.h"
#include "gears/ui/ie/ui_resources.h"
#include "genfiles/interfaces.h"

// HtmlDialogHost is a COM dialog which allows clients to display HTML
// resources as modal dialogs. It provides the underlying implementation for
// HtmlDialog in IE.
//
// String arguments can be specified through the dialogArguments and
// dialogResult IDispatch properties, which are accessible to JavaScript inside
// the dialog through the window.external object.
class HtmlDialogHost
    : public CAxDialogImpl<HtmlDialogHost>,
      public CComObjectRootEx<CComMultiThreadModel>,
      public IDispatchImpl<HtmlDialogHostInterface>,
      public IDispEventImpl<0, HtmlDialogHost, &DIID_DWebBrowserEvents2,
                            &LIBID_SHDocVw, 0xFFFF, 0xFFFF>,
      public IInternetSecurityManager,
      public IHTMLOMWindowServices,
      public IServiceProviderImpl<HtmlDialogHost> {
 public:
  // Required for HtmlDialogHost
  enum { IDD = IDD_GENERIC_HTML };

  // constructor
  HtmlDialogHost()
      : browser_(NULL), 
        document_(NULL), 
        is_position_set_(false), 
        hook_(NULL) {}

  DECLARE_PROTECT_FINAL_CONSTRUCT();
  DECLARE_NOT_AGGREGATABLE(HtmlDialogHost)

  BEGIN_COM_MAP(HtmlDialogHost)
    COM_INTERFACE_ENTRY(HtmlDialogHostInterface)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IInternetSecurityManager)
    COM_INTERFACE_ENTRY(IHTMLOMWindowServices)
    COM_INTERFACE_ENTRY(IServiceProvider)
  END_COM_MAP()

  BEGIN_MSG_MAP(HtmlDialogHost)
    MESSAGE_HANDLER(WM_CLOSE, OnClose)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    MESSAGE_HANDLER(WM_TIMER, OnTimer)
    MESSAGE_HANDLER(WM_SIZE, OnSize)
  END_MSG_MAP()

  BEGIN_SERVICE_MAP(HtmlDialogHost)
    SERVICE_ENTRY(__uuidof(IInternetSecurityManager))
    SERVICE_ENTRY(SID_SHTMLOMWindowServices)
  END_SERVICE_MAP()

  BEGIN_SINK_MAP(HtmlDialogHost)
    SINK_ENTRY_EX(0, DIID_DWebBrowserEvents2, DISPID_DOCUMENTCOMPLETE,
                  OnDocumentComplete)
  END_SINK_MAP()

  // Used with SetWindowsHookEx to route keyboard messages to the browser
  // control, so that things like tabfocus and dhtml keyboard events work
  // correctly.
  LRESULT MessageProc(int code, WPARAM wparam, LPARAM lparam);

  // Handles the ready state of the dialog being changed.
  void OnReadyStateChanged();

  // Updates the pointer to the HTML document contained in the dialog.
  HRESULT UpdateDocPointers();

  // From IInternetSecurityManager. We override this to make sure that our
  // dialog is viewable, when the local zone in IE is in high security mode.
  STDMETHODIMP ProcessUrlAction(LPCWSTR url, DWORD action, BYTE *policy,
                                DWORD policy_size, BYTE *context,
                                DWORD context_size, DWORD flags,
                                DWORD reserved);

  // From IInternetSecurityManager. We override this to make sure that our
  // dialog can use our local files, when the local zone in IE is in high
  // security mode.
  STDMETHODIMP MapUrlToZone(LPCWSTR url, DWORD *zone, DWORD flags);


  // Runs default IInternetSecurityManager behavior.
  STDMETHODIMP GetSecurityId(LPCWSTR url, BYTE *security_id,
                             DWORD *security_id_size, DWORD_PTR reserved) {
    return INET_E_DEFAULT_ACTION;
  }

  // Runs default IInternetSecurityManager behavior.
  STDMETHODIMP GetSecuritySite(IInternetSecurityMgrSite **site) {
    return INET_E_DEFAULT_ACTION;
  }

  // Runs default IInternetSecurityManager behavior.
  STDMETHODIMP GetZoneMappings(DWORD zone, IEnumString **enum_string,
                               DWORD flags) {
    return INET_E_DEFAULT_ACTION;
  }

  // Runs default IInternetSecurityManager behavior.
  STDMETHODIMP QueryCustomPolicy(LPCWSTR url, REFGUID guid_key, BYTE **policy,
                                 DWORD *policy_size, BYTE *context,
                                 DWORD context_size, DWORD reserved) {
    return INET_E_DEFAULT_ACTION;
  }

  // Runs default IInternetSecurityManager behavior.
  STDMETHODIMP SetSecuritySite(IInternetSecurityMgrSite *site) {
    return INET_E_DEFAULT_ACTION;
  }

  // Runs default IInternetSecurityManager behavior.
  STDMETHODIMP SetZoneMapping(DWORD zone, LPCWSTR pattern, DWORD flags) {
    return INET_E_DEFAULT_ACTION;
  }

  // IHTMLOMWindowServices implementation to respond to JavaScript
  // window.resizeBy(x,y) calls, we ignore the other methods.
  STDMETHODIMP moveTo(LONG x, LONG y) { return S_OK; }
  STDMETHODIMP moveBy(LONG x, LONG y) { return S_OK; }
  STDMETHODIMP resizeTo(LONG x, LONG y) { return S_OK; }
  STDMETHODIMP resizeBy(LONG x, LONG y);

  // Shows the dialog, encapsulating required calls in addition to DoModal.
  bool ShowDialog(const char16 *resource_file_name, const CSize& size,
                  const BSTR dialog_argument, BSTR *dialog_result);

  // Called by script to get dialog arguments.
  STDMETHODIMP GetDialogArguments(BSTR *args_string);

  // Called by script inside the dialog to close and send back result.
  STDMETHODIMP CloseDialog(const BSTR result_string);

  // Called by script to check if we are in Pocket IE or Desktop IE
  STDMETHODIMP IsPocketIE(VARIANT_BOOL *retval);

 protected:
  // Handles the DocumentComplete web browser control event.
  virtual void _stdcall OnDocumentComplete(IDispatch* disp, VARIANT* url);

 private:
  // Initializes the browser member variable.
  HRESULT InitBrowser();

  // Loads a URL in the browser.
  HRESULT NavigateToUrl(const char16 *url);

  // Handles window initialization.
  virtual LRESULT OnInitDialog(UINT message, WPARAM w, LPARAM l, BOOL& handled);

  // Handles the user hitting the 'x' button.
  LRESULT OnClose(UINT message, WPARAM w, LPARAM l, BOOL& handled);

  // Handles a timer firing.
  LRESULT OnTimer(UINT message, WPARAM w, LPARAM l, BOOL& handled);

  // Handles user resizing of the dialog window frame
  LRESULT OnSize(UINT message, WPARAM w, LPARAM l, BOOL& handled);

  // Sets the dialog position.
  void SetDialogPosition();

  // Runs post-constructor initialization of the options dialog.
  bool SetUp(const char16 *resource_file_name, const CSize& size);

  // Releases resources held by the dialog.
  void TearDown();

  // Initializes base_url_ and data_url_.
  bool InitBaseUrl();

  // Sets the caption based on the <title> of the HTML document being displayed.
  void UpdateCaption();

  // The hosted web control.
  CComPtr<IWebBrowser2> browser_;

  // The window hosting the control.
  CAxWindow2 browser_window_;

  // The dialog caption.
  CString caption_;

  // The desired window size.
  CSize desired_size_;

  // The DHTML document loaded in the web control.
  CComPtr<IHTMLDocument2> document_;

  // Whether the dialog position has been set.
  bool is_position_set_;

  // The res:// URL representing the folder where our HTML, images, etc are.
  CString base_url_;

  // The file:// URL representing the folder where our data is.
  CString data_url_;

  // The current URL.
  CString url_;

  // Arguments to make available to the script inside the dialog.
  CComBSTR dialog_arguments_;

  // Result sent back from dialog to caller.
  CComBSTR dialog_result_;

  // Windows hook used for trapping keyboard events and routing them to the
  // browser control.
  HHOOK hook_;

  DISALLOW_EVIL_CONSTRUCTORS(HtmlDialogHost);
};


#endif  // GEARS_UI_IE_HTML_DIALOG_HOST_H__
