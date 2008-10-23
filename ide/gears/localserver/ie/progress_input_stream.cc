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

#include "gears/localserver/common/progress_event.h"
#include "gears/localserver/ie/http_request_ie.h"
#include "gears/localserver/ie/progress_input_stream.h"

//------------------------------------------------------------------------------
//ProgressInputStream implementation
//------------------------------------------------------------------------------

ProgressInputStream::ProgressInputStream() : request_(NULL) {
}

ProgressInputStream::~ProgressInputStream() {
  // When the yahoo toolbar is installed, the stream gets released one too
  // many times. We arrange to detect when this is occuring and avoid the
  // extra release in that case.
  if (request_) {
    // The request still has a reference to this object, yet it's being deleted.
    // Drop our reference w/o releasing it.
    request_->post_data_stream_.Detach();
  }
}

void ProgressInputStream::Initialize(IEHttpRequest * request,
                                     IStream *input_stream) {
  request_ = request;
  input_stream_ = input_stream;
}

void ProgressInputStream::DetachRequest() {
  request_ = NULL;
}

//------------------------------------------------------------------------------
// ISequentialStream implementation
//------------------------------------------------------------------------------

STDMETHODIMP ProgressInputStream::Read(void* pv, ULONG cb, ULONG* read) {
  if (!request_) return E_FAIL;
  HRESULT result = input_stream_->Read(pv, cb, read);
  if (SUCCEEDED(result) && *read > 0) {
    static const LARGE_INTEGER offset = { 0 };
    ULARGE_INTEGER position;
    input_stream_->Seek(offset, STREAM_SEEK_CUR, &position);
    STATSTG statstg;
    input_stream_->Stat(&statstg, STATFLAG_NONAME);
    ProgressEvent::Update(request_, request_,
                          position.QuadPart, statstg.cbSize.QuadPart);
  }
  return result;
}

STDMETHODIMP ProgressInputStream::Write(const void* pv,
                                        ULONG cb, ULONG* written) {
  return input_stream_->Write(pv, cb, written);
}

//------------------------------------------------------------------------------
// IStream implementation
//------------------------------------------------------------------------------

STDMETHODIMP ProgressInputStream::Seek(LARGE_INTEGER move, ULONG origin,
                                       ULARGE_INTEGER* new_position) {
  return input_stream_->Seek(move, origin, new_position);
}

STDMETHODIMP ProgressInputStream::SetSize(ULARGE_INTEGER libNewSize) {
  return input_stream_->SetSize(libNewSize);
}

STDMETHODIMP ProgressInputStream::CopyTo(IStream* stream,
                                         ULARGE_INTEGER cb,
                                         ULARGE_INTEGER* read,
                                         ULARGE_INTEGER* written) {
  return input_stream_->CopyTo(stream, cb, read, written);
}

STDMETHODIMP ProgressInputStream::Commit(ULONG grfCommitFlags) {
  return input_stream_->Commit(grfCommitFlags);
}

STDMETHODIMP ProgressInputStream::Revert() {
  return input_stream_->Revert();
}

STDMETHODIMP ProgressInputStream::LockRegion(ULARGE_INTEGER libOffset,
                                             ULARGE_INTEGER cb,
                                             ULONG dwLockType) {
  return input_stream_->LockRegion(libOffset, cb, dwLockType);
}

STDMETHODIMP ProgressInputStream::UnlockRegion(ULARGE_INTEGER libOffset,
                                               ULARGE_INTEGER cb,
                                               ULONG dwLockType) {
  return input_stream_->UnlockRegion(libOffset, cb, dwLockType);
}

STDMETHODIMP ProgressInputStream::Stat(STATSTG *pstatstg, ULONG grfStatFlag) {
  return input_stream_->Stat(pstatstg, grfStatFlag);
}

STDMETHODIMP ProgressInputStream::Clone(IStream **ppstm) {
  if (!request_) return E_FAIL;
  CComObject<ProgressInputStream> *stream = NULL;
  HRESULT result = CComObject<ProgressInputStream>::CreateInstance(&stream);
  if (FAILED(result)) {
    return result;
  }
  CComQIPtr<IStream> istream(stream->GetUnknown());
  IStream *input_stream_clone;
  result = input_stream_->Clone(&input_stream_clone);
  if (FAILED(result)) {
    return result;
  }
  stream->Initialize(request_, input_stream_clone);
  *ppstm = istream.Detach();
  return S_OK;
}
