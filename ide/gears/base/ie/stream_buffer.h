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

#ifndef GEARS_BASE_IE_STREAM_BUFFER_H__
#define GEARS_BASE_IE_STREAM_BUFFER_H__

#include <assert.h>
#include <atlcom.h>
#include <algorithm>
#include <string>
#include "third_party/scoped_ptr/scoped_ptr.h"

/**
* A minimalistic implementation of IStream on a buffer that is owned by 
* the user of the class.  It is the responsibility of the user of this class
* to make sure that the data remains valid as long as the StreamBuffer instance
* is being used.
*/
class StreamBuffer
  : public CComObjectRootEx<CComMultiThreadModel>,
    public IStream {
 public:
  StreamBuffer() :
    buffer_(NULL),
    pos_(NULL),
    end_(NULL) {
  }

BEGIN_COM_MAP(StreamBuffer)
  COM_INTERFACE_ENTRY(ISequentialStream)
  COM_INTERFACE_ENTRY(IStream)
END_COM_MAP()

  //
  //@name ISequentialStream
  //@{
  STDMETHOD(Read)(void* pv, ULONG cb, ULONG* read) {
    assert(pv != NULL);
    assert(cb != 0);
    // read may be NULL

    assert(buffer_ != NULL);
    assert(pos_ != NULL);
    assert(end_ != NULL);

    HRESULT hr = S_OK;

    if (pos_ < end_) {
      ULONG can_read = std::min<ULONG>(cb, static_cast<ULONG>(end_ - pos_));

      memcpy(pv, pos_, can_read);
      pos_ += can_read;

      if (read)
        *read = can_read;
    } else if (read) {
      *read = 0;
      hr = S_FALSE;
    }

    return hr;
  }

  STDMETHOD(Write)(const void* pv, ULONG cb, ULONG* written) {
    assert(false);
    assert(buffer_ != NULL);
    assert(pos_ != NULL);
    assert(end_ != NULL);
    return E_NOTIMPL;
  }
  //@}

  //
  //@name IStream
  //@{
  STDMETHOD(Seek)(LARGE_INTEGER move, ULONG origin,
                  ULARGE_INTEGER* new_position) {
    // new_position may be NULL
    assert(buffer_ != NULL);
    assert(pos_ != NULL);
    assert(end_ != NULL);

    const char* pos = NULL;
    switch (origin) {
      case STREAM_SEEK_SET:
        pos = buffer_ + move.QuadPart;
        break;

      case STREAM_SEEK_CUR:
        pos = pos_ + move.QuadPart;
        break;

      case STREAM_SEEK_END:
        pos = end_ + move.QuadPart;
        break;
    }

    if (pos == NULL || pos > end_ || pos < buffer_)
      return E_INVALIDARG;

    pos_ = pos;

    if (new_position)
      new_position->QuadPart = (pos_ - buffer_);

    return S_OK;
  }

  STDMETHOD(SetSize)(ULARGE_INTEGER libNewSize) {
    assert(buffer_ != NULL);
    assert(pos_ != NULL);
    assert(end_ != NULL);
    assert(false);
    return E_NOTIMPL;
  }

  STDMETHOD(CopyTo)(IStream* stream, ULARGE_INTEGER cb, ULARGE_INTEGER* read,
                    ULARGE_INTEGER* written) {
    assert(stream != NULL);
    assert(buffer_ != NULL);
    assert(pos_ != NULL);
    assert(end_ != NULL);
    if (!stream)
      return E_POINTER;

    if ((end_ - pos_) < cb.QuadPart)
      cb.QuadPart = (end_ - pos_);

    DWORD bytes_written = 0;
    HRESULT hr = stream->Write(pos_, cb.LowPart, &bytes_written);

    if (SUCCEEDED(hr)) {
      pos_ += bytes_written;

      if (written)
        written->QuadPart = bytes_written;

      if (read)
        read->QuadPart = bytes_written;
    }

    return hr;
  }

  STDMETHOD(Commit)(ULONG grfCommitFlags) {
    assert(false);
    assert(buffer_ != NULL);
    assert(pos_ != NULL);
    assert(end_ != NULL);
    return E_NOTIMPL;
  }
  STDMETHOD(Revert)() {
    assert(false);
    assert(buffer_ != NULL);
    assert(pos_ != NULL);
    assert(end_ != NULL);
    return E_NOTIMPL;
  }
  STDMETHOD(LockRegion)(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb,
                        ULONG dwLockType) {
    assert(false);
    assert(buffer_ != NULL);
    assert(pos_ != NULL);
    assert(end_ != NULL);
    return E_NOTIMPL;
  }
  STDMETHOD(UnlockRegion)(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb,
                          ULONG dwLockType) {
    assert(false);
    assert(buffer_ != NULL);
    assert(pos_ != NULL);
    assert(end_ != NULL);
    return E_NOTIMPL;
  }

  STDMETHOD(Stat)(STATSTG *pstatstg, ULONG grfStatFlag) {
    assert(pstatstg != NULL);
    assert(buffer_ != NULL);
    assert(pos_ != NULL);
    assert(end_ != NULL);
    if (pstatstg == NULL)
      return E_POINTER;

    memset(pstatstg, 0, sizeof(STATSTG));
    if (0 == (grfStatFlag & STATFLAG_NONAME)) {
      const wchar_t kStreamBuffer[] = L"StreamBuffer";
      pstatstg->pwcsName =
          static_cast<wchar_t*>(::CoTaskMemAlloc(sizeof(kStreamBuffer)));
      wcscpy(pstatstg->pwcsName, kStreamBuffer);
    }
    pstatstg->type = STGTY_STREAM;
    pstatstg->cbSize.QuadPart = (end_ - buffer_);
    return S_OK;
  }

  STDMETHOD(Clone)(IStream **ppstm) {
    assert(false);
    assert(buffer_ != NULL);
    assert(pos_ != NULL);
    assert(end_ != NULL);
    return E_NOTIMPL;
  }
  //@}

#ifdef DEBUG
  virtual ~StreamBuffer() {
  }
#endif

 public:
  /**
  * Routine to initialize the internal buffers.
  * @param buffer A pointer to the data buffer. Must not be NULL.
  * @param size the size of the buffer.
  */
  void Initialize(const char* buffer, int size) {
    assert(buffer != NULL);
    assert(buffer_ == NULL);
    assert(pos_ == NULL);
    assert(end_ == NULL);

    buffer_ = buffer;
    pos_ = buffer;
    end_ = (buffer + size);
  }

 protected:
  const char* buffer_;
  const char* pos_;
  const char* end_;
};

#endif  // GEARS_BASE_IE_STREAM_BUFFER_H__
