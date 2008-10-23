#!/usr/bin/env python
#
# Copyright 2008 Google Inc. All Rights Reserved.
"""Convert an iceberg file into an m4 file useble by the build system."""
__author__ = 'playmobil@google.com (Jeremy Moskovich)'
import os
import plistlib
import re
import sys

# Perform in-place replacement of str_a with str_b in filename.
def replace(str_a, str_b, filename):
  pattern = re.compile(str_a)
  fl = open(filename, 'rb')
  out_lines = [a.replace(str_a, str_b) for a in fl.readlines()]
  fl.close()
  fl = open(filename, 'w')
  fl.write(''.join(out_lines))
  fl.close()

# Inserts M4 symbols as standins in a plist node.
def fix_plist_version(node):
  node['Attributes']['Settings'] \
    ['Version']['IFMajorVersion'] = 'PRODUCT_VERSION_MAJOR'
  node['Attributes']['Settings'] \
    ['Version']['IFMinorVersion'] = 'PRODUCT_VERSION_MINOR'

# Convert a packproj.m4 file back into a .packproj file that iceberg
# can edit.
def m42iceberg(m4_filename):
  if (not m4_filename.endswith('.packproj.m4')):
    sys.exit("error: input file must have .packproj.m4 extension.")  
  
  # Chop off .m4 extension.
  iceberg_filename = m4_filename[:-3]

  os.system('cp "%s" "%s"' % (m4_filename, iceberg_filename) );
  
  # Iceberg won't open a file where an integer tag contains a non-integral
  # value.
  replace('<integer>PRODUCT_VERSION_MAJOR</integer>', 
          '<integer>0</integer>',
          iceberg_filename);
  replace('<integer>PRODUCT_VERSION_MINOR</integer>', 
          '<integer>0</integer>',
          iceberg_filename);

# Convert a .packproj iceberg file to an m4 file.
def iceberg2m4(prof_filename):
  if (not prof_filename.endswith('.packproj')):
    sys.exit("error: input file not an iceberg file.")

  installer_file_name = prof_filename + ".m4"

  # a.packproj -> a.packproj.m4
  os.system('cp "%s" "%s"' % (prof_filename, installer_file_name) );

  installer = plistlib.Plist.fromFile(installer_file_name)
  fix_plist_version(installer['Hierarchy'])
  
  # In case of Metapackage which contains the Gears plugin as a sub-package
  # search through the components till we find Gears and fix it's version
  # as well.
  if (installer['Hierarchy']['Attributes'].has_key('Components')):
    components_list = installer['Hierarchy']['Attributes']['Components']
    for i in range(0,len(components_list)):
      if (components_list[i]['Name'] == 'PRODUCT_FRIENDLY_NAME_UQ'):
        fix_plist_version(components_list[i]);

  # 1 means that the value in 'build path' is an absolute path.
  installer['Settings']['Build Path Type'] = 1
  installer['Settings']['Build Path'] = 'GEARS_INSTALLER_OUT_DIR'
  installer.write(installer_file_name)

  # Fixup PRODUCT_VERSION_MAJOR & PRODUCT_VERSION_MINOR nodes which plistlib
  # outputs as <text> nodes, but must be <integer> nodes for iceberg to
  # correctly process the file.
  replace('<string>PRODUCT_VERSION_MAJOR</string>', 
          '<integer>PRODUCT_VERSION_MAJOR</integer>',
          installer_file_name);
  replace('<string>PRODUCT_VERSION_MINOR</string>', 
          '<integer>PRODUCT_VERSION_MINOR</integer>',
          installer_file_name);