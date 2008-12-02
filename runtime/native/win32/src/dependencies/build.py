import os
import sys
import shutil

target_dir = sys.argv[1]
titanium_version = '0.1.0'

home_path = 'C:\cygwin\home\Marshall'
#home_path = os.path.expanduser('~')

titanium_folder = os.path.join(home_path, '.appcelerator', 'releases', 'titanium', 'win32', titanium_version, 'pieces', 'titanium')
titanium_js = os.path.join(titanium_folder, 'titanium.js')
titanium_debug_js = os.path.join(titanium_folder, 'titanium-debug.js')

copy_dir = os.path.join(target_dir, 'Resources', 'titanium')
try:
	os.makedirs(copy_dir)
except OSError:
	pass


shutil.copy(titanium_debug_js, os.path.join(copy_dir, 'titanium.js'))
#shutil.copy(titanium_debug_js, copy_dir)
