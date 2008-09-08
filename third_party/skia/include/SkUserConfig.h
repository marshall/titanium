/* include/corecg/SkUserConfig.h
**
** Copyright 2006, Google Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#ifndef SkUserConfig_DEFINED
#define SkUserConfig_DEFINED

// BEGIN Gears changes:
#define SK_IGNORE_STDINT_DOT_H
#define SK_RESTRICT

typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;

#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)
inline void SkDebugf(const char format[], ...) {
}

#undef SK_RELEASE
#define SK_DEBUG
#undef SK_SCALAR_IS_FIXED
// END Gears changes.

#ifdef ANDROID
    #include <utils/misc.h>

#if 0
    // force fixed
    #define SK_SCALAR_IS_FIXED
    #undef  SK_SCALAR_IS_FLOAT
#else
    // force floats
    #ifdef SK_SCALAR_IS_FIXED
        #undef  SK_SCALAR_IS_FIXED
    #endif
    #define SK_SCALAR_IS_FLOAT
#endif

    #define SK_CAN_USE_FLOAT
    #define SkLONGLONG int64_t

    // replace some sw float routines (floor, ceil, etc.)
    #define SK_USE_FLOATBITS

    #if __BYTE_ORDER == __BIG_ENDIAN
        #define SK_CPU_BENDIAN
        #undef  SK_CPU_LENDIAN
    #else
        #define SK_CPU_LENDIAN
        #undef  SK_CPU_BENDIAN
    #endif

    // define SkDebugf to record file/line
    #define SkDebugf(...) Android_SkDebugf(__FILE__, __LINE__, \
                                           __FUNCTION__, __VA_ARGS__)
    void Android_SkDebugf(const char* file, int line, 
                          const char* function, const char* format, ...);
#endif

/*  This file is included before all other headers, except for SkPreConfig.h.
    That file uses various heuristics to make a "best guess" at settings for
    the following build defines.

    However, in this file you can override any of those decisions by either
    defining new symbols, or #undef symbols that were already set.
*/

// Gears:
// experimental for now
#undef SK_SUPPORT_MIPMAP

#ifdef SK_DEBUG
    #define SK_SUPPORT_UNITTEST
    /* Define SK_SIMULATE_FAILED_MALLOC to have
     * sk_malloc throw an exception. Use this to
     * detect unhandled memory leaks. */
    //#define SK_SIMULATE_FAILED_MALLOC
    //#define SK_FIND_MEMORY_LEAKS
#endif

#ifdef SK_BUILD_FOR_BREW
    #include "SkBrewUserConfig.h"
#endif

#endif

