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

#ifndef GEARS_DATABASE2_COMMON_H__
#define GEARS_DATABASE2_COMMON_H__

#include "gears/base/common/string16.h"

// Errors
// INVALID_STATE_ERR (per spec) exception
extern const char16 *kInvalidStateError;
extern const char16 *kTransactionClosed;
extern const char16 *kPrepareError;
extern const char16 *kBindError;
extern const char16 *kStepError;
extern const char16 *kResultSetError;
extern const char16 *kLastRowIdOutOfRangeError;

// HTML5 spec error codes
// The transaction failed for reasons unrelated to the database itself and not
// covered by any other error code.
extern const int kUnknownNonDatabaseError;

// The statement failed for database reasons not covered by any other error
// code.
extern const int kOtherDatabaseError;

// The statement failed because the expected version of the database didn't
// match the actual database version.
extern const int kDatabaseVersionMismatch;

#endif // GEARS_DATABASE2_COMMON_H__
