// Copyright 2008, Google Inc.
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

#ifdef OFFICIAL_BUILD
// The Drag-and-Drop API has not been finalized for official builds.
// Nor is it implemented on Windows CE.
#else
#ifdef WINCE
// The Drag-and-Drop API is not implemented on Windows CE.
#else
#include "gears/desktop/drop_target_ie.h"

#include <mshtmdid.h>
#include "gears/base/common/string16.h"
#include "gears/base/ie/activex_utils.h"
#include "gears/desktop/drag_and_drop_registry.h"
#include "gears/desktop/file_dialog.h"


HRESULT DropTarget::Detach(void) {
  return S_OK;
}


HRESULT DropTarget::Init(IElementBehaviorSite *element_behavior_site) {
  element_behavior_site_ = element_behavior_site;
  return S_OK;
}


HRESULT DropTarget::Notify(long lEvent, VARIANT *var) {
  HRESULT hr;
  if (lEvent != BEHAVIOREVENT_CONTENTREADY) {
    return S_OK;
  }
  CComPtr<IHTMLElement> html_element;
  hr = element_behavior_site_->GetElement(&html_element);
  if (FAILED(hr)) return hr;

  // TODO(nigeltao): Keep the cookie somewhere, and Unadvise (i.e. detach
  // the behavior from the element) at an appropriate time.
  DWORD ignored_cookie;
  hr = AtlAdvise(html_element, reinterpret_cast<IUnknown*>(this),
     DIID_HTMLElementEvents2, &ignored_cookie);
  if (FAILED(hr)) return hr;
  
  return S_OK;
}


HRESULT DropTarget::FindBehavior(
    BSTR name,
    BSTR url,
    IElementBehaviorSite *behavior_site,
    IElementBehavior **behavior_out) {
  if (!name || (_wcsicmp(name, L"DropTarget") != 0)) {
    return E_FAIL;
  }
  return QueryInterface(__uuidof(IElementBehavior),
      reinterpret_cast<void**>(behavior_out));
}


HRESULT DropTarget::Invoke(DISPID dispidMember, REFIID riid,
    LCID lcid, WORD wFlags, DISPPARAMS *pdispparams, VARIANT *pvarResult,
    EXCEPINFO *pexcepinfo, UINT *puArgErr)
{
  switch (dispidMember) {
    case DISPID_HTMLELEMENTEVENTS_ONDRAGENTER:
      return HandleOnDragEnter();
    case DISPID_HTMLELEMENTEVENTS_ONDRAGOVER:
      return HandleOnDragOver();
    case DISPID_HTMLELEMENTEVENTS_ONDRAGLEAVE:
      return HandleOnDragLeave();
    case DISPID_HTMLELEMENTEVENTS_ONDROP:
      return HandleOnDragDrop();
    default:
      // Do nothing;
      break;
  }
  return IDispatchImpl<IDispatch>::Invoke(dispidMember, riid, lcid, wFlags,
      pdispparams, pvarResult, pexcepinfo, puArgErr);
}


HRESULT DropTarget::GetHtmlDataTransfer(
    CComPtr<IHTMLEventObj> &html_event_obj,
    CComPtr<IHTMLDataTransfer> &html_data_transfer)
{
  HRESULT hr;
  if (!html_window_2_) {
    CComPtr<IHTMLElement> html_element;
    hr = element_behavior_site_->GetElement(&html_element);
    if (FAILED(hr)) return hr;
    CComPtr<IDispatch> dispatch;
    hr = html_element->get_document(&dispatch);
    if (FAILED(hr)) return hr;
    CComQIPtr<IHTMLDocument2> html_document_2(dispatch);
    if (!html_document_2) return E_FAIL;
    hr = html_document_2->get_parentWindow(&html_window_2_);
    if (FAILED(hr)) return hr;
  }
  hr = html_window_2_->get_event(&html_event_obj);
  if (FAILED(hr)) return hr;
  CComQIPtr<IHTMLEventObj2> html_event_obj_2(html_event_obj);
  if (!html_event_obj_2) return E_FAIL;
  hr = html_event_obj_2->get_dataTransfer(&html_data_transfer);
  return hr;
}


HRESULT DropTarget::CancelEventBubble(
    CComPtr<IHTMLEventObj> &html_event_obj,
    CComPtr<IHTMLDataTransfer> &html_data_transfer)
{
  HRESULT hr;
  hr = html_data_transfer->put_dropEffect(L"copy");
  if (FAILED(hr)) return hr;
  hr = html_event_obj->put_returnValue(CComVariant(VARIANT_FALSE));
  if (FAILED(hr)) return hr;
  hr = html_event_obj->put_cancelBubble(VARIANT_TRUE);
  return hr;
}


HRESULT DropTarget::HandleOnDragEnter()
{
  CComPtr<IHTMLEventObj> html_event_obj;
  CComPtr<IHTMLDataTransfer> html_data_transfer;
  HRESULT hr = GetHtmlDataTransfer(html_event_obj, html_data_transfer);
  if (FAILED(hr)) return hr;


  CComPtr<IServiceProvider> service_provider;
  hr = html_data_transfer->QueryInterface(&service_provider);
  if (FAILED(hr)) return hr;
  CComPtr<IDataObject> data_object;
  hr = service_provider->QueryService<IDataObject>(IID_IDataObject,
                                                   &data_object);
  if (FAILED(hr)) return hr;

  CComPtr<IEnumFORMATETC> enum_format_etc;
  hr = data_object->EnumFormatEtc(DATADIR_GET, &enum_format_etc);
  if (FAILED(hr)) return hr;
  FORMATETC format_etc;
  bool in_file_drop = false;
  while (enum_format_etc->Next(1, &format_etc, NULL) == S_OK) {
    if (format_etc.cfFormat == CF_HDROP) {
      in_file_drop = true;
      break;
    }
  }
  if (!in_file_drop) return E_FAIL;

  if (on_drag_enter_.get()) {
    module_environment_->js_runner_->InvokeCallback(
        on_drag_enter_.get(), 0, NULL, NULL);
  }

  hr = CancelEventBubble(html_event_obj, html_data_transfer);
  if (FAILED(hr)) return hr;
  return S_OK;
}


HRESULT DropTarget::HandleOnDragOver()
{
  CComPtr<IHTMLEventObj> html_event_obj;
  CComPtr<IHTMLDataTransfer> html_data_transfer;
  HRESULT hr = GetHtmlDataTransfer(html_event_obj, html_data_transfer);
  if (FAILED(hr)) return hr;

  if (on_drag_over_.get()) {
    module_environment_->js_runner_->InvokeCallback(
        on_drag_over_.get(), 0, NULL, NULL);
  }

  hr = CancelEventBubble(html_event_obj, html_data_transfer);
  if (FAILED(hr)) return hr;
  return S_OK;
}


HRESULT DropTarget::HandleOnDragLeave()
{
  CComPtr<IHTMLEventObj> html_event_obj;
  CComPtr<IHTMLDataTransfer> html_data_transfer;
  HRESULT hr = GetHtmlDataTransfer(html_event_obj, html_data_transfer);
  if (FAILED(hr)) return hr;

  if (on_drag_leave_.get()) {
    module_environment_->js_runner_->InvokeCallback(
        on_drag_leave_.get(), 0, NULL, NULL);
  }

  hr = CancelEventBubble(html_event_obj, html_data_transfer);
  if (FAILED(hr)) return hr;
  return S_OK;
}


HRESULT DropTarget::HandleOnDragDrop()
{
  CComPtr<IHTMLEventObj> html_event_obj;
  CComPtr<IHTMLDataTransfer> html_data_transfer;
  HRESULT hr = GetHtmlDataTransfer(html_event_obj, html_data_transfer);
  if (FAILED(hr)) return hr;

  if (on_drop_.get()) {
    CComPtr<IServiceProvider> service_provider;
    hr = html_data_transfer->QueryInterface(&service_provider);
    if (FAILED(hr)) return hr;
    CComPtr<IDataObject> data_object;
    hr = service_provider->QueryService<IDataObject>(
        IID_IDataObject, &data_object);
    if (FAILED(hr)) return hr;

    FORMATETC desired_format_etc =
      { CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    STGMEDIUM stg_medium;
    hr = data_object->GetData(&desired_format_etc, &stg_medium);
    if (FAILED(hr)) return hr;

    std::vector<std::string16> filenames;
    UINT num_files = DragQueryFile((HDROP)stg_medium.hGlobal, -1, 0, 0);
    TCHAR buffer[MAX_PATH + 1];
    for (UINT i = 0; i < num_files; i++) {
      int filename_length =
          DragQueryFile((HDROP)stg_medium.hGlobal, i, buffer, sizeof(buffer));
      filenames.push_back(std::string16(buffer));
    }
    ::ReleaseStgMedium(&stg_medium);

    std::string16 error;
    scoped_ptr<JsArray> file_objects(
        module_environment_->js_runner_->NewArray());
    if (!FileDialog::FilesToJsObjectArray(
            filenames, module_environment_.get(), file_objects.get(), &error)) {
      return E_FAIL;
    }

    const int argc = 1;
    JsParamToSend argv[argc] = {
      { JSPARAM_ARRAY, file_objects.get() }
    };
    module_environment_->js_runner_->InvokeCallback(
        on_drop_.get(), argc, argv, NULL);
  }

  hr = CancelEventBubble(html_event_obj, html_data_transfer);
  if (FAILED(hr)) return hr;
  return S_OK;
}


// A DropTarget instance automatically de-registers itself, on page unload.
void DropTarget::HandleEvent(JsEventType event_type) {
  assert(event_type == JSEVENT_UNLOAD);
  DragAndDropRegistry::UnregisterDropTarget(this);
}


#endif  // WINCE
#endif  // OFFICIAL_BUILD
