# Copyright 2005, Google Inc.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#  1. Redistributions of source code must retain the above copyright notice,
#     this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the above copyright notice,
#     this list of conditions and the following disclaimer in the documentation
#     and/or other materials provided with the distribution.
#  3. Neither the name of Google Inc. nor the names of its contributors may be
#     used to endorse or promote products derived from this software without
#     specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
# EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# Workaround for weirdness in pulse were PWD is set incorrectly to the pulse
# bin directory, rather than the real current working directory.  $(shell )
# is expensive, so we only run it if PWD is set incorrectly, by checking
# for the existence of a file named 'Makefile' in the current directory.
ifeq ($(wildcard $(PWD)/Makefile),)
PWD = $(shell pwd)
endif

# Enable these libraries unless indicated otherwise.
ifeq ($(USING_ICU),)
  USING_ICU = 1
endif
ifeq ($(USING_LIBGD),)
  USING_LIBGD = 1
endif
ifeq ($(USING_LIBJPEG),)
  USING_LIBJPEG = 1
endif
ifeq ($(USING_LIBPNG),)
  USING_LIBPNG = 1
endif
ifeq ($(USING_LIBSPEEX),)
  USING_LIBSPEEX = 1
endif
ifeq ($(USING_LIBTREMOR),)
  USING_LIBTREMOR = 1
endif
ifeq ($(USING_MOZJS),)
  ifeq ($(BROWSER),SF)
    USING_MOZJS = 1
  endif
endif
ifeq ($(USING_PORTAUDIO),)
  USING_PORTAUDIO = 1
endif
ifeq ($(USING_SQLITE),)
  USING_SQLITE = 1
endif
ifeq ($(USING_ZLIB),)
  USING_ZLIB = 1
endif

# Make-ish way of saying: if (browser == SF || browser == NPAPI)
ifneq ($(findstring $(BROWSER), SF|NPAPI),)
  USING_NPAPI = 1
endif

# Store value of unmodified command line parameters.
ifdef MODE
  CMD_LINE_MODE = $MODE
endif

ifdef BROWSER
  CMD_LINE_BROWSER = $BROWSER
endif

# Discover the OS
ifeq ($(shell uname),Linux)
OS = linux
else
ifeq ($(shell uname),Darwin)
OS = osx
else
ifneq ($(OS),symbian)
  IS_WIN32_OR_WINCE = 1
  ifneq ($(OS),wince)
  OS = win32
  endif
endif
endif
endif

# Set default build mode
#   dbg = debug build
#   opt = release build
MODE = dbg

# Set default OS architecture
#   OSX and Symbian builds will override this value (see rules.mk).
#   For other platforms we set just one value.
ifeq ($(OS),wince)
  ARCH = arm
else
ifeq ($(OS),android)
  # default platform for android
  ARCH = arm
else
ifeq ($(OS),osx)
  # On OSX we build a fat binary.
  ARCH = i386+ppc
else
ifeq ($(OS),symbian)
  # Don't set a value here; Symbian builds for two ARCHs by default.
else
  ARCH = i386
endif
endif
endif
endif

# $(shell ...) statements need to be different on Windows (%% vs %).
ifdef IS_WIN32_OR_WINCE
PCT=%%
else
PCT=%
endif

# Enable breakpad for Safari and Notifier in MacOSX.
ifeq ($(OS),osx)
ifeq ($(BROWSER),SF)
  USING_BREAKPAD_OSX = 1
endif
ifeq ($(BROWSER),NONE)
  USING_BREAKPAD_OSX = 1
endif
endif

MAKEFLAGS += --no-print-directory

CPPFLAGS += -I.. -I$($(BROWSER)_OUTDIR) -I$(COMMON_OUTDIR)

# Additional include paths for gurl.
# TODO(cprince): change this to GURL_CFLAGS if we ever define USING_GURL.
CPPFLAGS += -I../third_party/npapi -I../third_party -I../third_party/googleurl

ICU_CFLAGS += -I../third_party/icu38/public/common

LIBPNG_CFLAGS += -DPNG_USER_CONFIG -I../third_party/zlib

ZLIB_CFLAGS += -DNO_GZIP -DNO_GZCOMPRESS
ifeq ($(OS),wince)
ZLIB_CFLAGS += -DNO_ERRNO_H
endif

ifeq ($(USING_ICU),1)
CFLAGS += $(ICU_CFLAGS)
CPPFLAGS += $(ICU_CFLAGS)
endif
ifeq ($(USING_LIBPNG),1)
CFLAGS += $(LIBPNG_CFLAGS)
CPPFLAGS += $(LIBPNG_CFLAGS)
endif
ifeq ($(USING_ZLIB),1)
CFLAGS += $(ZLIB_CFLAGS)
CPPFLAGS += $(ZLIB_CFLAGS)
endif

ifdef IS_WIN32_OR_WINCE
# Breakpad assumes it is in the include path
CPPFLAGS += -I../third_party/breakpad/src
endif

ifeq ($(OS),osx)
# Breakpad assumes it is in the include path
CPPFLAGS += -I../third_party/breakpad_osx/src
endif

ifeq ($(BROWSER),FF2)
GECKO_BASE = ../third_party/gecko_1.8
else
GECKO_BASE = ../third_party/gecko_1.9
endif
GECKO_BIN = $(GECKO_SDK)/gecko_sdk/bin
GECKO_LIB = $(GECKO_SDK)/gecko_sdk/lib
# GECKO_SDK gets defined below (different for each OS).

$(BROWSER)_CPPFLAGS += -DBROWSER_$(BROWSER)=1

# SpiderMonkey (the Firefox JS engine)'s JS_GET_CLASS macro in jsapi.h needs
# this defined to work with the gecko SDK that we've built.
# The definition of JS_THREADSAFE must be kept in sync with MOZJS_CPPFLAGS.
$(BROWSER)_CPPFLAGS += -DJS_THREADSAFE

# TODO(cprince): Update source files so we don't need this compatibility define?
FF2_CPPFLAGS += -DBROWSER_FF=1
FF3_CPPFLAGS += -DBROWSER_FF=1

# FF2/FF3_CPPFLAGS includes several different base paths of GECKO_SDK because
# different sets of files include SDK/internal files differently.
FF2_CPPFLAGS += -I$(GECKO_BASE) -I$(GECKO_SDK) -I$(GECKO_SDK)/gecko_sdk/include -DMOZILLA_STRICT_API
FF3_CPPFLAGS += -I$(GECKO_BASE) -I$(GECKO_SDK) -I$(GECKO_SDK)/gecko_sdk/include -DMOZILLA_STRICT_API
IE_CPPFLAGS +=
CHROME_CPPFLAGS += -I../third_party/v8/bindings_local

# These flags are needed so that instead of exporting all symbols defined in
# the code, we just export those specifically marked, this reduces the output size.
SF_CPPFLAGS += -fvisibility=hidden
SF_CXXFLAGS += -fvisibility-inlines-hidden

# When adding or removing SQLITE_OMIT_* options, also update and
# re-run ../third_party/sqlite_google/google_generate_preprocessed.sh.
SQLITE_CFLAGS += -DSQLITE_CORE \
  -DSQLITE_ENABLE_FTS1 -DSQLITE_ENABLE_BROKEN_FTS1 \
  -DSQLITE_ENABLE_FTS2 -DSQLITE_ENABLE_BROKEN_FTS2 \
  -DTHREADSAFE=1 -DSQLITE_DEFAULT_FILE_PERMISSIONS=0600 \
  -DSQLITE_OMIT_ATTACH=1 \
  -DSQLITE_OMIT_LOAD_EXTENSION=1 \
  -DSQLITE_OMIT_VACUUM=1 \
  -DSQLITE_TRANSACTION_DEFAULT_IMMEDIATE=1 \
  -I../third_party/sqlite_google/src -I../third_party/sqlite_google/preprocessed

SKIA_LIB_DIR = ../third_party/skia
ifeq ($(OFFICIAL_BUILD),1)
# Not finalized for official builds.
else
# Make-ish way of saying: if (OS == win32 || OS == osx)
ifneq ($(findstring $(OS), win32|osx),)
SKIA_LIB = $(SKIA_LIB_DIR)/$(LIB_PREFIX)skia-$(MODE)-$(OS)-$(ARCH)$(LIB_SUFFIX)
endif
endif

LIBGD_CFLAGS += -I../third_party/libjpeg -I../third_party/libpng -DHAVE_CONFIG_H

ifeq ($(USING_LIBGD),1)
# libGD assumes it is in the include path
CPPFLAGS += -I../third_party/libgd
endif

PORTAUDIO_CFLAGS += -I../third_party/portaudio/src/common -I../third_party/portaudio/include
LIBSPEEX_CFLAGS += -I../third_party/speex/include
LIBSPEEX_CFLAGS += -DFIXED_POINT -DDISABLE_FLOAT_API -DHAVE_CONFIG_H
LIBTREMOR_CFLAGS += -I../third_party/tremor/include -I../third_party/tremor/lib
GTEST_CPPFLAGS += -I../third_party/gtest/include -I../third_party/gtest

# Common items, like notifier, is not related to any browser.
COMMON_CPPFLAGS += -DBROWSER_NONE=1

######################################################################
# OS == linux
######################################################################
ifeq ($(OS),linux)
CC = gcc
CXX = g++
OBJ_SUFFIX = .o
MKDEP = gcc -M -MF $(@D)/$(*F).pp -MT $@ $(CPPFLAGS) $($(BROWSER)_CPPFLAGS) $<
ECHO=@echo

CPPFLAGS += -DLINUX
LIBGD_CFLAGS += -Wno-unused-variable -Wno-unused-function -Wno-unused-label
SQLITE_CFLAGS += -Wno-uninitialized -DHAVE_USLEEP=1
# for libjpeg:
THIRD_PARTY_CFLAGS = -Wno-main

PORTAUDIO_CFLAGS += -I../third_party/portaudio/src/os/unix
# for PortAudio: build only the OSS hostapi for linux
PORTAUDIO_CFLAGS += -DPA_USE_OSS -DHAVE_SYS_SOUNDCARD_H=1
# for PortAudio: disable some warnings
PORTAUDIO_CFLAGS += -Wno-unused-variable
# for PortAudio: enable multithreading support with pthread library
PORTAUDIO_CFLAGS += -pthread

# for Speex:
LIBSPEEX_CFLAGS += -Wno-unused-variable
# for Tremor:
LIBTREMOR_CFLAGS += -Wno-unused-variable -Wno-unused-function

# all the GTK headers using includes relative to this directory
GTK_CFLAGS = -I../third_party/gtk/include/gtk-2.0 -I../third_party/gtk/include/atk-1.0 -I../third_party/gtk/include/glib-2.0 -I../third_party/gtk/include/pango-1.0 -I../third_party/gtk/include/cairo -I../third_party/gtk/lib/gtk-2.0/include -I../third_party/gtk/lib/glib-2.0/include
CPPFLAGS += $(GTK_CFLAGS)

COMPILE_FLAGS_dbg = -g -O0
COMPILE_FLAGS_opt = -O2
COMPILE_FLAGS = -c -o $@ -fPIC -fmessage-length=0 -Wall -Werror $(COMPILE_FLAGS_$(MODE))
# NS_LITERAL_STRING does not work properly without this compiler option
COMPILE_FLAGS += -fshort-wchar

CFLAGS = $(COMPILE_FLAGS)
CXXFLAGS = $(COMPILE_FLAGS) -fno-exceptions -fno-rtti -Wno-non-virtual-dtor -Wno-ctor-dtor-privacy -funsigned-char -Wno-char-subscripts

SHARED_LINKFLAGS = -o $@ -fPIC -Bsymbolic -pthread

MKLIB = ar
LIB_PREFIX = lib
LIB_SUFFIX = .a
LIBFLAGS = rcs $@

MKDLL = g++
DLL_PREFIX = lib
DLL_SUFFIX = .so
DLLFLAGS = $(SHARED_LINKFLAGS) -shared -Wl,--version-script -Wl,tools/xpcom-ld-script
# for PortAudio: need pthread and math
DLLFLAGS += -lpthread -lm

MKEXE = g++
EXE_PREFIX =
EXE_SUFFIX =
EXEFLAGS = $(SHARED_LINKFLAGS)

# These aren't used on Linux because ld doesn't support "@args_file".
#TRANSLATE_LINKER_FILE_LIST = cat -
#EXT_LINKER_CMD_FLAG = -Xlinker @

GECKO_SDK = $(GECKO_BASE)/linux

# Keep these in sync:
FF2_LIBS = -L$(GECKO_SDK)/gecko_sdk/lib -lxpcom -lxpcomglue_s
FF3_LIBS = -L$(GECKO_SDK)/gecko_sdk/lib -lxpcom -lxpcomglue_s
# Append differences here:
# Although the 1.9 SDK contains libnspr4, it is better to link against libxul,
# which in turn depends on libnspr4. In Ubuntu 8.04, libnspr4 was not listed in
# /usr/lib, only libxul was.
FF2_LIBS += -lnspr4
FF3_LIBS += -lxul
endif

######################################################################
# OS == osx
######################################################################
ifeq ($(OS),osx)
CC = gcc -arch ppc -arch i386
CXX = g++ -arch ppc -arch i386
OBJ_SUFFIX = .o
MKDEP = gcc -M -MF $(@D)/$(*F).pp -MT $@ $(CPPFLAGS) $($(BROWSER)_CPPFLAGS) $<
ECHO=@echo

CPPFLAGS += -DOS_MACOSX
# for breakpad
CPPFLAGS += -DUSE_PROTECTED_ALLOCATIONS=1

ifeq ($(USING_MOZJS),1)
CPPFLAGS += -I ../third_party/spidermonkey/nspr/pr/include
endif

ifeq ($(BROWSER),SF)
# SAFARI-TEMP
# Remove these - During development, it was convenient to have these defined in
# the Safari port.  Before release we want to clean this up, and replace these
# with a single BROWSER_SF symbol.
# We also want to consolidate the include paths, so we don't have to add these
# paths here.
CPPFLAGS += -DBROWSER_NPAPI -DBROWSER_WEBKIT -DBROWSER_SAFARI
else
CPPFLAGS += -DLINUX
endif

# Needed for the Safari package installer.
M4FLAGS += -DGEARS_ENABLER_PATH='$(PWD)/$(SF_INPUTMANAGER_BUNDLE)' -DGEARS_PLUGIN_PATH="$(PWD)/$(SF_PLUGIN_PROXY_BUNDLE)"
M4FLAGS += -DGEARS_INSTALLER_OUT_DIR='$(PWD)/$(INSTALLERS_OUTDIR)/Safari'

LIBGD_CFLAGS += -Wno-unused-variable -Wno-unused-function -Wno-unused-label

# JS_THREADSAFE *MUST* be kept in sync wuith $(BROWSER)_CPPFLAGS.
MOZJS_CFLAGS += -DJS_THREADSAFE
MOZJS_CFLAGS += -DXP_UNIX -DDARWIN -DHAVE_BSD_FLOCK -DXP_MACOSX -DHAVE_LCHOWN \
                -DHAVE_STRERROR -DFORCE_PR_LOG -D_PR_PTHREADS \
                -DUHAVE_CVAR_BUILT_ON_SEM -D_NSPR_BUILD_ \
                -DOSARCH=Darwin -DSTATIC_JS_API -DJS_USE_SAFE_ARENA \
                -DTRIMMED -DJS_HAS_EXPORT_IMPORT \
                -I ../third_party/spidermonkey/nspr/pr/include/private \
                -I ../third_party/spidermonkey/nspr/pr/include \
                -I ../third_party/spidermonkey/nspr/pr/include/obsolete \
                -I $(OSX_SDK_ROOT)/Developer/Headers/FlatCarbon/

SQLITE_CFLAGS += -Wno-uninitialized -Wno-pointer-sign -isysroot $(OSX_SDK_ROOT)
SQLITE_CFLAGS += -DHAVE_USLEEP=1
# for libjpeg:
THIRD_PARTY_CFLAGS = -Wno-main

# for gtest:
GTEST_CPPFLAGS += -DGTEST_NOT_MAC_FRAMEWORK_MODE

PORTAUDIO_CFLAGS += -I../third_party/portaudio/src/os/unix
# for PortAudio: build only the CoreAudio hostapi for osx
PORTAUDIO_CFLAGS += -DPA_USE_COREAUDIO
# for PortAudio: disable some warnings
PORTAUDIO_CFLAGS += -Wno-unused-variable -Wno-uninitialized

# for Ogg:
LIBOGG_CFLAGS += -D__MACOSX__

# for Speex:
LIBSPEEX_CFLAGS += -Wno-unused-variable

# for Tremor:
LIBTREMOR_CFLAGS += -Wno-unused-variable -Wno-unused-function

# COMMON_CPPFLAGS affects non-browser-specific code, generated in /common.
# -DOSX is needed by Glint 3-rd party library built into notifier.
COMMON_CPPFLAGS += -fvisibility=hidden -DOSX
COMMON_CXXFLAGS += -fvisibility-inlines-hidden

COMPILE_FLAGS_dbg = -O0
COMPILE_FLAGS_opt = -O2
ifeq ($(USING_BREAKPAD_OSX),1)
# Breakpad on OSX needs debug symbols to use the STABS format, rather than the
# default DWARF debug symbols format. Note that we enable gstabs for debug &
# opt; we strip them later in opt.
COMPILE_FLAGS += -gstabs+
else
# TODO(playmobil): use gstabs+ universally once we have Breakpad working for
# the FF OSX release.
COMPILE_FLAGS_dbg += -g
endif
COMPILE_FLAGS += -c -o $@ -fPIC -fmessage-length=0 -Wall $(COMPILE_FLAGS_$(MODE)) -isysroot $(OSX_SDK_ROOT)
# NS_LITERAL_STRING does not work properly without this compiler option
COMPILE_FLAGS += -fshort-wchar


# TODO(playmobil): Remove this condition and move -Werror directly into the COMPILE_FLAGS definition.
ifeq ($(BROWSER),SF)
# SAFARI-TEMP
# Need to re-enable -Werror for Safari port.
else
COMPILE_FLAGS += -Werror
endif

CFLAGS = $(COMPILE_FLAGS)
CXXFLAGS += $(COMPILE_FLAGS) -fno-exceptions -fno-rtti -Wno-non-virtual-dtor -Wno-ctor-dtor-privacy -funsigned-char -Wno-char-subscripts
CXXFLAGS += -include base/safari/prefix_header.h

# When a function is deprecated in gcc, it stupidly warns about all functions
# and member functions that have the same name, regardless of signature.
# Example: Standard osx headers deprecate 'SetPort', which causes a warning for
# url_canon::Replacements::SetPort().
CXXFLAGS += -Wno-deprecated-declarations

BREAKPAD_CPPFLAGS += -fvisibility=hidden
BREAKPAD_CXXFLAGS += -fvisibility-inlines-hidden

THIRD_PARTY_CPPFLAGS += -fvisibility=hidden
THIRD_PARTY_CXXFLAGS += -fvisibility-inlines-hidden

SHARED_LINKFLAGS = -o $@ -fPIC -Bsymbolic -arch ppc -arch i386 -isysroot $(OSX_SDK_ROOT) -Wl,-dead_strip

ifeq ($(USING_BREAKPAD_OSX),1)
# libcrypto required by breakpad.
SHARED_LINKFLAGS += -lcrypto
endif

# Command to strip debug symbols from a binary, we need this so we can dump
# the symbols from an opt build for breakpad, and then strip the symbols
# as a separate phase.
# TODO(playmobil): Remove the conditional and add to FF targets once we have
# Breakpad working for the FF OSX release.
ifeq ($(USING_BREAKPAD_OSX),1)
ifeq ($(MODE),opt)
STRIP_EXECUTABLE = /usr/bin/strip -S $@
else
STRIP_EXECUTABLE =
endif
endif

MKLIB = libtool
LIB_PREFIX = lib
LIB_SUFFIX = .a
LIBFLAGS = -static -o $@

MKDLL = g++
DLL_PREFIX = lib
DLL_SUFFIX = .dylib
DLLFLAGS = $(SHARED_LINKFLAGS) -bundle -framework Carbon -framework CoreServices -framework Cocoa
ifeq ($(BROWSER),SF)
DLLFLAGS += -mmacosx-version-min=10.4 -framework WebKit -lcurl
DLLFLAGS += -Ltools/osx -lleopard_support
DLLFLAGS += -L../plugin/titanium_plugin/scintilla -lscintilla
else
DLLFLAGS += -Wl,-exported_symbols_list -Wl,tools/xpcom-ld-script.darwin
endif
# for PortAudio: need CoreAudio, AudioToolbox, AudioUnit, Carbon frameworks
DLLFLAGS += -framework CoreAudio -framework AudioToolbox -framework AudioUnit -framework Carbon

MKEXE = g++
EXE_PREFIX =
EXE_SUFFIX =
EXEFLAGS += $(SHARED_LINKFLAGS) -framework Carbon -framework CoreServices
EXEFLAGS += -framework Cocoa -mmacosx-version-min=10.4
ifeq ($(BROWSER),NONE)
EXEFLAGS += -framework IOKit -framework ScreenSaver
endif

# ld on OSX requires filenames to be separated by a newline, rather than spaces
# used on most platforms. So TRANSLATE_LINKER_FILE_LIST changes ' ' to '\n'.
# We also filter out empty lines since ld chokes on them.
TRANSLATE_LINKER_FILE_LIST = egrep '.+' | tr " " "\n"
# Use '\' at the end of line to preserve the line trailing space from editors
# that remove them automatically. This particular space is important.
EXT_LINKER_CMD_FLAG = -filelist \

# Empty comment to accompany the trailing '\' above.

GECKO_SDK = $(GECKO_BASE)/osx
OSX_SDK_ROOT = /Developer/SDKs/MacOSX10.4u.sdk

# Keep these in sync:
FF2_LIBS = -L$(GECKO_SDK)/gecko_sdk/lib -lxpcom -lmozjs -lnspr4 -lplds4 -lplc4
FF3_LIBS = -L$(GECKO_SDK)/gecko_sdk/lib -lxpcom -lmozjs -lnspr4 -lplds4 -lplc4
# Append differences here:
FF2_LIBS +=  -lxpcom_core
FF3_LIBS +=  $(GECKO_SDK)/gecko_sdk/lib/XUL $(GECKO_SDK)/gecko_sdk/lib/libxpcomglue_s.a -lsqlite3 -lsmime3 -lssl3 -lnss3 -lnssutil3 -lsoftokn3

# Iceberg command line tool.
ICEBERG = /usr/local/bin/freeze

# Iceberg may not be present. Record whether it exists, so we can skip targets
# that need it.
ifeq ($(wildcard $(ICEBERG)),)
else
HAVE_ICEBERG = 1
endif

endif

######################################################################
# OS == win32 OR wince
######################################################################
ifdef IS_WIN32_OR_WINCE
CC = cl
CXX = cl
OBJ_SUFFIX = .obj
MKDEP = python tools/mkdepend.py $< $@ > $(@D)/$(*F).pp

# echo.exe outputs "Echo on." (or "Echo off.") if it is called with no
# arguments. This is no good for us because sometimes we mean to call it with
# no arguments and have it output empty string. The workaround is to call it
# with a trailing dot. For more info, see:
# http://blogs.msdn.com/oldnewthing/archive/2008/04/03/8352719.aspx
ECHO=@echo.

# Most Windows headers use the cross-platform NDEBUG and DEBUG #defines
# (handled later).  But a few Windows files look at _DEBUG instead.
CPPFLAGS_dbg = -D_DEBUG=1
CPPFLAGS_opt =
CPPFLAGS += /nologo -DSTRICT -D_UNICODE -DUNICODE -D_USRDLL -DWIN32 -D_WINDLL \
            -D_CRT_SECURE_NO_DEPRECATE -DNOMINMAX

# PortAudio assumes it is in the include path
PORTAUDIO_CFLAGS += -I../third_party/portaudio/src/os/win
# for PortAudio: build only the MME hostapi for win32/wince
PORTAUDIO_CFLAGS += -DPA_NO_DS -DPA_NO_ASIO

ifeq ($(OS),win32)
# We require APPVER=5.0 for things like HWND_MESSAGE.
# When APPVER=5.0, win32.mak in the Platform SDK sets:
#   C defines:  WINVER=0x0500
#               _WIN32_WINNT=0x0500
#               _WIN32_IE=0x0500
#               _RICHEDIT_VER=0x0010
#   RC defines: WINVER=0x0500
#   MIDL flags: /target NT50
# Note: _WIN32_WINDOWS was replaced by _WIN32_WINNT for post-Win95 builds.
# Note: XP_WIN is only used by Firefox headers
CPPFLAGS += -D_WINDOWS \
            -DWINVER=0x0500 \
            -D_WIN32_WINNT=0x0500 \
            -D_WIN32_IE=0x0500 \
            -D_RICHEDIT_VER=0x0010 \
            -D_MERGE_PROXYSTUB \
            -DBREAKPAD_AVOID_STREAMS \
            -DXP_WIN \
            $(CPPFLAGS_$(MODE))
else
# For Windows Mobile we need:
#   C defines:  _WIN32_WCE=0x0501
#               _UNDER_CE=0x0501
CPPFLAGS += -D_WIN32_WCE=0x501 \
            -DWINVER=_WIN32_WCE \
            -DUNDER_CE=0x501 \
            -DWINCE \
            -DWIN32_PLATFORM_PSPC \
            -DARM \
            -D_ARM_ \
            -DPOCKETPC2003_UI_MODEL \
            -D_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA \
            -D_CE_CRT_ALLOW_WIN_MINMAX \
            $(CPPFLAGS_$(MODE))
endif

LIBGD_CFLAGS += -DBGDWIN32

# Disable some warnings when building third-party code, so we can enable /WX.
# Examples:
#   warning C4244: conversion from 'type1' to 'type2', possible loss of data
#   warning C4018: signed/unsigned mismatch in comparison
#   warning C4003: not enough actual parameters for macro
LIBGD_CFLAGS += /wd4244 /wd4996 /wd4005 /wd4142 /wd4018 /wd4133 /wd4102

SQLITE_CFLAGS += /wd4018 /wd4244
ifeq ($(OS),wince)
SQLITE_CFLAGS += /wd4146
endif

BREAKPAD_CPPFLAGS += /wd4018 /wd4003
THIRD_PARTY_CPPFLAGS += /wd4018 /wd4003
# for PortAudio:
#   warning C4133: 'type' : incompatible types - from 'type1' to 'type2'
#   warning C4101: 'identifier' : unreferenced local variable
PORTAUDIO_CFLAGS += /wd4133 /wd4101

# for Speex:
LIBSPEEX_CFLAGS += -I../third_party/speex/win32
#   warning C4244: conversion from 'type1' to 'type2', possible loss of data
#   warning C4305: truncation from 'type1' to 'type2'
LIBSPEEX_CFLAGS += /wd4244 /wd4305

# for Tremor:
#   warning C4244: conversion from 'type1' to 'type2', possible loss of data
#   warning C4305: truncation from 'type1' to 'type2'
#   warning C4005: macro redefinition
LIBTREMOR_CFLAGS += /wd4244 /wd4305 /wd4005

COMPILE_FLAGS_dbg = /MTd /Zi /Zc:wchar_t-
COMPILE_FLAGS_opt = /MT  /Zi /Zc:wchar_t- /O2
COMPILE_FLAGS = /c /Fo"$@" /Fd"$(@D)/$(*F).pdb" /W3 /WX /GR- $(COMPILE_FLAGS_$(MODE))
# In VC8, the way to disable exceptions is to remove all /EH* flags, and to
# define _HAS_EXCEPTIONS=0 (for C++ headers) and _ATL_NO_EXCEPTIONS (for ATL).
COMPILE_FLAGS += -D_HAS_EXCEPTIONS=0 -D_ATL_NO_EXCEPTIONS
# Do not export UTF functions.
COMPILE_FLAGS += -DU_STATIC_IMPLEMENTATION

CFLAGS = $(COMPILE_FLAGS)
CXXFLAGS = $(COMPILE_FLAGS) /TP /J

# /RELEASE adds a checksum to the PE header to aid symbol loading.
# /DEBUG causes PDB files to be produced.
# We want both these flags in all build modes, despite their names.
SHARED_LINKFLAGS_dbg =
SHARED_LINKFLAGS_opt = /INCREMENTAL:NO /OPT:REF /OPT:ICF
SHARED_LINKFLAGS = /NOLOGO /OUT:$@ /DEBUG /RELEASE
ifeq ($(OS),win32)
SHARED_LINKFLAGS += \
	/MACHINE:X86 \
	/NODEFAULTLIB:msvcrt \
	$(SHARED_LINKFLAGS_$(MODE))
# Flags for security hardening (only available for win32, not wince).
SHARED_LINKFLAGS += \
	/DYNAMICBASE /SAFESEH
else
SHARED_LINKFLAGS += \
	/MACHINE:THUMB \
	/NODEFAULTLIB:secchk.lib \
	/NODEFAULTLIB:oldnames.lib \
	$(SHARED_LINKFLAGS_$(MODE))
endif


ifeq ($(OS),wince)
# Minidumps are not enabled in WinCE.
else
# Settings for enabling trace buffers. (These flags must be used together.)
# Minidumps are only sent in official builds, so enable trace buffers only in
# official (dbg) builds.
ifeq ($(OFFICIAL_BUILD),1)
COMPILE_FLAGS_dbg += /fastcap
SHARED_LINKFLAGS_dbg += base\common\trace_buffers_win32\trace_buffers_win32.lib
endif # OFFICIAL_BUILD
endif # wince / win32

MKLIB = lib
LIB_PREFIX =
LIB_SUFFIX = .lib
LIBFLAGS = /OUT:$@

MKDLL = link
DLL_PREFIX =
DLL_SUFFIX = .dll
# We need DLLFLAGS_NOPDB for generating other targets than gears.dll
# (e.g. setup.dll for Windows Mobile).
DLLFLAGS_NOPDB = $(SHARED_LINKFLAGS) /DLL
# Wo only use /SUBSYSTEM on DLLs. For EXEs we omit the flag, and
# the presence of main() or WinMain() determines the subsystem.
ifeq ($(OS),win32)
DLLFLAGS_NOPDB += /SUBSYSTEM:WINDOWS
else
DLLFLAGS_NOPDB += /SUBSYSTEM:WINDOWSCE,5.01
endif
DLLFLAGS = $(DLLFLAGS_NOPDB) /PDB:"$(@D)/$(MODULE).pdb"

FF2_DLLFLAGS =
FF3_DLLFLAGS =
IE_DLLFLAGS = /DEF:tools/mscom.def

CHROME_DLLFLAGS = /DEF:base/chrome/module.def

# Set the preferred base address.  This value was chosen because (a) it's near
# the top of the valid address range, and (b) it doesn't conflict with other
# DLLs loaded by Chrome in either the browser or plugin process.
CHROME_DLLFLAGS += /BASE:0x65000000

CHROME_CPPFLAGS += -DBROWSER_CHROME=1

MKEXE = link
EXE_PREFIX =
EXE_SUFFIX = .exe
# Note: cannot use *F because that only works when the rule uses patterns.
EXEFLAGS = $(SHARED_LINKFLAGS) /PDB:"$(@D)/$(patsubst %.exe,%.pdb,$(@F))"

TRANSLATE_LINKER_FILE_LIST = cat -
EXT_LINKER_CMD_FLAG = @

GECKO_SDK = $(GECKO_BASE)/win32

FF2_LIBS = $(GECKO_LIB)/xpcom.lib $(GECKO_LIB)/xpcomglue_s.lib $(GECKO_LIB)/nspr4.lib $(GECKO_LIB)/js3250.lib ole32.lib shell32.lib shlwapi.lib advapi32.lib wininet.lib comdlg32.lib user32.lib
FF3_LIBS = $(GECKO_LIB)/xpcom.lib $(GECKO_LIB)/xpcomglue_s.lib $(GECKO_LIB)/nspr4.lib $(GECKO_LIB)/js3250.lib ole32.lib shell32.lib shlwapi.lib advapi32.lib wininet.lib comdlg32.lib user32.lib
ifeq ($(OS),win32)
IE_LIBS = kernel32.lib user32.lib gdi32.lib uuid.lib sensapi.lib shlwapi.lib shell32.lib advapi32.lib wininet.lib comdlg32.lib user32.lib
else # wince
IE_LIBS = wininet.lib ceshell.lib coredll.lib corelibc.lib ole32.lib oleaut32.lib uuid.lib commctrl.lib atlosapis.lib piedocvw.lib cellcore.lib htmlview.lib imaging.lib toolhelp.lib aygshell.lib iphlpapi.lib gpsapi.lib
endif
NPAPI_LIBS = delayimp.lib /DELAYLOAD:"comdlg32.dll" comdlg32.lib
NOTIFIER_SHELL_LIBS = advapi32.lib shell32.lib shlwapi.lib

# Other tools specific to win32/wince builds.
RC = rc
RCFLAGS_dbg = -DDEBUG=1
RCFLAGS_opt = -DNDEBUG=1
RCFLAGS = $(RCFLAGS_$(MODE)) -D_UNICODE -DUNICODE -I$($(BROWSER)_OUTDIR) -I$(COMMON_OUTDIR) /l 0x409 /fo"$(@D)/$(*F).res"
ifeq ($(OS),wince)
RCFLAGS += -DWINCE -D_WIN32 -D_WIN32_WCE -DUNDER_CE -N -I..
endif

GGUIDGEN = tools/gguidgen.exe
endif

######################################################################
# set the following for all OS values
######################################################################

CPPFLAGS += $(INCLUDES)
M4FLAGS  += --prefix-builtins
M4FLAGS  += -I$($(BROWSER)_OUTDIR)

ifeq ($(MODE),dbg)
CPPFLAGS += -DDEBUG=1
M4FLAGS  += -DDEBUG=1
else
CPPFLAGS += -DNDEBUG=1
M4FLAGS  += -DNDEBUG=1
endif

ifeq ($(OFFICIAL_BUILD),1)
CPPFLAGS += -DOFFICIAL_BUILD=1
M4FLAGS  += -DOFFICIAL_BUILD=1
endif

# Add USING_CCTESTS in debug builds and non-official opt builds.
# This adds the GearsTest object (gears/cctest), which can be
# used to access a perf timer and run the C++ unit tests.
# It also adds notifier_test to run tests for the notifier
# application.
USING_CCTESTS = 0
ifeq ($(MODE),dbg)
USING_CCTESTS = 1
else
ifneq ($(OFFICIAL_BUILD),1)
USING_CCTESTS = 1
endif
endif

ifeq ($(USING_CCTESTS),1)
CPPFLAGS += -DUSING_CCTESTS=1 $(GTEST_CPPFLAGS)
M4FLAGS  += -DUSING_CCTESTS=1
endif

# Additional values needed for M4 preprocessing

M4FLAGS  += -DPRODUCT_VERSION=$(VERSION)
M4FLAGS  += -DPRODUCT_VERSION_MAJOR=$(MAJOR)
M4FLAGS  += -DPRODUCT_VERSION_MINOR=$(MINOR)
M4FLAGS  += -DPRODUCT_VERSION_BUILD=$(BUILD)
M4FLAGS  += -DPRODUCT_VERSION_PATCH=$(PATCH)

M4FLAGS  += -DPRODUCT_OS=$(OS)
ifeq ($(ARCH), i386)
M4FLAGS  += -DPRODUCT_ARCH="x86"
else
M4FLAGS  += -DPRODUCT_ARCH="$(ARCH)"
endif

M4FLAGS  += -DPRODUCT_GCC_VERSION="gcc3"
M4FLAGS  += -DPRODUCT_MAINTAINER="google"

# These three macros are suggested by the GNU make documentation for creating
# a comma-separated list.
COMMA    := ,
EMPTY    :=
SPACE    := $(EMPTY) $(EMPTY)
M4FLAGS  += -DI18N_LANGUAGES="($(subst $(SPACE),$(COMMA),$(strip $(I18N_LANGS))))"

# The friendly name can include spaces.
# The short name should be lowercase_with_underscores.
# "UQ" means "un-quoted".
# IMPORTANT: these values affect most visible names, but there are exceptions.
# If you change a name below, also search the code for "[naming]"

FRIENDLY_NAME=Appcelerator Titanium
SHORT_NAME=gears
M4FLAGS  += -DPRODUCT_FRIENDLY_NAME_UQ="$(FRIENDLY_NAME)"
M4FLAGS  += -DPRODUCT_SHORT_NAME_UQ="$(SHORT_NAME)"
