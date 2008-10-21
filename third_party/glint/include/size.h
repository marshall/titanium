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

// Simple Size struct with some useful methods.

#ifndef GLINT_INCLUDE_SIZE_H__
#define GLINT_INCLUDE_SIZE_H__

namespace glint {

struct Size {
  Size() : width(0), height(0) {}

  Size(int w, int h) {
    this->width = w;
    this->height = h;
  }

  bool operator==(const Size &s) {
    return (s.width == width && s.height == height);
  }

  bool operator!=(const Size &s) {
    return !(*this == s);
  }

  bool IsZero() const {
    return width == 0 && height == 0;
  }

  static Size HugeSize() {
    return Size(static_cast<int>(0x70000000),
                static_cast<int>(0x70000000));
  }

  void Max(const Size &s);
  void Min(const Size &s);

  int width;
  int height;
};


}  // namespace glint

#endif  // GLINT_INCLUDE_SIZE_H__
