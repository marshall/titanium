#!/usr/bin/env python
#
# Copyright 2008, Google Inc.  All Rights Reserved
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

"""Outputs a file as a constant C array, so it can be bundled into a program.

USAGE: %s FILENAME [--bytetype=TYPENAME] [--arrayname=ARRAYNAME]

    FILENAME              : The file to output as a constant C array.
    --bytetype=TYPENAME   : The data type of each array element.
                            Default is 'unsigned char'.
    --arrayname=ARRAYNAME : The variable name of the array.
                            Default is FILENAME (sanitized for C).

Sample output:

#include <stddef.h>
extern const unsigned char array_name[] = {
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
};
extern const size_t array_name_size = 29;
"""

import re
import struct
import sys


MAX_BYTES_PER_LINE = 15

def PrintFileAsArray(file_name, byte_type, array_name):
  try:
    f = open(file_name, 'rb')
    content = f.read()
    length = len(content)

    print '#include <stddef.h>'
    print 'extern const %s %s[] = {' % (byte_type, array_name)

    position = 0
    while position < length:
      line_bytes = 0
      line_string = '  '
      while (line_bytes < MAX_BYTES_PER_LINE) and (position < length):
        byte_value = struct.unpack('B', content[position])[0]
        line_string += '0x%02X' % byte_value
        line_bytes += 1
        position += 1
        if (position < length):
          line_string += ','
      print line_string

    print '};'
    print 'extern const size_t %s_size = %d;' % (array_name, length)
    print

    f.close()

  except:
    sys.exit('Error reading file: %s' % file_name)


def PrintUsageAndExit(program_name):
  sys.exit(__doc__ % program_name)


def main(argv):
  # Set defaults.
  file_name = ''
  byte_type = 'unsigned char'
  array_name = ''

  # Parse arguments.
  program_name = argv.pop(0)
  while len(argv) > 0:
    if not argv[0].startswith('-'):
      file_name = argv[0]
    else:
      if argv[0].startswith('--bytetype='):
        byte_type = argv[0][len('--bytetype='):]
      elif argv[0].startswith('--arrayname='):
        array_name = argv[0][len('--arrayname='):]
      else:
        print 'Unrecognized argument: %s' % argv[0]
        print ''
        PrintUsageAndExit(program_name)
    argv.pop(0)

  if file_name == '':
    PrintUsageAndExit(program_name)

  if array_name == '':
    array_name = re.sub(r'[^A-Za-z0-9_]', r'_', file_name)

  PrintFileAsArray(file_name, byte_type, array_name)


if __name__ == '__main__':
  main(sys.argv)
