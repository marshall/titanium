#!/usr/bin/env python
#
# Copyright 2008 Google Inc. All Rights Reserved.
"""Convert Convert a packproj.m4 file back into a .packproj file that iceberg
 can edit."""

__author__ = 'playmobil@google.com (Jeremy Moskovich)'

import os
import plistlib
import re
import sys
import iceberg_utils

def main():
  if (len(sys.argv) != 2):
    sys.exit("usage: m42iceberg.py FILE.packproj.m4")
  (m4_filename,) = sys.argv[1:]
  iceberg_utils.m42iceberg(m4_filename)

if __name__ == "__main__":
  main()