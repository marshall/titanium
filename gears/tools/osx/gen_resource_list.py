#!/usr/bin/env python
#
# Copyright 2008 Google Inc. All Rights Reserved.


"""Generate the resource_list.h file, for more info see 
gears/base/safari/resource_archive.mm

 usage:
 >  gen_resource_list.py out_file.h a.h b.h c.h
 
 Where the .h files are dumps of webarchives created by xxd -i <file>
"""

__author__ = 'playmobil@google.com (Jeremy Moskovich)'

import os
import re
import sys


def extract_identifiers(fl):
  """Extract the array name and array length symbols xxd outputs
  these are present on the first and last lines of the file.
  
  first line looks like:
  unsigned char foo[] = {
  
  last line looks like:
  unsigned int foo_len = 56419;
  """
  symbol_regexp = re.compile(r'unsigned char\s+([^\[]+)\[')
  symbol_len_regexp = re.compile(r'int\s+([^=]+)\s+')
  file_lines = open(fl).readlines()
  first_line = file_lines[0]
  last_line = file_lines[-1]
  
  symbol     = symbol_regexp.search(first_line).group(1)
  symbol_len = symbol_len_regexp.search(last_line).group(1)
  
  return (symbol, symbol_len)
  

def write_header_file(out_filename, header_file_list):
  """Creates resource_list.h file that's included from 
  gears/base/safari/resource_archive.cc.
  
  The file looks like:
  #include "a.h"
  StaticResource g_static_resources[] = {
    {std::string16(STRING16(L"foo"), ptr1, ptr1_len},
    ...
  };
  """
  fl = open(out_filename, "w")
  
  # Include headers
  for header_file in header_file_list:
    print >> fl, '#include "%s"' % os.path.basename(header_file)
  
  # struct start
  print >> fl, 'StaticResource g_static_resources[] = {'

  # Print out data: file name as string, symbol name for file data pointer,
  # symbol name for data length.
  for header_file in header_file_list:
    (ptr_name, ptr_len_name) = extract_identifiers(header_file)
    header_file = os.path.basename(header_file).replace('.h', '.webarchive')
    print >> fl, '{std::string16(STRING16(L"%s")), %s, %s},' % (header_file, ptr_name, ptr_len_name)
    
  # struct end
  print >> fl, '};'
  

def main():
  
  out_filename = sys.argv[1]
  header_files = sys.argv[2:]
  
  write_header_file(out_filename, header_files)

if __name__ == "__main__":
  main()
