# Copyright 2006, Google Inc.
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

# NOTE: The allowed ranges for these numbers are <0-255,0-255,0-654,0-99>.
#
# These ranges are derived from the format and limits of MSI ProductVersion
# numbers, which are: <0-255,0-255,0-65535>. We combine the last two Gears
# version fields to create the last MSI ProductVersion field. Splitting the
# range in this way gives us lots or range where we need it (we create lots of
# builds) and little where we don't need it (we create few patches). It also
# makes it easy to get the Gears version from MSI ProductVersion field.
#
# For more information on the MSI ProductVersion field, see:
# http://msdn.microsoft.com/en-us/library/aa370859(VS.85).aspx
MAJOR = 0
MINOR = 4
BUILD = 20
PATCH = 0

VERSION = $(MAJOR).$(MINOR).$(BUILD).$(PATCH)
