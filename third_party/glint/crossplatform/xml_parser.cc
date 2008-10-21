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

#include <string>
#include "glint/include/xml_parser.h"
#include "glint/crossplatform/core_util.h"
#include "glint/crossplatform/lexer.h"
#include "glint/crossplatform/xml_lite.h"
#include "glint/include/array.h"
#include "glint/include/color.h"
#include "glint/include/node.h"
#include "glint/include/rectangle.h"
#include "glint/include/transform.h"
#include "glint/include/type_system.h"

namespace glint {

static const char* kProtocolPrefixes[] = {
  "file://", "res://", "http://", "https://" };

static const char kPathDelimiter = '/';

bool XmlParser::StringToRect(const std::string& string, Rectangle* result) {
  ASSERT(result);
  Lexer lex(string);
  int left, top, right, bottom;
  if (!lex.ParseInteger(&left) ||
      !lex.ParseInteger(&top) ||
      !lex.ParseInteger(&right) ||
      !lex.ParseInteger(&bottom)) {
    return false;
  }
  result->Set(left, top, right, bottom);
  return true;
}

bool XmlParser::StringToInt(const std::string& string, int* result) {
  ASSERT(result);
  Lexer lex(string);
  return lex.ParseInteger(result) &&
         lex.IsEmpty();
}

bool XmlParser::StringToBool(const std::string& string, bool* result) {
  ASSERT(result);
  Lexer lex(string);
  std::string word;
  if (!lex.ParseIdentifier(&word))
    return false;
  if (word == "true") {
    *result = true;
    return lex.IsEmpty();
  } else if (word == "false") {
    *result = false;
    return lex.IsEmpty();
  }
  return false;
}

bool XmlParser::StringToColor(const std::string& string, Color* result) {
  ASSERT(result);
  Lexer lex(string);
  std::string color_name;
  if (lex.ParseIdentifier(&color_name)) {
    return Color::FromName(color_name, result) &&
           lex.IsEmpty();
  }
  // if identifier was not there, try to get color_ref (#AARRGGBB or #RRGGBB)
  return lex.ParseColor(result) &&
         lex.IsEmpty();
}

// matrix(00 01 10 11 dx dy) ...
// translate(dx dy) ...
// scale(sx sy) ...
// rotate(deg) ...
bool XmlParser::StringToTransform(const std::string& string,
                                  Transform* result) {
  ASSERT(result);
  Lexer lex(string);
  Transform transform;
  result->Set(transform);
  if (!lex.ParseSingleTransform(result))
    return false;
  while (lex.ParseSingleTransform(result)) {}
  return lex.IsEmpty();
}

// \n \r \t \" \\ get converted into corresponding characters.
bool XmlParser::StringToText(const std::string& string, std::string* result) {
  ASSERT(result);
  result->resize(string.length());  // trailing 0 is accounted for by STL
  char* buffer = &(result->at(0));
  int length = string.length();
  int result_length = 0;
  for (int i = 0; i < length; ++i) {
    char next_character = string[i];
    if ((next_character == '\\') && (i < length - 1)) {
      ++i;
      switch (string[i]) {
        case '\\':
          next_character = '\\';
          break;
        case 'n':
          next_character = '\n';
          break;
        case 'r':
          next_character = '\r';
          break;
        case 't':
          next_character = '\t';
          break;
        case '"':
          next_character = '"';
          break;
        default:
          --i;  // Not recognized sequence, let it in.
          break;
      }
    }
    *buffer++ = next_character;
    ++result_length;
  }
  result->resize(result_length);
  return true;
}

// Holds an object under construction. When XML encounters new object
// (normally as an opening tag), a new instance of XmlStackItem is pushed
// on XML stack so the we can always refer to the object at the top as
// "current object to set properties on".
struct XmlStackItem : BaseObject {
  XmlStackItem() : object(NULL), type(NULL) {}
  ~XmlStackItem() {
    if (object)
      delete object;
  }
  BaseObject* object;
  TypeDescriptor* type;
};

class ParserHelper : public XmlLite {
 public:
  explicit ParserHelper(const std::string& base_uri)
    : root_object_(NULL),
      base_uri_(base_uri) {
    if (!base_uri_.empty() &&
        base_uri_[base_uri_.length() - 1] != kPathDelimiter) {
      base_uri_.append(1, kPathDelimiter);
    }
  }

  virtual void OnElementStart(const char* name, const char** attributes) {
    TypeDescriptor* type = TypeDescriptor::FromName(name);
    if (!type) {
      Stop(std::string("Unknown object: '") + name + "'.");
      return;
    }

    XmlStackItem* item = new XmlStackItem();
    item->type = type;
    item->object = type->CreateInstance();
    xml_stack_.Add(item);

    while (attributes && attributes[0] && attributes[1]) {
      PropertyDescriptor* property = NULL;
      TypeDescriptor* base_type = type;
      while (base_type) {
        property = base_type->FindProperty(attributes[0]);
        if (property)
          break;
        base_type = base_type->base_type();
      }

      if (!property) {
        Stop(std::string("Unknown property '") +
             std::string(attributes[0]) +
             std::string("' on object '") +
             type->name() +
             std::string("'."));
        return;
      }

      // All properties that are set from attributes are going to be parsed
      // from strings. Prefix them with base_uri if needed.
      std::string value(attributes[1]);
      if (property->value_is_uri() && value.length() > 2) {
        bool absolute_uri = false;
        if (value[1] == ':') {
          absolute_uri = true;
        } else {
          for (size_t i = 0;
               i < sizeof(kProtocolPrefixes) / sizeof(kProtocolPrefixes[0]);
               ++i) {
            std::string::size_type length = strlen(kProtocolPrefixes[i]);
            if (value.compare(0, length, kProtocolPrefixes[i], length) == 0) {
              absolute_uri = true;
              break;
            }
          }
        }
        if (!absolute_uri) {
          if (!value.empty() && value[0] == kPathDelimiter) {
            value = base_uri_ + value.substr(1);
          } else {
            value = base_uri_ + value;
          }
        }
      }
      SetPropertyResult result = property->SetPropertyFromString(
          item->object, value);

      // TODO(dimich): add more error messaging here, to use entire enum range.
      if (result != PROPERTY_OK) {
        Stop(std::string("Can not set property '") +
             std::string(attributes[0]) +
             std::string("' on object '") +
             type->name() +
             std::string("' to value '") +
             value +
             std::string("'."));
        return;
      }
      attributes += 2;
    }
  }

  // Invoked on closing tag
  virtual void OnElementEnd(const char* name) {
    if (xml_stack_.length() == 0) {
      Stop(std::string("Unexpected closing tag for object '") +
           std::string(name) +
           std::string("' at the end of file."));
      return;
    }
    XmlStackItem* item = xml_stack_[xml_stack_.length() - 1];
    if (item->type->name() != name) {
      Stop(std::string("Mismatched closing tag for object '") +
           std::string(name) +
           std::string("', expected '") +
           item->type->name() +
           std::string("'."));
      return;
    }

    BaseObject* built_object = item->object;
    TypeDescriptor* built_type = item->type;
    item->object = NULL;

    xml_stack_.RemoveAt(xml_stack_.length() - 1);
    if (xml_stack_.length() == 0) {
      // Outermost element closed. Set result and return.
      root_object_ = built_object;
    } else {
      // Nested object was built, set it as default property on the parent one.
      item = xml_stack_[xml_stack_.length() - 1];
      PropertyDescriptor* default_property = item->type->default_property();
      if (!default_property) {
        Stop(std::string("Object '") +
             item->type->name() +
             std::string("' can not have nested objects."));
        return;
      }

      TypeDescriptor* property_type = default_property->type();

      if (!built_type->IsOfType(property_type)) {
        Stop(std::string("Object '") +
             item->type->name() +
             std::string("' can not parent '") +
             built_type->name() +
             std::string("'."));
        return;
      }

      SetPropertyResult result = default_property->SetProperty(
          item->object, built_object);

      // TODO(dimich): add more error messaging here, to use entire enum range.
      if (result != PROPERTY_OK) {
        Stop(std::string("Can not set property '") +
             default_property->type()->name() +
             std::string("' on object '") +
             item->type->name() +
             std::string("'."));
        return;
      }
    }
  }

  // Invoked when text is encountered between opening and closing tags.
  // Note: text is not null-terminated.
  virtual void OnElementText(const char* text, int len) {
    std::string string(text, len);
    // TODO(dimich): add code to process this.
  }

  BaseObject* root_object() {
    return root_object_;
  }

 private:
  Array<XmlStackItem> xml_stack_;
  BaseObject* root_object_;
  std::string text_;
  std::string base_uri_;
};


bool XmlParser::Parse(const std::string& xml_text,
                      const std::string& base_uri,
                      Node** root_node) {
  if (!root_node)
    return false;
  ParserHelper helper(base_uri);

  error_message_.clear();
  error_line_number_ = -1;
  error_position_ = -1;
  base_uri_ = base_uri;

  bool result = helper.Parse(xml_text);
  if (result) {
    *root_node = static_cast<Node*>(helper.root_object());
  } else {
    error_message_ = helper.GetErrorMessage();
    error_line_number_ = helper.GetErrorLine();
    error_position_ = helper.GetErrorPosition();
  }
  return result;
}

}  // namespace glint

#endif  // GLINT_ENABLE_XML
