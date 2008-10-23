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

#ifndef GLINT_CROSSPLATFORM_LEXER_H__
#define GLINT_CROSSPLATFORM_LEXER_H__

#include <string>
#include "glint/include/base_object.h"
#include "glint/include/color.h"
#include "glint/include/types.h"

namespace glint {

class Transform;

// Lexer - performs simple lexical analysis of the string and converts it
// into a stream of tokens based on a fixed set of patterns.
// Only operates on ASCII subset of Utf8 since it is only used to parse
// specific 'languages' that describe Glint properties in XML notation,
// like "matrix(1 0 0 1 153 371)" or "#FF00FF".
class Lexer : public BaseObject {
 public:
  explicit Lexer(const std::string& text);

  enum Tokens {
    IDENTIFIER,  // starts with a letter
    NUMBER,
    COMMA,
    DOT,
    MINUS,
    OPEN_PAREN,
    CLOSE_PAREN,
    COLOR_HEX,
    EOL,
    STAR,
    UNKNOWN,
  };

  // Parses the next 'thing' from the input string, return 'false' if can't.
  bool ParseInteger(int* value);
  bool ParseReal32(real32* value);
  bool ParseIdentifier(std::string* value);
  bool ParseColor(Color* value);
  bool ParseRotate(Transform* result);
  bool ParseScale(Transform* result);
  bool ParseTranslate(Transform* result);
  bool ParseMatrix(Transform* result);
  bool ParseSingleTransform(Transform* result);

  // Check and skip the specified token - used to skip things like ';' etc.
  bool CheckToken(Tokens token);
  Tokens token() { return token_; }
  bool IsEmpty() { return token_ == EOL; }
  void MoveNext();

 private:
  inline void SkipSpace() {
    while (position_ < text_.length() &&
           IsSpace(text_[position_])) {
      ++position_;
    }
  }

  inline static bool IsSpace(char ch) {
    return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
  }

  inline static bool IsAlpha(char ch) {
    return (ch >= 'A' && ch <= 'Z') ||
           (ch >= 'a' && ch <= 'z') ||
           ch == '_';
  }

  inline static bool IsDigit(char ch) {
    return ch >= '0' && ch <= '9';
  }

  inline static int HexDigit(char ch) {
    if (ch >= 'A' && ch <= 'F') {
      return ch - 'A' + 10;
    } else if (ch >= 'a' && ch <= 'f') {
      return ch - 'a' + 10;
    } else if (IsDigit(ch)) {
      return ch - '0';
    }
    return -1;
  }

  bool GetNumber(int* value);
  void ScanIdentifier();
  void ScanNumber();
  void ScanColor();
  void ScanSimpleToken();

  std::string text_;
  Tokens token_;
  std::string identifier_;
  Color color_;
  int number_;
  size_t position_;  // first position after the current token
  DISALLOW_EVIL_CONSTRUCTORS(Lexer);
};

}  // namespace glint

#endif  // GLINT_CROSSPLATFORM_LEXER_H__


