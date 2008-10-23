#!/usr/bin/env python
#
# Copyright 2008 Google Inc. All Rights Reserved.
"""Convert an iceberg file into an m4 file useble by the build system."""

__author__ = 'playmobil@google.com (Jeremy Moskovich)'

import os
import plistlib
import re
import sys
import iceberg_utils

def main():
  if (len(sys.argv) != 2):
    sys.exit("usage: iceberg2m4.py FILE.packproj")
  (proj_filename,) = sys.argv[1:]
  iceberg_utils.iceberg2m4(proj_filename)

if __name__ == "__main__":
  main()