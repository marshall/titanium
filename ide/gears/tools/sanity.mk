# Copyright 2007, Google Inc.
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

# Sanity check for command line parameters
# When adding new MODEs or BROWSERs you will also need to update this file.

ifdef CMD_LINE_MODE
  ifneq ($(MODE),dbg)
  ifneq ($(MODE),opt)
    $(error MODE can only be one of dbg | opt)
  endif
  endif
endif

# Check that our browser target is relevant for current build OS.
ifdef CMD_LINE_BROWSER
  ifeq ($(OS),win32)
    ifneq ($(BROWSER),FF2)
    ifneq ($(BROWSER),FF3)
    ifneq ($(BROWSER),IE)
    ifneq ($(BROWSER),NPAPI)
    ifneq ($(BROWSER),NONE)
      $(error On Windows, BROWSER can only be one of: FF2 | FF3 | IE | NPAPI | NONE)
    endif
    endif
    endif
    endif
    endif
  else
  ifeq ($(OS),wince)
    ifneq ($(BROWSER),IE)
    ifneq ($(BROWSER),NONE)
      $(error On WinCE, BROWSER can only be one of: IE | NONE)
    endif
    endif
  else
  ifeq ($(OS),osx)
    ifneq ($(BROWSER),FF2)
    ifneq ($(BROWSER),FF3)
    ifneq ($(BROWSER),SF)
    ifneq ($(BROWSER),NONE)
      $(error On OS X, BROWSER can only be one of: FF2 | FF3 | SF | NONE)
    endif
    endif
    endif
    endif
  else
  ifeq ($(OS),linux)
    ifneq ($(BROWSER),FF2)
    ifneq ($(BROWSER),FF3)
    ifneq ($(BROWSER),NPAPI)
    ifneq ($(BROWSER),NONE)
      $(error On Linux, BROWSER can only be one of: FF2 | FF3 | NPAPI | NONE)
    endif
    endif
    endif
    endif
  else
  ifeq ($(OS),android)
    ifneq ($(BROWSER),NPAPI)
    ifneq ($(BROWSER),NONE)
      $(error On Android, BROWSER can only be one of: NPAPI | NONE)
    endif
    endif
  else
  ifeq ($(OS),symbian)
    ifneq ($(BROWSER),NPAPI)
    ifneq ($(BROWSER),NONE)
      $(error On Symbian, BROWSER can only be one of: NPAPI | NONE)
    endif
    endif
  else
    # Shouldn't get here, as config.mk should always set OS if the user doesn't.
    $(error Unrecognized OS)
  endif  # ifeq ($(OS),symbian)
  endif  # ifeq ($(OS),android)
  endif  # ifeq ($(OS),linux)
  endif  # ifeq ($(OS),osx)
  endif  # ifeq ($(OS),wince)
  endif  # ifeq ($(OS),win32)
endif
