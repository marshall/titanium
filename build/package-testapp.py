#!/usr/bin/env python

# this script creates build/<os>/titanium_testapp folder
# after running this script, you can execute the test app with <os>/titanium_testapp/titanium_testapp

import os, os.path as path, shutil, glob
import distutils.dir_util as dir_util
import sys

app_name = 'titanium_testapp'
version = '0.2'

runtime_libs = ['kroll', 'khost']
third_party = ['webkit', 'poco']
modules = [
    'api', 'javascript', 'foo', # 'ruby', 'python'
    'tiplatform', 'tiapp', 'tiui', #'tinetwork',
    'tigrowl', 'tifilesystem', 'timedia', 'tidesktop', 'tiprocess',
]

lib_prefix = ''

if sys.platform == 'win32':
	osname = 'win32'
	lib_ext = '.dll'
	exe_ext = '.exe'
	lib_dir = 'bin'
elif sys.platform == 'linux2':
	osname = 'linux'
	lib_ext = '.so'
	exe_ext = ''
	lib_prefix = 'lib'
	lib_dir = 'lib'
	third_party.append('libcurl')
	third_party.append('libicu')

top_dir = path.abspath('./../')
build_dir = path.join(top_dir, 'build')
build_dir = path.join(build_dir, osname)
third_party_dir = path.join(top_dir, 'kroll/thirdparty/' + osname);

app_dir = path.join(build_dir, app_name)
runtime_dir = path.join(app_dir, 'runtime');
modules_dir = path.join(app_dir, 'modules');


# delete current package if one is there
if path.isdir(app_dir):
    dir_util.remove_tree(app_dir)

# create directories for app
for d in [app_dir, runtime_dir, modules_dir]:
    os.makedirs(d)
    
# Gather all runtime third-party libraries
for lib in runtime_libs:
    f = path.join(build_dir, lib_prefix + lib + lib_ext)
    shutil.copy(f, runtime_dir)

for tp in third_party:
    lib_files_dir = path.join(third_party_dir, tp)
    lib_files_dir = path.join(lib_files_dir, lib_dir)
    lib_files_dir = path.join(lib_files_dir, '*')
    for d in glob.glob(lib_files_dir):
        shutil.copy(d, runtime_dir)
        
# Gather all module libs
for m in modules:
    mlib = lib_prefix + m + 'module' + lib_ext
    mlib = path.join(build_dir, mlib)
    out_dir = '%s/%s' % (modules_dir, m)
    os.makedirs(out_dir)
    shutil.copy(mlib, out_dir)
    
# create executable file
kboot_file = path.join(build_dir, 'kboot' + exe_ext)
shutil.copy(kboot_file, path.join(app_dir, app_name + exe_ext))

# copy test app resources
app_src = path.join(top_dir, 'build')
app_src = path.join(app_src, 'testapp')
dir_util.copy_tree(app_src, app_dir, preserve_symlinks=True)
