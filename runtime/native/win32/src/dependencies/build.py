# Copyright (c) 2008 Appcelerator, Inc
# Licensed under Apache Public License 2.0
#
# copy our html files and titanium.js / gears into the build directory for testing
#

import os
import sys
import shutil

def mkdir(x):
	try:
		os.makedirs(x)
	except OSError:
		pass

target_dir = sys.argv[1]
titanium_version = '0.1.0'

home_path = 'C:\cygwin\home\Marshall'
#home_path = os.path.expanduser('~')

pieces_folder = os.path.join(home_path, '.appcelerator', 'releases', 'titanium', 'win32', titanium_version, 'pieces')
titanium_folder = os.path.join(pieces_folder, 'titanium')
gears_folder = os.path.join(pieces_folder, 'gears')

gears_dll = os.path.join(gears_folder, 'gears_titanium.dll')
titanium_js = os.path.join(titanium_folder, 'titanium.js')
titanium_debug_js = os.path.join(titanium_folder, 'titanium-debug.js')

copy_dir = os.path.join(target_dir, 'Resources', 'titanium')
plugins_dir = os.path.join(target_dir, 'plugins')
mkdir(copy_dir)
mkdir(plugins_dir)

shutil.copy(gears_dll, os.path.join(plugins_dir, 'npgears_titanium.dll'))
os.chmod(os.path.join(plugins_dir, 'npgears_titanium.dll'), os.S_IEXEC)
shutil.copy(titanium_debug_js, os.path.join(copy_dir, 'titanium.js'))
#shutil.copy(titanium_debug_js, copy_dir)
