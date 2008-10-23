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

#include <string>
#include "glint/crossplatform/lexer.h"
#include "glint/crossplatform/core_util.h"
#include "glint/include/point.h"
#include "glint/include/transform.h"

namespace glint {

// TODO(dimich): Add unittest for this class.

Lexer::Lexer(const std::string& text)
  : text_(text),
    token_(UNKNOWN),
    position_(0) {
  MoveNext();
}

bool Lexer::ParseIdentifier(std::string* value) {
  ASSERT(value);
  if (token_ != IDENTIFIER)
    return false;

  *value = identifier_;
  MoveNext();
  return true;
}

bool Lexer::GetNumber(int* value) {
  ASSERT(value);
  if (token_ != NUMBER)
    return false;

  *value = number_;
  MoveNext();
  return true;
}

bool Lexer::ParseColor(Color* value) {
  ASSERT(value);
  if (token_ != COLOR_HEX)
    return false;

  *value = color_;
  MoveNext();
  return true;
}

bool Lexer::CheckToken(Tokens token) {
  if (token_ != token)
    return false;

  MoveNext();
  return true;
}

void Lexer::MoveNext() {
  SkipSpace();

  if (position_ >= text_.length()) {
    token_ = EOL;
    return;
  }

  // Peek the next character.
  char character = text_[position_];

  if (IsAlpha(character)) {
    ScanIdentifier();
  } else if (IsDigit(character)) {
    ScanNumber();
  } else if (character == '#') {
    ScanColor();
  } else {
    ScanSimpleToken();
  }
}

void Lexer::ScanIdentifier() {
  token_ = IDENTIFIER;
  identifier_.clear();
  for (char ch = text_[position_];
       position_ < text_.length() && (IsAlpha(ch) || IsDigit(ch));
       ch = text_[position_]) {
    identifier_.append(1, ch);
    ++position_;
  }
}

void Lexer::ScanNumber() {
  token_ = NUMBER;
  number_ = 0;
  for (char ch = text_[position_];
       position_ < text_.length() && IsDigit(ch);
       ch = text_[position_]) {
    number_ = number_ * 10 + (ch - '0');
    ++position_;
  }
}

// position_ points to '#'
void Lexer::ScanColor() {
  int count = 0;
  uint32 argb = 0;

  size_t pos = position_ + 1;
  while (pos < text_.length()) {
    char ch = text_[pos++];
    int hex = HexDigit(ch);
    if (hex == -1)
      break;
    argb = (argb << 4) + hex;
    ++count;
  }
  if (count == 6 || count == 8) {
    token_ = COLOR_HEX;
    position_ = pos;
    color_ = Color(argb);
    // If alpha is omitted, then the color is opaque
    if (count == 6)
      color_.set_alpha(0xFF);
    else
      color_.Premultiply();
  } else {
   ScanSimpleToken();
  }
}

void Lexer::ScanSimpleToken() {
  char character = text_[position_];
  ++position_;
  switch (character) {
    case '.':
      token_ = DOT;
      break;
    case ',':
      token_ = COMMA;
      break;
    case '-':
      token_ = MINUS;
      break;
    case '(':
      token_ = OPEN_PAREN;
      break;
    case ')':
      token_ = CLOSE_PAREN;
      break;
    case '*':
      token_ = STAR;
      break;
    default:
      token_ = UNKNOWN;
      break;
  }
}

bool Lexer::ParseInteger(int* value) {
  ASSERT(value);
  bool negative = CheckToken(Lexer::MINUS);
  if (!GetNumber(value))
    return false;
  if (negative)
    *value = -(*value);
  return true;
}

bool Lexer::ParseReal32(real32* value) {
  ASSERT(value);
  int whole = 1;
  int fraction = 0;
  bool found = ParseInteger(&whole);

  if (CheckToken(Lexer::DOT)) {
    // Ignore result, fraction is optional
    GetNumber(&fraction);
  }
  if (!found)
    return false;
  *value = static_cast<real32>(fraction);
  while (fraction > 0) {
    *value *= 0.1f;
    fraction /= 10;
  }
  if (whole < 0)
    *value = static_cast<real32>(whole) - *value;
  else
    *value = static_cast<real32>(whole) + *value;
  return true;
}

bool Lexer::ParseRotate(Transform* result) {
  ASSERT(result);
  real32 angle;
  if (CheckToken(Lexer::OPEN_PAREN) &&
      ParseReal32(&angle) &&
      CheckToken(Lexer::CLOSE_PAREN)) {
    result->AddRotation(angle);
    return true;
  }
  return false;
}

bool Lexer::ParseScale(Transform* result) {
  ASSERT(result);
  Vector scale;
  if (CheckToken(Lexer::OPEN_PAREN) &&
      ParseReal32(&scale.x) &&
      ParseReal32(&scale.y) &&
      CheckToken(Lexer::CLOSE_PAREN)) {
    result->AddScale(scale);
    return true;
  }
  return false;
}

bool Lexer::ParseTranslate(Transform* result) {
  ASSERT(result);
  Vector offset;
  if (CheckToken(Lexer::OPEN_PAREN) &&
      ParseReal32(&offset.x) &&
      ParseReal32(&offset.y) &&
      CheckToken(Lexer::CLOSE_PAREN)) {
    result->AddOffset(offset);
    return true;
  }
  return false;
}

bool Lexer::ParseMatrix(Transform* result) {
  ASSERT(result);
  real32 m00, m01, m10, m11, dx, dy;
  if (CheckToken(Lexer::OPEN_PAREN) &&
      ParseReal32(&m00) &&
      ParseReal32(&m01) &&
      ParseReal32(&m10) &&
      ParseReal32(&m11) &&
      ParseReal32(&dx) &&
      ParseReal32(&dy) &&
      CheckToken(Lexer::CLOSE_PAREN)) {
    Transform transform(m00, m01, m10, m11, dx, dy);
    result->Set(transform);
    return true;
  }
  return false;
}

bool Lexer::ParseSingleTransform(Transform* result) {
  ASSERT(result);
  std::string prefix;

  if (!ParseIdentifier(&prefix))
    return false;

  Transform temp;
  if ((prefix == "matrix" && ParseMatrix(&temp)) ||
      (prefix == "translate" && ParseTranslate(&temp)) ||
      (prefix == "scale" && ParseScale(&temp)) ||
      (prefix == "rotate" && ParseRotate(&temp))) {
    result->Set(temp);
    return true;
  }
  return false;
}

}  // namespace glint


