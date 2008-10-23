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

#ifndef GEARS_BLOB_BLOB_STREAM_IE_H___
#define GEARS_BLOB_BLOB_STREAM_IE_H___

#include <assert.h>
#include <atlcom.h>
#include "gears/base/common/scoped_refptr.h"
#include "gears/blob/blob_interface.h"

// BlobStream implements IStream in terms of BlobInterface.

class BlobStream
  : public CComObjectRootEx<CComMultiThreadModel>,
    public IStream {
 public:
  BEGIN_COM_MAP(BlobStream)
    COM_INTERFACE_ENTRY(ISequentialStream)
    COM_INTERFACE_ENTRY(IStream)
  END_COM_MAP()

  BlobStream() : offset_(0) {
  }

  virtual ~BlobStream() {
  }

  void Initialize(BlobInterface* blob, int64 seek) {
    blob_ = blob;
    offset_ = seek;
  }

  //
  //@name ISequentialStream
  //@{
  STDMETHOD(Read)(void* pv, ULONG cb, ULONG* read) {
    if (!pv)
      return E_POINTER;
    
    int64 length = blob_->Length();
    if (offset_ > length) {
      if (read)
        *read = 0;
      return S_FALSE;
    }

    int64 available = length - offset_;
    if (available > cb)
      available = cb;

    int64 result = blob_->Read(static_cast<uint8*>(pv), offset_, available);
    if (result < 0) {
      if (read)
        *read = 0;
      return E_FAIL;
    }

    offset_ += result;
    if (read) {
      // Since this is limited by cb, this cast is safe.
      *read = static_cast<ULONG>(result);
    }
    return (result > 0) ? S_OK : S_FALSE;
  }


  STDMETHOD(Write)(const void* pv, ULONG cb, ULONG* written) {
    return E_NOTIMPL;
  }
  //@}

  //
  //@name IStream
  //@{
  STDMETHOD(Seek)(LARGE_INTEGER move, ULONG origin,
                  ULARGE_INTEGER* new_position) {
    int64 seek = -1;
    switch (origin) {
      case STREAM_SEEK_SET:
        seek = move.QuadPart;
        break;

      case STREAM_SEEK_CUR:
        seek = offset_ + move.QuadPart;
        break;

      case STREAM_SEEK_END:
        seek = blob_->Length() + move.QuadPart;
        break;

      default:
        return STG_E_INVALIDFUNCTION;
    }
    // MSDN: "It is not, however, an error to seek past the end of the stream."
    if (seek < 0) {
      return STG_E_INVALIDFUNCTION;
    }

    offset_ = seek;
    if (new_position) {
      new_position->QuadPart = offset_;
    }
    return S_OK;
  }

  STDMETHOD(SetSize)(ULARGE_INTEGER libNewSize) {
    return E_NOTIMPL;
  }

  STDMETHOD(CopyTo)(IStream* stream, ULARGE_INTEGER cb, ULARGE_INTEGER* read,
                    ULARGE_INTEGER* written) {
    if (!stream)
      return E_POINTER;

    HRESULT hr = S_OK;
    uint64 did_read = 0;

    // Copy 64K at a time.
    uint8 buffer[64 * 1024];
    while ((S_OK == hr) && (did_read < cb.QuadPart)) {
      int64 result = blob_->Read(buffer, offset_ + did_read, sizeof(buffer));
      if (result <= 0) {
        if (result < 0) {
          hr = E_FAIL;
        }
        break;
      }

      ULONG temp, will_write = static_cast<ULONG>(result);
      for (ULONG did_write = 0; did_write < will_write; did_write += temp) {
        temp = 0;
        hr = stream->Write(buffer + did_write, will_write - did_write, &temp);
        if (0 == temp) {
          hr = STG_E_MEDIUMFULL;
        }
        if (S_OK != hr) {
          break;
        }
      }

      did_read += result;
    }

    // MSDN: "If IStream::CopyTo returns an error, you cannot assume that the
    // seek pointers are valid for either the source or destination.
    // Additionally, the values of pcbRead and pcbWritten are not meaningful
    // even though they are returned."
    offset_ += did_read;
    if (read)
      read->QuadPart = did_read;
    if (written)
      written->QuadPart = did_read;
    return hr;
  }

  STDMETHOD(Commit)(ULONG grfCommitFlags) {
    return E_NOTIMPL;
  }

  STDMETHOD(Revert)() {
    return E_NOTIMPL;
  }

  STDMETHOD(LockRegion)(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb,
                        ULONG dwLockType) {
    return STG_E_INVALIDFUNCTION;
  }

  STDMETHOD(UnlockRegion)(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb,
                          ULONG dwLockType) {
    return STG_E_INVALIDFUNCTION;
  }

  STDMETHOD(Stat)(STATSTG *pstatstg, ULONG grfStatFlag) {
    if (pstatstg == NULL)
      return E_POINTER;

    memset(pstatstg, 0, sizeof(STATSTG));
    if (0 == (grfStatFlag & STATFLAG_NONAME)) {
      const wchar_t kStreamBuffer[] = L"BlobStream";
      pstatstg->pwcsName =
          static_cast<wchar_t*>(::CoTaskMemAlloc(sizeof(kStreamBuffer)));
      wcscpy(pstatstg->pwcsName, kStreamBuffer);
    }
    pstatstg->type = STGTY_STREAM;
    pstatstg->cbSize.QuadPart = blob_->Length();
    return S_OK;
  }

  STDMETHOD(Clone)(IStream **ppstm) {
    CComObject<BlobStream> *stream = NULL;
    HRESULT hr = CComObject<BlobStream>::CreateInstance(&stream);
    if (FAILED(hr))
      return hr;
    CComQIPtr<IStream> istream(stream->GetUnknown());
    stream->Initialize(blob_.get(), offset_);
    *ppstm = istream.Detach();
    return S_OK;
  }
  //@}

 private:
  scoped_refptr<BlobInterface> blob_;
  int64 offset_;
};

#endif  // GEARS_BLOB_BLOB_STREAM_IE_H___
