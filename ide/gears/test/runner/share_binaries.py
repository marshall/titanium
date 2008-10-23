import os
import sys
import shutil

# Copies files to share directory
# Used by build machine to copy compiled binaries to share directory.
 
# Usage:
# python share_binaries.py source_directory absolute_dir_to_share output_dir
# e.g.
# "python share_binaries.py bin-dbg\installers \\filer\foo 43/win32/installers"
# and it will create
# This will create "\\filer\foo\43\win32\installers" folder with 
# installers copied from build.

def assertPathAbsolute(directory):
  if not os.path.isabs(directory):
    raise "Provide absolute path for the directory instead of %s" % directory

def joinPaths(parent, child): 
  return parent + os.sep + child

def makedirs(starting_dir, directory_structure):
  """ Create a directory tree based on directory_structure.

  Alternative to os.makedirs, however continues creating dir tree structure 
  if parent dirs are already created.
  """
  assertPathAbsolute(starting_dir)
  created_dirs = [starting_dir]
  directories = directory_structure.split('/')
  for directory in directories:
    created_dirs.append(directory)
    if not os.path.exists(reduce(joinPaths, created_dirs)):
      os.mkdir(reduce(joinPaths, created_dirs))
  return reduce(joinPaths, created_dirs)

def copytree(src, dst, symlinks=0):
  """ Copy a directory tree to dst location.
  
  Alternative to shutil.copytree, however does not create the root directory.
  """
  names = os.listdir(src)
  for name in names:
      srcname = os.path.join(src, name)
      dstname = os.path.join(dst, name)
      try:
          if symlinks and os.path.islink(srcname):
              linkto = os.readlink(srcname)
              os.symlink(linkto, dstname)
          elif os.path.isdir(srcname):
              shutil.copytree(srcname, dstname)
          else:
              shutil.copy2(srcname, dstname)
      except (IOError, os.error), why:
          print "Can't copy %s to %s: %s" % (`srcname`, `dstname`, str(why))

if __name__ == '__main__':
  src_dir = sys.argv[1]
  directory_share = sys.argv[2]
  destination_dir = sys.argv[3]

  dest_dir = makedirs(directory_share, destination_dir)
  copytree(src_dir, dest_dir)
