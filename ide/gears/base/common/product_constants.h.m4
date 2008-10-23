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

m4_changequote(`^',`^')m4_dnl
m4_changecom()m4_dnl
#ifndef GEARS_BASE_COMMON_PRODUCT_CONSTANTS_H__
#define GEARS_BASE_COMMON_PRODUCT_CONSTANTS_H__

#include "gears/base/common/string16.h"


#define PRODUCT_VERSION_STRING       L"PRODUCT_VERSION"
#define PRODUCT_VERSION_STRING_ASCII  "PRODUCT_VERSION"

#define ^PRODUCT_VERSION_MAJOR^  PRODUCT_VERSION_MAJOR
#define ^PRODUCT_VERSION_MINOR^  PRODUCT_VERSION_MINOR

#define PRODUCT_FRIENDLY_NAME       L"PRODUCT_FRIENDLY_NAME_UQ"
#define PRODUCT_FRIENDLY_NAME_ASCII  "PRODUCT_FRIENDLY_NAME_UQ"
#define PRODUCT_SHORT_NAME          L"PRODUCT_SHORT_NAME_UQ"
#define PRODUCT_SHORT_NAME_ASCII     "PRODUCT_SHORT_NAME_UQ"


#endif  // GEARS_BASE_COMMON_PRODUCT_CONSTANTS_H__
