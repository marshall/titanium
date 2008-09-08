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

#include "gears/ui/ie/html_dialog_host.h"

#include <assert.h>
#include <oleidl.h>
#include <windows.h>

#include "gears/base/common/common.h"
#include "gears/base/common/paths.h"
#include "gears/base/common/string_utils.h"
#include "gears/base/common/url_utils.h"
#include "gears/base/common/thread_locals.h"
#include "gears/base/ie/atl_headers.h"


// The timer associated with making sure navigation errors don't cause a
// problem.
const unsigned int kNavigationErrorTimer = 0;

// Some macros defined here to keep changes from original source to a low roar.
// See //depot/googleclient/bar/common/(debugbase|macros).h for original
// definitions.
#define ASSERT(expr) assert(expr);

// Checks for HRESULT and if it fails returns. The macro will ASSERT in debug.
#define CHK(cmd) \
do { \
  HRESULT __hr = (cmd); \
  if(FAILED(__hr)) { \
    ASSERT(false); \
    return __hr; \
  } \
} \
while(0);

// Verify an operation succeeded and ASSERT on failure. Only asserts in DEBUG.
#ifdef DEBUG
#define VERIFY(expr) ASSERT(expr)
#else
#define VERIFY(expr) expr
#endif

#define ERROR_MSG(msg) \
{ \
  LOG((msg)); \
  assert(false); \
}


const ThreadLocals::Slot kThreadLocalKey = ThreadLocals::Alloc();


static LRESULT CALLBACK MessageProcStatic(int code, WPARAM wparam,
                                          LPARAM lparam) {
  HtmlDialogHost *instance = reinterpret_cast<HtmlDialogHost*>(
      ThreadLocals::GetValue(kThreadLocalKey));

  if (!instance) {
    assert(false);
    LOG(("HtmlDialogHost::MessageProcStatic: could not get current instance"));
    return 0;
  } else {
    return instance->MessageProc(code, wparam, lparam);
  }
}


LRESULT HtmlDialogHost::MessageProc(int code, WPARAM wparam, LPARAM lparam) {
  if (code >= 0) {
    MSG* message = reinterpret_cast<MSG*>(lparam);
    if (message->message >= WM_KEYFIRST && message->message <= WM_KEYLAST &&
        ::IsChild(m_hWnd, message->hwnd)) {
      CComPtr<IOleInPlaceActiveObject> active_object;
      VERIFY(SUCCEEDED(browser_->QueryInterface(&active_object)));

      // Enables access keys in side the browser control.
      if (active_object != NULL) {
        active_object->TranslateAccelerator(message);
      }

      // Prevent the dialog box from handling certain messages.
      // - tab: If the dialog handles this, if shifts focus away from the
      //   browser control each time tab is pressed.
      // - syschar: This is ALT+<anything except F4>. We prevent the dialog
      //   from handling these because otherwise it beeps because there is no
      //   menu bar visible.
      if (message->wParam == VK_TAB || message->message == WM_SYSCHAR) {
        ZeroMemory(message, sizeof(MSG));
      }
    }
  }

  return CallNextHookEx(hook_, code, wparam, lparam);
}


HRESULT HtmlDialogHost::InitBrowser() {
  // InitBrowser should only be called once.
  ASSERT(!browser_window_.IsWindow());

  // Get the HTML control.
  HWND browser_hwnd = GetDlgItem(IDC_GENERIC_HTML);
  if (!browser_hwnd)
    return E_FAIL;
  browser_window_.Attach(browser_hwnd);
  
  // We need to implement IInternetSecurityManager so that the dialog works when
  // IE's security settings are set to high.
  CComPtr<IObjectWithSite> object_with_site;
  CHK(browser_window_.QueryHost(&object_with_site));
  CHK(object_with_site->SetSite(GetUnknown()));

  // Get the web browser object.
  CHK(browser_window_.QueryControl(&browser_));

  // Set ourself as the window.external object.
  VERIFY(SUCCEEDED(browser_window_.SetExternalDispatch(this)));

  // Have the web browser start sending us events.
  VERIFY(SUCCEEDED(DispEventAdvise(browser_)));

  // Disable the drag and drop functionality.
  VERIFY(SUCCEEDED(browser_->put_RegisterAsDropTarget(VARIANT_FALSE)));

  // Get the IAxWinAmbientDispatch.  IAxWinAmbientDispatch and
  // IAxWinAmbientDispatchEx are interfaces used internally by ATL.
  CComPtr<IAxWinAmbientDispatch> ambient;
  VERIFY(SUCCEEDED(browser_window_.QueryHost(&ambient)));
  ASSERT(ambient != NULL);

  // Twiddle document flags to make sure the appearance is appropriate.
  DWORD doc_host_flags = 0;
  VERIFY(SUCCEEDED(ambient->get_DocHostFlags(&doc_host_flags)));
  doc_host_flags = (doc_host_flags & ~DOCHOSTUIFLAG_SCROLL_NO) |
                   DOCHOSTUIFLAG_FLAT_SCROLLBAR | DOCHOSTUIFLAG_THEME;
  VERIFY(SUCCEEDED(ambient->put_DocHostFlags(doc_host_flags)));

  // Disable the context menu.
  VERIFY(SUCCEEDED(ambient->put_AllowContextMenu(VARIANT_FALSE)));

  return S_OK;
}


HRESULT HtmlDialogHost::NavigateToUrl(const char16 *url) {
  ASSERT(browser_ != NULL);
  if (browser_->Navigate(CComBSTR(url), 0, 0, 0, 0) != S_OK) {
    ERROR_MSG("NavigateToUrl() Navigate2 failed.");
    return E_FAIL;
  }

  KillTimer(kNavigationErrorTimer);
  SetTimer(kNavigationErrorTimer, 5000, NULL);

  return S_OK;
}


void _stdcall HtmlDialogHost::OnDocumentComplete(IDispatch* disp, VARIANT* url) {
  ASSERT(disp != NULL);
  ASSERT(url != NULL);

  // Sanity check for the web control.
  // The interface we query here and the interface we've got when we
  // instantiated the control must be the same.
  CComPtr<IWebBrowser2> browser;
  VERIFY(SUCCEEDED(disp->QueryInterface(&browser)));
  if (browser != browser_)
    return;  // This is a frame or iframe, not the root document

  VERIFY(SUCCEEDED(UpdateDocPointers()));
  OnReadyStateChanged();
}


LRESULT HtmlDialogHost::OnInitDialog(UINT message, WPARAM w, LPARAM l,
    BOOL& handled) {
  // Validate member variables.
  ASSERT(!url_.IsEmpty());
  ASSERT(desired_size_.cx > 0 && desired_size_.cy > 0);
  ASSERT(!is_position_set_);

  // Make sure window is offscreen so the dialog doesn't display
  // prior to loading the page.
  MoveWindow(CRect(-5000, -5000, 0, 0));

  // Prepare the dialog.
  VERIFY(SUCCEEDED(InitBrowser()));
  CAxDialogImpl<HtmlDialogHost>::OnInitDialog(message, w, l, handled);

  // Initially size the browser control to its desired size so as the page
  // loads, it will observe that size layout accordingly.
  browser_window_.MoveWindow(CRect(0, 0, desired_size_.cx, desired_size_.cy));

  // Navigate to the first page
  VERIFY(SUCCEEDED(NavigateToUrl(url_)));
  return TRUE;
}


void HtmlDialogHost::OnReadyStateChanged() {
  UpdateCaption();
  SetDialogPosition();
}


LRESULT HtmlDialogHost::OnClose(UINT message, WPARAM w, LPARAM l,
                                BOOL& handled) {
  return EndDialog(IDCANCEL);
}


LRESULT HtmlDialogHost::OnTimer(UINT message, WPARAM w, LPARAM l,
                                BOOL& handled) {
  unsigned int timer = static_cast<unsigned int>(w);
  if (timer == kNavigationErrorTimer) {
    SetMsgHandled(TRUE);
    SetDialogPosition();
  } else {
    SetMsgHandled(FALSE);
  }
  return TRUE;
}


LRESULT HtmlDialogHost::OnSize(UINT message, WPARAM w, LPARAM l,
                               BOOL& handled) {
  // The user has resized the dialog window frame.
  if (is_position_set_ && browser_window_.IsWindow()) {
    int width = GET_X_LPARAM(l);  // width of client area
    int height = GET_Y_LPARAM(l); // height of client area
    browser_window_.MoveWindow(CRect(0, 0, width, height));
  }
  handled = FALSE;
  return 0;
}


HRESULT HtmlDialogHost::resizeBy(LONG x, LONG y) {
  // Script in the hosted page has called window.resizeBy(). When
  // called prior to the initial display of the window, we adjust 
  // the 'desired_size_' which will be used to set the actual window
  // position later. Otherwise, we actually resize the window.

  // Get our current bounds
  CRect bounds;
  if (is_position_set_) {
    if (!GetWindowRect(&bounds))
      return S_OK;  // give up if anything goes wrong
  } else {
    bounds.SetRect(0, 0, desired_size_.cx, desired_size_.cy);
  }

  // Constrain new bounds based on the desktop size
  CRect desktop;
  if (!::GetWindowRect(GetDesktopWindow(), &desktop))
    return S_OK;  // give up if anything goes wrong
  const int kMinDimension = 32;
  int width = bounds.Width() + static_cast<int>(x);
  int height = bounds.Height() + static_cast<int>(y);
  if (width < desktop.Width() && width > kMinDimension)
    bounds.right = bounds.left + width;
  if (height < desktop.Height() && height > kMinDimension)
    bounds.bottom = bounds.top + height;

  // Set our new bounds
  if (is_position_set_) {
    MoveWindow(bounds);
  } else {
    desired_size_ = bounds.Size();
  }
  return S_OK;
}


HRESULT HtmlDialogHost::ProcessUrlAction(LPCWSTR url, DWORD action, 
                                         BYTE *policy,
                                         DWORD policy_size, BYTE *context,
                                         DWORD context_size, DWORD flags,
                                         DWORD reserved) {
  // As a precaution, we only let the browser control load res:// URLs that are
  // from our DLL. Any URL that is prefixed with our base url should be safe.
  if (wcsncmp(url, base_url_, base_url_.GetLength()) == 0) {
    return S_OK;
  }

  return INET_E_DEFAULT_ACTION;
}

HRESULT HtmlDialogHost::MapUrlToZone(LPCWSTR url, DWORD *zone, DWORD flags) {
  // If the file is in our data directory, and is a png file, trust it.
  size_t length = wcslen(url);
  if (wcsncmp(url, data_url_, data_url_.GetLength()) == 0 &&
      length >= 4 &&
      _wcsicmp(&url[length - 4], L".png") == 0) {
    *zone = URLZONE_TRUSTED;
    return S_OK;
  }

  // Run default IInternetSecurityManager behavior.
  return INET_E_DEFAULT_ACTION;
}


void HtmlDialogHost::UpdateCaption() {
  CComBSTR caption;
  if (FAILED(document_->get_title(&caption))) {
    ERROR_MSG("Could not get document title");
    return;
  }
  SetWindowText(caption);
}


void HtmlDialogHost::SetDialogPosition() {
  // Only set the position once.
  if (is_position_set_)
    return;

  KillTimer(kNavigationErrorTimer);

  // Get the parent information
  bool has_parent(false);
  HWND parent = GetParent();
  CRect parent_bounds;
  CPoint parent_center(0, 0);
  CPoint window_location(0, 0);
  if (parent != NULL && ::GetWindowRect(parent, &parent_bounds)) {
    has_parent = true;
    // Figure out the center of the parent
    parent_center.x = parent_bounds.left + parent_bounds.Width() / 2;
    parent_center.y = parent_bounds.top + parent_bounds.Height() / 2;
    // Put in an initial dialog location estimate
    window_location.x = parent_center.x - desired_size_.cx / 2;
    window_location.y = parent_center.y - desired_size_.cy / 2;
  }

  // Move the window to the initial location - shouldn't even flicker, but
  // get the location as close as possible to be safe.
  MoveWindow(CRect(window_location, desired_size_));
  // Figure out the delta in client size
  CRect client_rect;
  GetClientRect(&client_rect);
  CSize window_size(2 * desired_size_.cx - client_rect.Width(),
                    2 * desired_size_.cy - client_rect.Height());

  // Figure out the final location
  if (has_parent) {
    window_location.x = parent_center.x - window_size.cx / 2;
    window_location.y = parent_center.y - window_size.cy / 2;
  }

  // Move the windows to their final locations
  MoveWindow(CRect(window_location, window_size));
  browser_window_.MoveWindow(CRect(0, 0, desired_size_.cx, desired_size_.cy));

  is_position_set_ = true;
}


bool HtmlDialogHost::SetUp(const char16 *resource_file_name,
                           const CSize& size) {
  if (!AtlAxWinInit()) {
    ERROR_MSG("AtlAxWinInit() failed.");
    return false;
  }

  if (!InitBaseUrl()) {
    return false;
  }

  desired_size_ = size;
  url_ = base_url_;
  url_ += resource_file_name;

  if (ThreadLocals::HasValue(kThreadLocalKey)) {
    LOG(("HtmlDialogHost::SetUp: Cannot open dialog because there is already "
         "one open"));
    return false;
  }

  hook_ = ::SetWindowsHookEx(WH_GETMESSAGE, MessageProcStatic, NULL,
                             GetCurrentThreadId());

  ThreadLocals::SetValue(kThreadLocalKey, this, NULL);
  return true;
}


bool HtmlDialogHost::ShowDialog(const char16 *resource_file_name,
                                const CSize& size, const BSTR dialog_arguments,
                                BSTR *dialog_result) {
  dialog_arguments_ = dialog_arguments;

  if (!SetUp(resource_file_name, size)) 
    return false;
  
  DoModal();

  if (dialog_result_ == NULL) {
    (*dialog_result) = NULL;
  } else {
    dialog_result_.CopyTo(dialog_result);
  }

  TearDown();
  return true;
}


void HtmlDialogHost::TearDown() {
  // Now we have a chance to release up the web browser control.
  // FinalRelease or the destructor of this class are inappropriate since the
  // ref counting mechanism does not work anymore at that time. Therefore,
  // we have to release all the interfaces before the flow
  // reaches the destructor. This is related to the nature of the WebUI object.
  // It is used like a C++ object by the clients but it has also exposed itself
  // as a COM object for the window.external and the web control sink.
  if (browser_)
    DispEventUnadvise(browser_);

  if (browser_window_.IsWindow())
    browser_window_.DestroyWindow();

  if (IsWindow())
    DestroyWindow();

  HtmlDialogHost *current_instance = reinterpret_cast<HtmlDialogHost*>(
      ThreadLocals::GetValue(kThreadLocalKey));

  if (current_instance != this) {
    // Sanity check that we are the current (only) instance on this thread.
    assert(false);
    LOG(("HtmlDialogHost::TearDown: Unexpected current_instance for thread"));
  }

  ::UnhookWindowsHookEx(hook_);
  ThreadLocals::DestroyValue(kThreadLocalKey);

  hook_ = NULL;
  AtlAxWinTerm();
}


HRESULT HtmlDialogHost::UpdateDocPointers() {
  // Retrieve the automation object of the active document.
  CComPtr<IDispatch> disp_doc;
  CHK(browser_->get_Document(&disp_doc));

  // Query for the IHTMLDocument2.
  document_ = 0;
  CHK(disp_doc->QueryInterface(&document_));

  return S_OK;
}


bool HtmlDialogHost::InitBaseUrl() {
  char16 module_path[MAX_PATH];
  if (0 == GetModuleFileName(_AtlBaseModule.GetModuleInstance(), 
                             &(module_path[0]), MAX_PATH)) {
    ERROR_MSG("GetModuleFileName() failed.");
    return false;
  }

  base_url_ = STRING16(L"res://");
  base_url_ += module_path;
  base_url_ += L'/';

  std::string data_url_utf8;
  std::string16 data_url;
  if (!GetBaseDataDirectory(&data_url)) {
    ERROR_MSG("GetBaseDataDirectory() failed.");
    return false;
  }

  if (!String16ToUTF8(data_url.c_str(), &data_url_utf8)) {
    ERROR_MSG("String16ToUTF8() failed.");
    return false;
  }

  data_url_utf8 = UTF8PathToUrl(data_url_utf8, true);

  if (!UTF8ToString16(data_url_utf8.c_str(), &data_url)) {
    ERROR_MSG("UTF8ToString16() failed.");
    return false;
  }

  data_url_ = data_url.c_str();

  return true;
}


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
  return EndDialog(IDCANCEL);
}


HRESULT HtmlDialogHost::IsPocketIE(VARIANT_BOOL *retval) {
  *retval = VARIANT_FALSE;
  return S_OK;
}
