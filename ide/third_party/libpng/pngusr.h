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

#ifndef GEARS_THIRD_PARTY_LIBPNG_PNGUSR_H__
#define GEARS_THIRD_PARTY_LIBPNG_PNGUSR_H__

// Windows CE doesn't have an abort function.
#ifdef WINCE
  #define abort() exit(EXIT_FAILURE)
#endif

//
// Common flags
//

#define PNG_NO_GLOBAL_ARRAYS
#define PNG_NO_INFO_IMAGE
#define PNG_NO_USER_MEM
#define PNG_NO_FIXED_POINT_SUPPORTED
#define PNG_NO_MNG_FEATURES
#define PNG_NO_USER_TRANSFORM_PTR
// Change to get Skia to compile.
// #define PNG_NO_HANDLE_AS_UNKNOWN
#define PNG_NO_CONSOLE_IO
#define PNG_NO_ZALLOC_ZERO
#define PNG_NO_ERROR_NUMBERS
#define PNG_NO_EASY_ACCESS
#define PNG_NO_WARN_UNINITIALIZED_ROW
#define PNG_NO_WARNINGS
#define png_warning(s1,s2) (void)""
#define png_chunk_warning(s1,s2) (void)""
#define PNG_NO_ERROR_TEXT
#define png_error(s1,s2) png_err(s1)
#define png_chunk_error(s1,s2) png_err(s1)
#define deflateParams(a,b,c) Z_OK


//
// Read flags
//

#define PNG_NO_READ_GAMMA
#define PNG_NO_READ_BACKGROUND
#define PNG_NO_READ_DITHER
#define PNG_NO_READ_INVERT
#define PNG_NO_READ_SHIFT
#define PNG_NO_READ_PACKSWAP
#define PNG_NO_READ_SWAP_ALPHA
#define PNG_NO_READ_STRIP_ALPHA
#define PNG_NO_READ_INVERT_ALPHA
#define PNG_NO_READ_RGB_TO_GRAY
#define PNG_NO_READ_USER_TRANSFORM
#define PNG_NO_READ_bKGD
#define PNG_NO_READ_cHRM
#define PNG_NO_READ_gAMA
#define PNG_NO_READ_hIST
#define PNG_NO_READ_iCCP
#define PNG_NO_READ_oFFs
#define PNG_NO_READ_pCAL
#define PNG_NO_READ_pHYs
#define PNG_NO_READ_sCAL
#define PNG_NO_READ_sPLT
#define PNG_NO_READ_TEXT
#define PNG_NO_READ_tIME
#define PNG_NO_READ_EMPTY_PLTE
#define PNG_NO_READ_OPT_PLTE
// Changes to get Skia to compile:
// TODO(cprince): Remove these from Skia instead?
// #define PNG_NO_READ_sBIT
// #define PNG_NO_READ_FILLER
// #define PNG_NO_READ_UNKNOWN_CHUNKS
// #define PNG_NO_READ_USER_CHUNKS


#ifdef OFFICIAL_BUILD
#define PNG_NO_READ_PACK
#endif

// Gears addition: disables verifying the embedded CRCs when reading.
// (Zlib adds a static 8KB table, plus additional code, for CRC support.)
#define GEARS_PNG_NO_READ_VERIFY_CRC

//
// Write flags
//

#ifdef OFFICIAL_BUILD

#define PNG_NO_WRITE_SUPPORTED

#else

#define PNG_NO_WRITE_BACKGROUND
//#define PNG_NO_WRITE_GAMMA
#define PNG_NO_WRITE_DITHER
#define PNG_NO_WRITE_INVERT
#define PNG_NO_WRITE_SHIFT
#define PNG_NO_WRITE_PACK
#define PNG_NO_WRITE_PACKSWAP
#define PNG_NO_WRITE_FILLER
#define PNG_NO_WRITE_SWAP_ALPHA
#define PNG_NO_WRITE_INVERT_ALPHA
#define PNG_NO_WRITE_RGB_TO_GRAY
#define PNG_NO_WRITE_USER_TRANSFORM
#define PNG_NO_WRITE_bKGD
#define PNG_NO_WRITE_cHRM
//#define PNG_NO_WRITE_gAMA
//#define PNG_NO_WRITE_sRGB
#define PNG_NO_WRITE_hIST
#define PNG_NO_WRITE_iCCP
//#define PNG_NO_WRITE_oFFs
#define PNG_NO_WRITE_pCAL
#define PNG_NO_WRITE_pHYs
#define PNG_NO_WRITE_sBIT
#define PNG_NO_WRITE_sCAL
#define PNG_NO_WRITE_sPLT
#define PNG_NO_WRITE_TEXT
#define PNG_NO_WRITE_tIME
#define PNG_NO_WRITE_UNKNOWN_CHUNKS
#define PNG_NO_WRITE_USER_CHUNKS
#define PNG_NO_WRITE_EMPTY_PLTE
#define PNG_NO_WRITE_OPT_PLTE
//#define PNG_NO_WRITE_FILTER
//#define PNG_NO_WRITE_WEIGHTED_FILTER
//#define PNG_NO_WRITE_INTERLACING_SUPPORTED

#endif  // OFFICIAL_BUILD


#endif  // GEARS_THIRD_PARTY_LIBPNG_PNGUSR_H__
