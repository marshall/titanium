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

"""Unit tests for mkdepend.py
"""


import unittest
import mkdepend

class TestMkDepend(unittest.TestCase):

  def testGrepForIncludes(self):
    file = """/* some comments */
#include "foo.h"
#include"baz.h"
#include_ "foo.h"
#include <baz.h>
#include  "bar.h"
"""
    self.assertEquals(mkdepend.grepForIncludes(file), ['foo.h', 'bar.h'])

  def testResolve(self):
    file_exists = {}
    def f_file_exists(f):
      if file_exists.has_key(f):
        return True
      return False

    file_exists['foo.h'] = True
    file_exists[r'..\baz.h'] = True
    self.assertEquals(mkdepend.resolve('foo.h', f_file_exists), 'foo.h')
    self.assertEquals(mkdepend.resolve('bar.h', f_file_exists), None)
    self.assertEquals(mkdepend.resolve('baz.h', f_file_exists), r'..\baz.h')

  def testGetResolvedIncludes(self):

    def mockResolve(filename):
      files = {'a':'a', 'b':'../b', 'd': 'd'}
      if files.has_key(filename):
        return files[filename]
      return None

    def mockGetIncludes(filename):
      return ['a', 'a', 'b', 'c', 'd']

    self.assertEquals(mkdepend.getResolvedIncludes('foo.c',
                                                   mockResolve,
                                                   mockGetIncludes),
                      ['a', 'a', '../b', 'd'])

  def testGetDependencies(self):
    def mock_getIncludes(filename):
      if dependencies.has_key(filename):
        return dependencies[filename]
      return []

    dependencies = {}
    # No dependencies.
    self.assertEquals(mkdepend.getDependencies(['a.cpp'], mock_getIncludes),
                      ['a.cpp'])

    # One dependency.
    dependencies['a.cpp'] = ['a.h']
    self.assertEquals(mkdepend.getDependencies(['a.cpp'], mock_getIncludes),
                      ['a.cpp', 'a.h'])

    # Multiple dependencies.
    dependencies['a.cpp'] = ['a.h', 'b.h']
    dependencies['b.h'] = ['bb.h', 'bc.h', 'bd.h']
    dependencies['bc.h'] = ['bca.h']
    self.assertEquals(mkdepend.getDependencies(['a.cpp'], mock_getIncludes),
                      ['a.cpp', 'a.h', 'b.h', 'bb.h', 'bc.h', 'bca.h', 'bd.h'])

  def testSortAndRemoveDupes(self):
    self.assertEquals(mkdepend.sortAndRemoveDupes(['b', 'a', 'a']), ['a', 'b'])

  def testSortDependencies(self):
    # No deps.
    self.assertEquals(mkdepend.sortDependencies(['main.cc']), ['main.cc'])

    # One dep.
    self.assertEquals(mkdepend.sortDependencies(['main.cc', 'foo.h']), ['main.cc', 'foo.h'])

    # Multiple deps.
    self.assertEquals(mkdepend.sortDependencies(['main.cc', 'foo.h', 'baz.h', 'bar.h']),
                      ['main.cc', 'bar.h', 'baz.h', 'foo.h'])

  def testPrintDependencies(self):
    output = ['']
    def mock_write(s):
      output[0] = output[0] + s

    mkdepend.printDependencies(['a.c', 'a.h', 'b.h'], 'a.obj', mock_write)
    self.assertEquals(output[0], 'a.obj : a.c a.h b.h ')

  def testCheckArgs(self):
    output = ['']
    def mock_write(s):
      output[0] = output[0] + s

    usage_string = "Usage: %s target source"
    self.assertEquals(mkdepend.checkArgs(
      ['mkdepends.py', 'main.cc', 'main.obj'], usage_string, mock_write),
                      True)
    self.assertEquals(output[0], '')

    self.assertEquals(mkdepend.checkArgs(
      ['mkdepends.py'], usage_string, mock_write),
                      False)
    self.assertEquals(output[0], 'Usage: mkdepends.py target source')


if __name__ == '__main__':
  unittest.main()
