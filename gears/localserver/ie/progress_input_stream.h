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

#ifndef GEARS_LOCALSERVER_IE_PROGRESS_INPUT_STREAM_H__
#define GEARS_LOCALSERVER_IE_PROGRESS_INPUT_STREAM_H__

#include "gears/base/common/scoped_refptr.h"
#include "gears/base/ie/atl_headers.h"

class IEHttpRequest;

class ProgressInputStream : public CComObjectRootEx<CComMultiThreadModel>,
                            public IStream {
 public:
  BEGIN_COM_MAP(ProgressInputStream)
    COM_INTERFACE_ENTRY(ISequentialStream)
    COM_INTERFACE_ENTRY(IStream)
  END_COM_MAP()

  ProgressInputStream();
  virtual ~ProgressInputStream();
  void Initialize(IEHttpRequest * request, IStream *input_stream);
  void DetachRequest();

  // ISequentialStream
  STDMETHOD(Read)(void* pv, ULONG cb, ULONG* read);
  STDMETHOD(Write)(const void* pv, ULONG cb, ULONG* written);

  // IStream
  STDMETHOD(Seek)(LARGE_INTEGER move, ULONG origin,
                  ULARGE_INTEGER* new_position);
  STDMETHOD(SetSize)(ULARGE_INTEGER libNewSize);
  STDMETHOD(CopyTo)(IStream* stream, ULARGE_INTEGER cb, ULARGE_INTEGER* read,
                    ULARGE_INTEGER* written);
  STDMETHOD(Commit)(ULONG grfCommitFlags);
  STDMETHOD(Revert)();
  STDMETHOD(LockRegion)(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb,
                        ULONG dwLockType);
  STDMETHOD(UnlockRegion)(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb,
                          ULONG dwLockType);
  STDMETHOD(Stat)(STATSTG *pstatstg, ULONG grfStatFlag);
  STDMETHOD(Clone)(IStream **ppstm);

 private:
  IEHttpRequest *request_;
  CComPtr<IStream> input_stream_;
};

#endif  // GEARS_LOCALSERVER_IE_PROGRESS_INPUT_STREAM_H__
