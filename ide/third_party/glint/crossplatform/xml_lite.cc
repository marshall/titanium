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

#ifdef GLINT_ENABLE_XML

#include <stdarg.h>
#include <stdlib.h>
#include "glint/crossplatform/xml_lite.h"
#include "expat.h"
#include "glint/include/allocator.h"
#include "glint/include/platform.h"

namespace glint {

XML_Memory_Handling_Suite suite = {
  Allocator::Allocate,
  Allocator::Reallocate,
  Allocator::Free
};

static void StartElementHandler(void* user_data,
                                const XML_Char* name,
                                const XML_Char** attributes) {
  XmlLite* parser = static_cast<XmlLite*>(user_data);
  parser->OnElementStart(name, attributes);
}

static void EndElementHandler(void *user_data,
                              const XML_Char *name) {
  XmlLite* parser = static_cast<XmlLite*>(user_data);
  parser->OnElementEnd(name);
}

static void CharacterDataHandler(void *user_data,
                                 const XML_Char *s,
                                 int len) {
  XmlLite* parser = static_cast<XmlLite*>(user_data);
  parser->OnElementText(s, len);
}

XmlLite::XmlLite() : error_(false) {
  XML_Parser expat = XML_ParserCreate_MM("UTF-8", &suite, "#");
  expat_ = reinterpret_cast<ExpatXMLParser*>(expat);
  XML_SetUserData(expat, this);
  XML_SetElementHandler(expat, StartElementHandler, EndElementHandler);
  XML_SetCharacterDataHandler(expat, CharacterDataHandler);
}

XmlLite::~XmlLite() {
  XML_Parser expat = reinterpret_cast<XML_Parser>(expat_);
  XML_ParserFree(expat);
}

bool XmlLite::Parse(const std::string &xml_text) {
  if (error_)
    return false;

  XML_Parser expat = reinterpret_cast<XML_Parser>(expat_);
  XML_Status status = XML_Parse(expat,
                                xml_text.c_str(),
                                static_cast<int>(xml_text.length()),
                                true);
  if (status != XML_STATUS_OK) {
    error_ = true;
    platform()->Trace("XML parsing failed, reason: %s\n",
                      GetErrorMessage().c_str());
  }

  return !error_;
}

void XmlLite::Stop(const std::string& reason) {
  XML_Parser expat = reinterpret_cast<XML_Parser>(expat_);
  XML_StopParser(expat, false);
  stop_reason_ = reason;
}

std::string XmlLite::GetErrorMessage() {
  if (!error_)
    return std::string("");

  XML_Parser expat = reinterpret_cast<XML_Parser>(expat_);
  XML_Error code = XML_GetErrorCode(expat);

  std::string message(XML_ErrorString(code));
  if (!stop_reason_.empty()) {
    message.append(" : ");
    message.append(stop_reason_);
  }
  return message;
}

int XmlLite::GetErrorLine() {
  XML_Parser expat = reinterpret_cast<XML_Parser>(expat_);
  return XML_GetCurrentLineNumber(expat);
}

int XmlLite::GetErrorPosition() {
  XML_Parser expat = reinterpret_cast<XML_Parser>(expat_);
  return XML_GetCurrentColumnNumber(expat);
}

}  // namespace glint

#endif  // GLINT_ENABLE_XML
