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

// XmlParser that consumes Glint XML and produces a tree of nodes.

#ifndef GLINT_INCLUDE_XML_PARSER_H__
#define GLINT_INCLUDE_XML_PARSER_H__

#ifdef GLINT_ENABLE_XML

#include <string>
#include "glint/include/base_object.h"

namespace glint {

class Color;
class Node;
class Rectangle;
class Transform;

class XmlParser : public BaseObject {
 public:
  XmlParser() : error_line_number_(-1), error_position_(-1) {
  }

  // Static helpers.
  static bool StringToRect(const std::string& string, Rectangle* result);
  static bool StringToInt(const std::string& string, int* result);
  static bool StringToBool(const std::string& string, bool* result);
  static bool StringToColor(const std::string& string, Color* result);
  static bool StringToText(const std::string& string, std::string* result);
  static bool StringToTransform(const std::string& string, Transform* result);

  // Parses complete xml passed in as a string, returns root node
  // of the corresponding tree if the parse was successful.
  // Takes the base_uri to prefix the relative URI of images etc.
  bool Parse(const std::string& xml_text,
             const std::string& base_uri,
             Node** root_node);

  // in case of error, these return the location and error description.
  std::string error_message() {
    return error_message_;
  }

  int error_line_number() {
    return error_line_number_;
  }

  int error_position() {
    return error_position_;
  }

 private:
  std::string error_message_;
  int error_line_number_;
  int error_position_;
  std::string base_uri_;
};

}  // namespace glint

#endif  // GLINT_ENABLE_XML

#endif  // GLINT_INCLUDE_XML_PARSER_H__

