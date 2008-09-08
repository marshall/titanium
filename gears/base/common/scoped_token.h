// Copyright 2006, Google Inc.
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

#ifndef GEARS_BASE_COMMON_SCOPED_TOKEN_H__
#define GEARS_BASE_COMMON_SCOPED_TOKEN_H__

#include <assert.h>
#include <algorithm>

// scoped_token manages the lifetime of a data structure with an opaque type,
// analogously to how scoped_ptr manages the lifetime of an object.

// Each scoped_token must provide a traits class which defines a Free function
// and a Default value.  The Free function will release all resources associated
// with the token.  The Default value represents an inactive token which is
// not managing any resources.  A token with the Default value will never be
// Freed.
//
//class TokenTypeTraits {
// public:
//  static void Free(TokenType x) { ... }
//  static TokenType Default() { return ...; }
//};

// Forward declaration of scoped_token.
template<typename TokenType, typename TokenTraits>
class scoped_token;
 
// Forward declaration of as_out_parameter.
template<typename TokenType, typename TokenTraits>
TokenType* as_out_parameter(scoped_token<TokenType, TokenTraits>&);

template<typename TokenType, typename TokenTraits>
class scoped_token {
 public:
  typedef TokenType element_type;

  scoped_token() : val_(TokenTraits::Default()) {}

  explicit scoped_token(TokenType v): val_(v) {}

  ~scoped_token() {
    if (TokenTraits::Default() != val_) {
      TokenTraits::Free(val_);
    }
  }

  void reset(TokenType v) {
    if (TokenTraits::Default() != val_) {
      TokenTraits::Free(val_);
    }
    val_ = v;
  }

  bool operator==(TokenType v) const {
    return val_ == v;
  }

  bool operator!=(TokenType v) const {
    return val_ != v;
  }

  TokenType get() const {
    return val_;
  }

  void swap(scoped_token& b) {
    std::swap(val_, b.val_);
  }

  TokenType release() {
    TokenType v = val_;
    val_ = TokenTraits::Default();
    return v;
  }

 private:
  TokenType val_;

  friend TokenType* as_out_parameter<>(scoped_token<TokenType, TokenTraits>&);

  // Disallow copy construction and assignment.
  scoped_token(const scoped_token&);
  scoped_token& operator=(const scoped_token&);

  // No reason to use these: each scoped_token should have its own object.
  template <typename U1, typename U2>
  bool operator==(scoped_token<U1, U2> const& v) const;
  template <typename U1, typename U2>
  bool operator!=(scoped_token<U1, U2> const& v) const;
};

template<typename TokenType, typename TokenTraits>
void swap(scoped_token<TokenType, TokenTraits>& a,
          scoped_token<TokenType, TokenTraits>& b) {
  a.swap(b);
}

template<typename TokenType, typename TokenTraits>
bool operator==(TokenType v, const scoped_token<TokenType, TokenTraits>& b) {
  return v == b.get();
}

template<typename TokenType, typename TokenTraits>
bool operator!=(TokenType v, const scoped_token<TokenType, TokenTraits>& b) {
  return v != b.get();
}

// Returns the internal address of the token, suitable for passing to functions
// that return a token as an out parameter.
// Example:
//   bool CreateToken(Foo* created);
//   scoped_token<Foo, FreeFoo, NULL> foo;
//   CreateObject(as_out_parameter(foo));
template<typename TokenType, typename TokenTraits>
TokenType* as_out_parameter(scoped_token<TokenType, TokenTraits>& p) {
  assert(TokenTraits::Default() == p.val_);
  return &p.val_;
}

// DECLARE_SCOPED_TRAITS conveniently declares a scoped_token traits class.
//  Usage:
//   typedef DECLARE_SCOPED_TRAITS(Handle, FreeHandle, NULL) HandleTraits;
//   typedef scoped_token<Handle, HandleTraits> scoped_Handle;
#define DECLARE_SCOPED_TRAITS(TokenType, FreeFunction, DefaultValue) \
class { \
 public: \
  static void Free(TokenType x) { FreeFunction(x); } \
  static TokenType Default() { return DefaultValue; } \
}

#endif // GEARS_BASE_COMMON_SCOPED_TOKEN_H__
