#!/usr/bin/env python

import os, sys, re
import getopt, subprocess
import shutil, distutils.dir_util as dir_util
import hudson

hudson_config = hudson.HudsonConfig()

if hudson_config.args.is_clean():
	try:
		dir_util.remove_tree(hudson_config.build_dir)
	except:
		pass

build_command = "scons -D"
if not hudson_config.args.is_release_mode():
	build_command += " debug=1"

package_command = build_command + " package=1"
build_testsuite_command = build_command + " testsuite=1"

print "Building Titanium %s on %s..." % (hudson_config.get_version(), hudson_config.get_platform_id())

hudson_config.call(build_command)
hudson_config.call(package_command)

titanium_runtime = "titanium_runtime.%s" % hudson_config.get_runtime_extension()
runtime_target = hudson_config.get_runtime_binary()
shutil.copy(os.path.join(hudson_config.build_dir, titanium_runtime), os.path.join(hudson_config.package_dir, runtime_target))

# TODO: enable testsuite
#os.system(build_testsuite_command)
#
#if hudson_config.platform.is_linux():
#	os.system(os.path.join(build_dir, "titanium_testsuite", "titanium_testsuite"))
#elif hudson_config.platform.is_win32():
#	os.system(os.path.join(build_dir, "titanium_testsuite", "titanium_testsuite.exe"))
#elif hudson_config.platform.is_osx():
#	os.system(os.path.join(build_dir, "titanium_testsuite", "Contents", "MacOS", "titanium_testsuite"))