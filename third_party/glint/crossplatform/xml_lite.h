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

// XmlLite is a C++ wrapper of open-source Expat XML parser.

#ifndef GLINT_CROSSPLATFORM_XML_LITE_H__
#define GLINT_CROSSPLATFORM_XML_LITE_H__

#ifdef GLINT_ENABLE_XML

#include <string>
#include "glint/include/base_object.h"

namespace glint {

class ExpatXMLParser;

// When parser encounters an error, it stops and returns 'false' from the
// Parse method. Error information can be retrieved via
class XmlLite : public BaseObject {
 public:
  XmlLite();
  ~XmlLite();

  // Parses XML from the string, calls virtual callbacks in the process.
  bool Parse(const std::string &xml_text);

  // Called from callbacks to stop parsing (because of error)
  void Stop(const std::string& reason);

  // in case of error, these return the location and error description.
  std::string GetErrorMessage();
  int GetErrorLine();
  int GetErrorPosition();

  // Invoked on open element tag. attributes is an array of
  // attribute-value pairs.
  virtual void OnElementStart(const char* name, const char** attributes) {}

  // Invoked on closing tag
  virtual void OnElementEnd(const char* name) {}

  // Invoked when text is encountered between opening and closing tags.
  // Note: text is not null-terminated.
  virtual void OnElementText(const char* text, int len) {}

 private:
  ExpatXMLParser* expat_;
  bool error_;
  std::string stop_reason_;
};

}  // namespace glint

#endif  // GLINT_ENABLE_XML

#endif  // GLINT_CROSSPLATFORM_XML_LITE_H__


