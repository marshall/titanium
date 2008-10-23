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
#else
#include "gears/desktop/drop_target_ff.h"

#include <gecko_sdk/include/nsIFile.h>
#include <gecko_sdk/include/nsIFileURL.h>
#include <gecko_sdk/include/nsIDOMHTMLElement.h>
#include <gecko_sdk/include/nsIDOMEvent.h>
#include <gecko_sdk/include/nsIDOMEventTarget.h>
#include <gecko_sdk/include/nsIIOService.h>
#include <gecko_sdk/include/nsISupportsPrimitives.h>
#include <gecko_sdk/include/nsIURI.h>
#include "gears/desktop/drag_and_drop_registry.h"
#include "gears/desktop/file_dialog.h"


NS_IMPL_ISUPPORTS1(DropTarget, nsIDOMEventListener)


NS_IMETHODIMP DropTarget::HandleEvent(nsIDOMEvent *event) {
  static const nsString drag_enter_as_string(STRING16(L"dragenter"));
  static const nsString drag_over_as_string(STRING16(L"dragover"));
  static const nsString drag_exit_as_string(STRING16(L"dragexit"));
  static const nsString drag_drop_as_string(STRING16(L"dragdrop"));

  nsCOMPtr<nsIDragService> drag_service =
      do_GetService("@mozilla.org/widget/dragservice;1");
  if (!drag_service) { return false; }
  nsCOMPtr<nsIDragSession> drag_session;
  nsresult nr = drag_service->GetCurrentSession(getter_AddRefs(drag_session));
  if (NS_FAILED(nr) || !drag_session.get()) { return false; }
  nr = drag_session->SetDragAction(nsIDragService::DRAGDROP_ACTION_COPY);
  if (NS_FAILED(nr)) { return false; }

  nsString event_type;
  event->GetType(event_type);

  if (on_drop_.get() && event_type.Equals(drag_drop_as_string)) {
    std::string16 error;
    scoped_ptr<JsArray> file_objects(
        module_environment_->js_runner_->NewArray());
    if (!GetDroppedFiles(drag_session.get(), file_objects.get(), &error)) {
      return NS_ERROR_FAILURE;
    }

    // If we've got this far, then the drag-and-dropped data was indeed one or
    // more files, so we will notify our callback.  We also stop the event
    // propagation, to avoid the browser doing the default action, which is to
    // load that file (and navigate away from the current page). I (nigeltao)
    // would have thought that event->PreventDefault() would be the way to do
    // that, but that doesn't seem to work, whilst StopPropagation() does.
    event->StopPropagation();

    const int argc = 1;
    JsParamToSend argv[argc] = {
      { JSPARAM_ARRAY, file_objects.get() }
    };
    module_environment_->js_runner_->InvokeCallback(
        on_drop_.get(), argc, argv, NULL);

  } else {
    JsRootedCallback *callback = NULL;
    if (on_drag_enter_.get() && event_type.Equals(drag_enter_as_string)) {
      callback = on_drag_enter_.get();
    } else if (on_drag_over_.get() && event_type.Equals(drag_over_as_string)) {
      callback = on_drag_over_.get();
    } else if (on_drag_leave_.get() && event_type.Equals(drag_exit_as_string)) {
      callback = on_drag_leave_.get();
    }
    if (callback) {
      module_environment_->js_runner_->InvokeCallback(callback, 0, NULL, NULL);
    }
  }
  return NS_OK;
}


bool DropTarget::GetDroppedFiles(
    nsIDragSession *drag_session,
    JsArray *files_out,
    std::string16 *error_out) {
  // Note to future maintainers: the nsIIOService docs say that it may only be
  // used from the main thread. On the other hand, all we're using it for is
  // converting a string like "file:///blah" into a nsIFileURL, rather than
  // doing any actual I/O, so it's probably safe, regardless.
  nsCOMPtr<nsIIOService> io_service =
      do_GetService("@mozilla.org/network/io-service;1");
  if (!io_service) { return false; }

  PRUint32 num_drop_items;
  nsresult nr = drag_session->GetNumDropItems(&num_drop_items);
  if (NS_FAILED(nr) || num_drop_items <= 0) { return false; }

  std::vector<std::string16> filenames;
  for (int i = 0; i < static_cast<int>(num_drop_items); i++) {
    nsCOMPtr<nsITransferable> transferable =
      do_CreateInstance("@mozilla.org/widget/transferable;1", &nr);
    if (NS_FAILED(nr)) {
      return false;
    }
    transferable->AddDataFlavor("text/x-moz-url");
    drag_session->GetData(transferable, i);

    nsCOMPtr<nsISupports> data;
    PRUint32 data_length;
    nr = transferable->GetTransferData(
        "text/x-moz-url", getter_AddRefs(data), &data_length);
    if (NS_FAILED(nr)) {
      return false;
    }
    nsCOMPtr<nsISupportsString> data_as_xpcom_string = do_QueryInterface(data);
    nsString data_as_string;
    data_as_xpcom_string->GetData(data_as_string);

    nsCString data_as_cstring;
    nr = NS_UTF16ToCString(
        data_as_string, NS_CSTRING_ENCODING_UTF8, data_as_cstring);
    nsCOMPtr<nsIURI> uri;
    nr = io_service->NewURI(data_as_cstring, NULL, NULL, getter_AddRefs(uri));
    if (NS_FAILED(nr)) { return false; }

    nsCOMPtr<nsIFileURL> file_url = do_QueryInterface(uri);
    if (!file_url) { return false; }
    nsCOMPtr<nsIFile> file;
    nr = file_url->GetFile(getter_AddRefs(file));
    if (NS_FAILED(nr)) { return false; }
    nsString filename;
    nr = file->GetPath(filename);
    if (NS_FAILED(nr)) { return false; }
    
    filenames.push_back(std::string16(filename.get()));
  }

  return FileDialog::FilesToJsObjectArray(
      filenames, module_environment_.get(), files_out, error_out);
}


// A DropTarget instance automatically de-registers itself, on page unload.
void DropTarget::HandleEvent(JsEventType event_type) {
  assert(event_type == JSEVENT_UNLOAD);
  DragAndDropRegistry::UnregisterDropTarget(this);
}


#endif  // OFFICIAL_BUILD
