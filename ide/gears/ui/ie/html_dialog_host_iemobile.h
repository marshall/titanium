// Copyright 2007, Google Inc.
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
//   dialogArguments/dialogResult string properties since Scour interacts with
//   HTML dialogs purely through JSON strings.
// - Some macros were defined locally since they don't exist in Gears.
// - Pure virtual methods were removed since in Gears this class is not used as
//   a base class.
// - The title is now automatically set based on the title in the HTML for
//   compatibility with the Firefox implementation.
// - Support for specifying a parent window was removed, since specifying the
//   parent window in a cross-platform way would be challenging. Instead the
//   active window of the calling thread/context is always used.

#ifndef GEARS_UI_IE_HTML_DIALOG_HOST_IEMOBILE_H__
#define GEARS_UI_IE_HTML_DIALOG_HOST_IEMOBILE_H__

#include <webvw.h>
#include <htmlctrl.h>

#include "gears/base/common/base_class.h"
#include "gears/base/ie/resource.h"  // for .rgs resource ids (IDR_*)

#include "genfiles/interfaces.h"
#include "gears/base/common/common.h"
#include "gears/base/ie/atl_headers.h"
#include "gears/ui/ie/ui_resources.h"
#include "gears/base/ie/activex_utils.h"

// HtmlDialogHost is a COM dialog which allows clients to display HTML
// resources as modal dialogs. It provides the underlying implementation for
// HtmlDialog in PocketIE.
//
// String arguments can be specified through the dialogArguments and
// dialogResult IDispatch properties, which are accessible to JavaScript
// inside the dialog through the BrowserProperties instance (pie.dialog)
//
// Contrary to the Desktop IE implementation, we can not use the
// window.external trick to be accessed from the javascript side.
// Our workaround is to declare an activex object in the html
// (PIEDialogBridge) and let that object access a global variable
// pointing to our dialog instance (see html_dialog_pie.cc).
//
// We do not want to simply provide a direct access to the dialog through
// the global variable; that would allow a possible security issue if another
// activex bridge is created in parallel to the dialog window.
// Instead, we 'validate' the access to this global variable by traversing
// the DOM of the document hosted by this dialog and setting the instance
// only for PIEDialogBridge objects -- therefore the only PIEDialogBridge
// instances that will have access to the dialog are the instances created
// in the html we load in this dialog. Additionnally, only one dialog can be
// on the screen (PocketIE only has a single window, and we use a modal dialog).

class HtmlDialogHost : public CDialogImpl<HtmlDialogHost>,
                          public IDispatchImpl<PIEDialogHostInterface>,
                          public CComObjectRootEx<CComMultiThreadModel> {
 public:

  HtmlDialogHost() : browser_view_(0) {}

  enum { IDD = IDD_GENERIC_HTML };

  DECLARE_PROTECT_FINAL_CONSTRUCT();
  DECLARE_NOT_AGGREGATABLE(HtmlDialogHost)

  BEGIN_COM_MAP(HtmlDialogHost)
    COM_INTERFACE_ENTRY(HtmlDialogHostInterface)
    COM_INTERFACE_ENTRY(IDispatch)
  END_COM_MAP()

  BEGIN_MSG_MAP(HtmlDialogHost)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    MESSAGE_HANDLER(WM_NOTIFY, OnNotify)
    MESSAGE_HANDLER(WM_CLOSE, OnClose)
    MESSAGE_HANDLER(WM_SIZE, OnResize)
    MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingChange)
    MESSAGE_HANDLER(WM_COMMAND, OnCommand)
    MESSAGE_HANDLER(WM_HOTKEY, OnHotKey)
  END_MSG_MAP()

  bool ShowDialog(const char16 *resource_file_name,
                  const CSize& size, const BSTR dialog_arguments,
                  BSTR *dialog_result);

 private:

  // Event handlers.
  LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnResize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnSettingChange(UINT uMsg, WPARAM wParam, LPARAM lParam,
                          BOOL& bHandled);
  LRESULT OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnHotKey(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnClose(UINT message, WPARAM w, LPARAM l, BOOL& handled);

  // Called by script to get dialog arguments.
  STDMETHODIMP GetDialogArguments(BSTR *args_string);

  // Called by script inside the dialog to close and send back result.
  STDMETHODIMP CloseDialog(const BSTR result_string);

  // Called by script to check if we are in Pocket IE or Desktop IE.
  STDMETHODIMP IsPocketIE(VARIANT_BOOL *retval);

  // Called by script to check if we are on a Smartphone.
  STDMETHODIMP IsSmartPhone(VARIANT_BOOL *retval);

  // Called by script to resize the dialog on Pocket IE.
  STDMETHODIMP ResizeDialog();

  // Used by javascript to provide an execution context for
  // the softkey "callback" scripts.
  STDMETHODIMP SetScriptContext(IDispatch *val);

  // Used by javascript to set an action on the Right softkey button.
  STDMETHODIMP SetButton(const BSTR label, const BSTR script);

  // Used by javascript to enable/disable the Right softkey button.
  STDMETHODIMP SetButtonEnabled(VARIANT_BOOL *val);

  // Used by javascript to set the cancel button label.
  STDMETHODIMP SetCancelButton(const BSTR label);

  // Utility function returning true if we are on a Smartphone.
  bool CheckIsSmartPhone();

  // Utility function executing a script in the provided context.
  HRESULT ExecScript(const BSTR script);

  // Initialize the Win32 HTML control and set the document variable.
  void InitBrowserView();

  // Returns a pointer to a resource contained in the dll.
  HRESULT LoadFromResource(CString rsc, void** resource, int* len);

  // Loads the css file from the dll and informs the html control.
  HRESULT LoadCSS(CString rsc);

  // Loads an image, decompress it and send it to the Web control.
  HRESULT LoadImage(CString rsc, DWORD cookie);

  // Computes and set the window size depending on the available screen area
  HRESULT ResizeWindow();

  // The window handler to the Win32 HTML Control.
  HWND browser_view_;

  // Our html document (used by the PIEDialogBridge).
  CComQIPtr<IPIEHTMLDocument2> document_;

  // The html window.
  CComQIPtr<IPIEHTMLWindow2> html_window_;

  CComBSTR allow_action_;

  // Execute the script provided by SetButton().
  HRESULT RightButtonAction();

  // Track if the right softkey button is enabled.
  bool right_button_enabled_;

  // The res:// URL representing the folder where our HTML, images, etc are.
  CString base_url_;

  // The current URL.
  CString url_;

  // Arguments to make available to the script inside the dialog.
  CComBSTR dialog_arguments_;

  // Result sent back from dialog to caller.
  CComBSTR dialog_result_;

  // The desired window size.
  CSize desired_size_;

  // Boolean storing the smartphone test.
  bool is_smartphone_;

  // Guard variable to prevent resizing.
  bool resizing_;

  // The global variable allowing the activex bridge PIEDialogBridge to
  // access the dialog.
  static HtmlDialogHost* html_permissions_dialog_;

  // We declare PIEDialogBridge as friend so that it can access
  // the html_permissions_dialog_ global variable.
  friend class PIEDialogBridge;

  DISALLOW_EVIL_CONSTRUCTORS(HtmlDialogHost);
};

#endif  // GEARS_UI_IE_HTML_DIALOG_HOST_IEMOBILE_H__
