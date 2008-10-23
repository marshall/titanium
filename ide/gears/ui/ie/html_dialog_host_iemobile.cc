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

#include "gears/ui/ie/html_dialog_host_iemobile.h"
#include <initguid.h>
#include <imaging.h>
#include <tpcshell.h>
#include <winuser.h>

#include "gears/base/common/string_utils.h"

// Some macros defined here to keep changes from original source to a low roar.
// See //depot/googleclient/bar/common/(debugbase|macros).h for original
// definitions.
#ifdef WINCE
#else
#define ASSERT(expr) assert(expr);
#endif

// Checks for HRESULT and if it fails returns. The macro will ASSERT in debug.
#define CHK(cmd) \
do { \
  HRESULT __hr = (cmd); \
  if (FAILED(__hr)) { \
    ASSERT(false); \
    return __hr; \
  } \
} \
while(0);

// Note: we use a VERIFY macro, whichs verify that an operation succeed and
// ASSERTs on failure. Only asserts in DEBUG.
// This VERIFY macro is already defined by dbgapi.h on Windows mobile.

#define HTML MAKEINTRESOURCE(23)
const char16* kSmartPhone = STRING16(L"SmartPhone");

// The static variable we use to access the dialog from the ActiveX object.
HtmlDialogHost* HtmlDialogHost::html_permissions_dialog_;

bool HtmlDialogHost::ShowDialog(const char16 *resource_file_name,
                                const CSize& size,
                                const BSTR dialog_arguments,
                                BSTR *dialog_result) {
  dialog_arguments_ = dialog_arguments;
  desired_size_ = size;
  url_ = resource_file_name;

  DoModal();

  if (dialog_result_ == NULL) {
    (*dialog_result) = NULL;
  } else {
    dialog_result_.CopyTo(dialog_result);
  }

  if (IsWindow()) DestroyWindow();
  return true;
}

LRESULT HtmlDialogHost::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam,
                                     BOOL& bHandled) {
  // We create a softkey bar.

  BOOL rb;
  SHINITDLGINFO sid;
  sid.dwMask = SHIDIM_FLAGS;
  sid.dwFlags = SHIDIF_SIZEDLGFULLSCREEN;
  sid.hDlg = m_hWnd;
  rb = SHInitDialog(&sid);

  SHMENUBARINFO mbi;
  memset(&mbi, 0, sizeof(SHMENUBARINFO));
  mbi.cbSize = sizeof(SHMENUBARINFO);
  mbi.hwndParent = m_hWnd;
  mbi.nToolBarId = IDR_HTML_DIALOG_MENUBAR;
  mbi.hInstRes = _AtlBaseModule.GetModuleInstance();

  rb = SHCreateMenuBar(&mbi);

  // We want to be notified when the VK_TBACK button is pressed.

  HWND bar = SHFindMenuBar(m_hWnd);

  HRESULT hr = ::SendMessage(bar, SHCMBM_OVERRIDEKEY, VK_TBACK,
                             MAKELPARAM(SHMBOF_NODEFAULT | SHMBOF_NOTIFY,
                             SHMBOF_NODEFAULT | SHMBOF_NOTIFY));

  is_smartphone_ = CheckIsSmartPhone();
  resizing_ = false;
  right_button_enabled_ = false;

  InitBrowserView();
  HtmlDialogHost::html_permissions_dialog_ = this;
  SendMessage(browser_view_, DTM_CLEAR, 0, 0);

  void* html_file;
  int html_size;

  if SUCCEEDED(LoadFromResource(url_, &html_file, &html_size)) {
    char* content = new char[html_size+1];
    memcpy(content, html_file, html_size);
    content[html_size] = '\0';
    CHK(SendMessage(browser_view_, DTM_ADDTEXT, FALSE,
                    reinterpret_cast<LPARAM> (content)));
    CHK(SendMessage(browser_view_, DTM_ENDOFSOURCE, 0, 0));
    ResizeWindow();
    delete [] content;
  }

  return true;
}

LRESULT HtmlDialogHost::ResizeWindow() {
  if (resizing_) return false;
  resizing_ = true;

  // We do not care about desired_size_.cx and cy, as on Windows Mobile the
  // screen resolution is rather small, and the DPI can also change (i.e., we
  // can have high resolution with the same physical space, and in that case
  // resizing the dialog as if it was a classic 72 dpi will not bring nice
  // results...). So we compute the dialog size ourself, according to the
  // available screen size, the kind of device, and the html content.

  int screen_width = GetSystemMetrics(SM_CXSCREEN);
  int screen_height = GetSystemMetrics(SM_CYSCREEN);
  int scrollbar_width = GetSystemMetrics(SM_CXVSCROLL);
  int scrollbar_height = GetSystemMetrics(SM_CYHSCROLL);
  int menu_height = GetSystemMetrics(SM_CYMENU);

  int border = 2;
  int offscreen = -5000;
  int max_height = screen_height -2*menu_height -2*border;

  // WinMo 5.0 bug, SM_CYMENU wrongly reported as 23 in VGA!
  if ((screen_height > 320) && (screen_width > 320))
    menu_height = 52;
  
  screen_height -= 2* menu_height;

  int desired_width = screen_width;
  int desired_height = max_height;

  int pos_x = (screen_width - desired_width) / 2;
  int pos_y = (screen_height - desired_height) / 2;
  pos_y += menu_height;
  RECT rc;

  if (is_smartphone_) {
    // On Smartphone we just display the dialog fullscreen.

    MoveWindow(pos_x, pos_y, desired_width, desired_height, false);
    GetClientRect(&rc);

    int html_width = (rc.right - rc.left) - 2*border;
    int html_height = (rc.bottom - rc.top) - 2*border;

    ::MoveWindow(browser_view_, border, border,
                 html_width, html_height, false);
  } else {
    // On winmo we usually have bigger screen size and can resize the dialog
    // properly according to its content and center it.

    MoveWindow(offscreen, offscreen, desired_width, desired_height, false);
    GetClientRect(&rc);

    int html_width = (rc.right - rc.left) - 2*border;
    int html_height = (rc.bottom - rc.top) - 2*border;

    int delta = desired_height - html_height;

    ::MoveWindow(browser_view_, offscreen+border, offscreen+border,
                 html_width, html_height, false);

    int height = 2*border + static_cast<int>(::SendMessage(browser_view_,
                                               DTM_LAYOUTHEIGHT, 0, 0L));

    if (height + delta < desired_height)
      desired_height = height + delta;

    if (desired_height > max_height)
      desired_height = max_height;

    pos_y = (screen_height - desired_height) / 2;
    pos_y += menu_height;

    MoveWindow(pos_x, pos_y, desired_width, desired_height);

    GetClientRect(&rc);
    html_height = (rc.bottom - rc.top) - 2*border;

    ::MoveWindow(browser_view_, border, border,
                 html_width, html_height, true);

  } 

  HWND inner_browser_view_ = ::GetWindow(browser_view_, GW_CHILD);
  ::SetFocus(inner_browser_view_);

  resizing_ = false;

  return true;
}

LRESULT HtmlDialogHost::OnHotKey(UINT uMsg, WPARAM wParam, LPARAM lParam,
                                 BOOL& bHandled) {
  // Intercept the back key and send a command event instead (simulating
  // pressing the cancel button).
  if (HIWORD(lParam) == VK_TBACK) {
    SHSendBackToFocusWindow(uMsg, wParam, lParam);
    SendMessage(WM_COMMAND, ID_CANCEL, NULL);
  }
  return 0;
}

LRESULT HtmlDialogHost::OnCommand(UINT uMsg, WPARAM wParam,
                                   LPARAM lParam, BOOL& bHandled) {
  switch (wParam) {
    case ID_CANCEL:
      // TODO: For the moment we only need to cancel a dialog by
      // exiting that way. It may be interesting to add similar JS functions
      // for the left softkey as for the right one (ie, able to set a script,
      // and a button label).
      CloseDialog(CComBSTR(""));
      break;
    case ID_ALLOW:
      RightButtonAction();
      break;
  }

  return 0;
}

LRESULT HtmlDialogHost::OnSettingChange(UINT uMsg, WPARAM wParam,
                                        LPARAM lParam, BOOL& bHandled) {
  ResizeWindow();
  return true;
}

LRESULT HtmlDialogHost::OnResize(UINT uMsg, WPARAM wParam, LPARAM lParam,
                                 BOOL& bHandled) {
  ResizeWindow();
  return true;
}

// The OnNotify() function is called upon WM_NOTIFY.
// We just process the NM_INLINE_IMAGE and NM_INLINE_STYLE notifications to
// load images or css includes manually from the dll, as by default the HTML
// control will not load anything automatically unless we use the navigate
// message. Of course this does not work on res:// url, so..
LRESULT HtmlDialogHost::OnNotify(UINT uMsg, WPARAM wParam, LPARAM lParam,
                                 BOOL& bHandled) {
  NM_HTMLVIEW* html_view = reinterpret_cast<NM_HTMLVIEW*> (lParam);
  switch (html_view->hdr.code) {
    case NM_INLINE_IMAGE:
    case NM_INLINE_STYLE:
    {
      const char* str = reinterpret_cast<const char*> (html_view->szTarget);
      CString rsc_name(str);
      if (((html_view->hdr.code == NM_INLINE_STYLE)
          && SUCCEEDED(LoadCSS(rsc_name)))
          || SUCCEEDED(LoadImage(rsc_name, html_view->dwCookie))) {
        SetWindowLong(DWL_MSGRESULT, true);
        return true;
      }
      break;
    }
  }
  return false;
}

LRESULT HtmlDialogHost::OnClose(UINT message, WPARAM w, LPARAM l,
                                BOOL& handled) {
  HtmlDialogHost::html_permissions_dialog_ = NULL;
  return EndDialog(IDCANCEL);
}

WNDPROC inner_browser_wndproc;

LRESULT CALLBACK innerBrowserWinProc(HWND hWnd, UINT uMsg, WPARAM wParam,
                                     LPARAM lParam) {
  // There is a bug on Windows Mobile Standard, where some key events are
  // not received by the HTML control if the parent window is a dialog.
  // Answering the WM_GETDLGCODE message with DLGC_WANTMESSAGE fix it.
  switch (uMsg) {
    case WM_GETDLGCODE:
        return DLGC_WANTMESSAGE;
  }
  return CallWindowProc(inner_browser_wndproc, hWnd, uMsg, wParam, lParam);
}

void HtmlDialogHost::InitBrowserView() {
  RECT rc;
  GetClientRect(&rc);

  HINSTANCE module_instance = _AtlBaseModule.GetModuleInstance();
  InitHTMLControl(module_instance);
  DWORD mask = WS_CHILD | WS_VISIBLE | WS_BORDER;
  browser_view_ = CreateWindow(WC_HTML, NULL, mask,
                               2, 2, (rc.right - rc.left) -4,
                               (rc.bottom - rc.top) -4,
                               m_hWnd, NULL,  module_instance, NULL);

  // We need to intercept some messages to work around a bug on Windows
  // Mobile Standard. See the innerBrowserWinProc() function.
  HWND inner_browser_view = ::GetWindow(browser_view_, GW_CHILD);
  inner_browser_wndproc = reinterpret_cast<WNDPROC>
                           (::SetWindowLong(inner_browser_view, 
                              GWL_WNDPROC, 
                              reinterpret_cast<LONG>(innerBrowserWinProc)));

  SendMessage(browser_view_, DTM_ENABLESCRIPTING, 0, TRUE);
  SendMessage(browser_view_, DTM_DOCUMENTDISPATCH, 0, 
              reinterpret_cast<LPARAM> (&document_));
}

// This function returns a pointer to a resource contained in the dll
// NOTE: LockResource() does not do anything on Winmo...
HRESULT HtmlDialogHost::LoadFromResource(CString rsc, void** resource, 
                                         int* len) {
  HMODULE hmodule = _AtlBaseModule.GetModuleInstance();
  HRSRC rscInfo = FindResource(hmodule, rsc, HTML);
  if (rscInfo) {
    HGLOBAL rscData = LoadResource(hmodule, rscInfo);
    *resource = LockResource(rscData);
    int size = SizeofResource(hmodule, rscInfo);
  
    if ((size == 0) && rscData) {
      // Workaround for windows mobile 6 standard devices (ex-smartphones).
      // For some reason SizeofResource does not work(!). In the resource file
      // (gears/ui/ie/ui_resources.rc.m4) we add after each resource a dummy
      // text resource containing the string "\0END\0".
      // This dummy resource is named by appending ".end" to the previous
      // resource name. As the resources are aggregated linearly we use the
      // address of the dummy resource as an indication of the end of the
      // previous resource.

      HRSRC e_rscInfo = FindResource(hmodule, rsc + L".end", HTML);
      if (e_rscInfo) {
        HGLOBAL e_rscData = LoadResource(hmodule, e_rscInfo);
        char* end = static_cast<char*> (LockResource(e_rscData));
        char* start = static_cast<char*> (*resource);
        size = end - start;
      } else {
        char* c = static_cast<char*> (*resource);
        size = strlen(c);
      }
    }
    *len = size;
    return S_OK;
  }
  return E_FAIL;
}

HRESULT HtmlDialogHost::LoadCSS(CString rsc) {
  void* css_file;
  int css_size;

  if SUCCEEDED(LoadFromResource(rsc, &css_file, &css_size)) {
    char* content = new char[css_size+1];
    memcpy(content, css_file, css_size);
    content[css_size] = '\0';

    HRESULT hr = SendMessage(browser_view_, DTM_ADDSTYLE, 0, 
                             reinterpret_cast<LPARAM> (content));
    delete [] content;
    return hr;
  } else {
    return E_FAIL;
  }
}

HRESULT HtmlDialogHost::LoadImage(CString rsc, DWORD cookie) {
  IImagingFactory *image_factory = NULL;
  IImage *image = NULL;
  ImageInfo image_info;
  void* image_resource;
  int image_size;
  HRESULT hr;

  hr = CoCreateInstance(CLSID_ImagingFactory, NULL, CLSCTX_INPROC_SERVER,
                        __uuidof(IImagingFactory),
                        reinterpret_cast<void**>(&image_factory));

  if (hr == S_OK) {
    hr = LoadFromResource(rsc, &image_resource, &image_size);
  }

  if (hr == S_OK) {
    image_factory->CreateImageFromBuffer(image_resource,
                                         image_size,
                                         BufferDisposalFlagNone,
                                         &image);
  }

  if (hr == S_OK) {
    hr = image->GetImageInfo(&image_info);
  }

  if (hr == S_OK) {
    HDC dc = GetDC();
    HDC bitmap_dc = CreateCompatibleDC(dc);

    // We need to convert our pImage to a HBITMAP.
    HBITMAP bitmap = CreateCompatibleBitmap(dc, image_info.Width,
                                            image_info.Height);

    if (bitmap) {
      HGDIOBJ old_bitmap_dc = SelectObject(bitmap_dc, bitmap);
      HGDIOBJ brush = GetStockObject(WHITE_BRUSH);
      if (brush) {
        FillRect(bitmap_dc, CRect(0, 0, image_info.Width, image_info.Height),
                 (HBRUSH)brush);
      }
      image->Draw(bitmap_dc, CRect(0, 0, image_info.Width,
                  image_info.Height), NULL);
      SelectObject(bitmap_dc, old_bitmap_dc);
    }

    // We can now fill the structure with our image.
    INLINEIMAGEINFO inline_image_info;
    inline_image_info.dwCookie = cookie;
    inline_image_info.iOrigHeight = image_info.Height;
    inline_image_info.iOrigWidth = image_info.Width;
    inline_image_info.hbm = bitmap;
    // The bitmap will be destructed when the page is closed.
    inline_image_info.bOwnBitmap = TRUE;

    SendMessage(browser_view_, DTM_SETIMAGE, FALSE, 
                reinterpret_cast<LPARAM>(&inline_image_info));
    DeleteDC(bitmap_dc);
  }

  if (image)
    image->Release();

  if (image_factory)
    image_factory->Release();

  return hr;
}

bool HtmlDialogHost::CheckIsSmartPhone() {
  wchar_t buffer[256];
  SystemParametersInfo(SPI_GETPLATFORMTYPE, 256, buffer, false);
  CString platformType(buffer);
  if (platformType.Compare(kSmartPhone) == 0) {
    return true;
  } else {
    return false;
  }
}

HRESULT HtmlDialogHost::ExecScript(const BSTR script) {
  // Note: following code copied from js_runner_ie.cc
  if (!script) return E_FAIL;
  DISPID function_iid;

  if (FAILED(ActiveXUtils::GetDispatchMemberId(html_window_,
                                               STRING16(L"eval"),
                                               &function_iid))) {
    return E_FAIL;
  }
  CComVariant script_variant(script);
  DISPPARAMS parameters = {0};
  parameters.cArgs = 1;
  parameters.rgvarg = &script_variant;
  return SUCCEEDED(html_window_->Invoke(
        function_iid,           // member to invoke
        IID_NULL,               // reserved
        LOCALE_SYSTEM_DEFAULT,  // TODO(cprince): should this be user default?
        DISPATCH_METHOD,        // dispatch/invoke as...
        &parameters,            // parameters
        NULL,                   // receives result (NULL okay)
        NULL,                   // receives exception (NULL okay)
        NULL));                 // receives badarg index (NULL okay)
  return S_OK;
}

HRESULT HtmlDialogHost::RightButtonAction() {
  if (right_button_enabled_) {
    return ExecScript(allow_action_);
  }
  return E_FAIL;
}

/*
 * The following functions are called by Javascript
 */

HRESULT HtmlDialogHost::GetDialogArguments(BSTR *args_string) {
  // For convenience, we interpret a null string the same as "null".
  if (dialog_arguments_ == NULL) {
    (*args_string) = NULL;
    return S_OK;
  } else {
    return dialog_arguments_.CopyTo(args_string);
  }
}

HRESULT HtmlDialogHost::CloseDialog(const BSTR result_string) {
  dialog_result_ = result_string;

  // We put back the original WinProc before exiting
  HWND inner_browser_view_ = ::GetWindow(browser_view_, GW_CHILD);
  ::SetWindowLong(inner_browser_view_, GWL_WNDPROC, 
                  reinterpret_cast<LONG>(inner_browser_wndproc)); 
  return EndDialog(IDCANCEL);
}

HRESULT HtmlDialogHost::IsPocketIE(VARIANT_BOOL *retval) {
  *retval = VARIANT_TRUE;
  return S_OK;
}

HRESULT HtmlDialogHost::IsSmartPhone(VARIANT_BOOL *retval) {
  if (is_smartphone_)
    *retval = VARIANT_TRUE;
  else
    *retval = VARIANT_FALSE;
  return S_OK;
}

HRESULT HtmlDialogHost::ResizeDialog() {
  ResizeWindow();
  return S_OK;
}

HRESULT HtmlDialogHost::SetScriptContext(IDispatch *val) {
  html_window_ = val;
  return S_OK;
}

HRESULT HtmlDialogHost::SetButton(const BSTR label, const BSTR script) {
  allow_action_ = script;
  
  HWND bar = SHFindMenuBar(m_hWnd);
  TBBUTTONINFO tbbi;
  tbbi.cbSize = sizeof(TBBUTTONINFO);
  tbbi.dwMask = TBIF_TEXT;
  tbbi.pszText = label;
  ::SendMessage(bar, TB_SETBUTTONINFO, ID_ALLOW, 
    reinterpret_cast<LPARAM>(&tbbi));

  return S_OK;
}

HRESULT HtmlDialogHost::SetButtonEnabled(VARIANT_BOOL *val) {
  HWND bar = SHFindMenuBar(m_hWnd);

  if (*val == VARIANT_TRUE) {
    TBBUTTONINFO tbbi;
    tbbi.cbSize = sizeof(TBBUTTONINFO);
    tbbi.dwMask = TBIF_STATE;
    tbbi.fsState = TBSTATE_ENABLED;
    ::SendMessage(bar, TB_SETBUTTONINFO, ID_ALLOW, 
      reinterpret_cast<LPARAM>(&tbbi));
    right_button_enabled_ = true;
  } else {
    TBBUTTONINFO tbbi;
    tbbi.cbSize = sizeof(TBBUTTONINFO);
    tbbi.dwMask = TBIF_STATE;
    tbbi.fsState = TBSTATE_INDETERMINATE;
    ::SendMessage(bar, TB_SETBUTTONINFO, ID_ALLOW, 
      reinterpret_cast<LPARAM>(&tbbi));
    right_button_enabled_ = false;
  }

  return S_OK;
}

HRESULT HtmlDialogHost::SetCancelButton(const BSTR label) {
  HWND bar = SHFindMenuBar(m_hWnd);
  TBBUTTONINFO tbbi;
  tbbi.cbSize = sizeof(TBBUTTONINFO);
  tbbi.dwMask = TBIF_TEXT;
  tbbi.pszText = label;
  ::SendMessage(bar, TB_SETBUTTONINFO, ID_CANCEL, 
    reinterpret_cast<LPARAM>(&tbbi));

  return S_OK;
}