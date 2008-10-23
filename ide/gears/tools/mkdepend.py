#!/usr/bin/python2.4
#
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

"""A really crappy C/C++ dependency generator (TM).

usage: %s target source

Print a list of all dependencies of source, in a form suitable for including
in a Makefile, i.e.

target : source dep1 dep2 dep3...

We make a couple simplifying assumptions here:
   1.  We are only executed from the top-level source directory
   2.  We assume that our include search path is really simple (in fact, it is
       hard-coded -- yikes!)
   3.  We ignore the fact that some include files may be conditional (#ifdef)
"""

import os
import re
import sys

SEARCH_DIRS=['', '..']


def readAllLines(f):
  return open(f, 'r').read()


def grepForIncludes(file_contents):
  return re.findall('#include\s+"(.*)"', file_contents)


def getIncludes(filename):
  return grepForIncludes(readAllLines(filename))


def resolve(filename, f_fileexists =os.path.exists):
  """
  Scan the file system for a given file, trying each of the directories in
  SEARCH_DIRS.  If found, return the name of the file, otherwise return None.
  """
  for search_dir in SEARCH_DIRS:
    resolved_filename = os.path.join(search_dir, filename)
    if f_fileexists(resolved_filename):
      return resolved_filename

  return None


def getResolvedIncludes(filename, f_resolver =resolve, f_getIncludes =getIncludes):
  return filter(None,
                map(f_resolver,
                    f_getIncludes(filename)))


def getDependencies(filenames, f_getIncludes =getResolvedIncludes):
  """
  Return all the dependencies of the files in the list "filenames" by recursively
  applying the function f_getIncludes.
  """
  includes = []
  includes.extend(filenames)
  for filename in filenames:
    for fname_include in f_getIncludes(filename):
      includes.extend(getDependencies([fname_include], f_getIncludes))
  return includes


def sortAndRemoveDupes(list):
  h = {}
  for elt in list:
    h[elt] = 1
  l = h.keys()
  l.sort()
  return l


def printDependencies(deps, target, f_writer =sys.stdout.write):
  f_writer("%s : " % target)
  for dep in deps:
    f_writer(os.path.normpath(dep) + ' ')


def checkArgs(argv, usage_string, f_writer =sys.stderr.write):
  if len(argv) != 3:
    f_writer(usage_string % argv[0])
    return False
  return True


def sortDependencies(deps):
  """
  Return first element (the .cc file) + sort of remaining elements (the header files).
  """
  return [deps[0]] + sortAndRemoveDupes(deps[1:])


if __name__ == '__main__':
  if (not checkArgs(sys.argv, __doc__)):
    sys.exit(1)

  printDependencies(sortDependencies(getDependencies([sys.argv[1]])),
                    sys.argv[2])
