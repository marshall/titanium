#!/usr/bin/env python

import os, sys, re
import getopt, subprocess
import shutil, distutils.dir_util as dir_util
import hudson, version

hudson_config = hudson.HudsonConfig()

workspace_dir = os.environ["WORKSPACE"]
kroll_dir = os.path.join(workspace_dir, "kroll")
hudson_dir = os.getcwd()
build_dir = os.path.join(workspace_dir, 'build', hudson_config.platform.shortname())
package_dir = os.path.join(workspace_dir, 'distribution', hudson_config.get_platform_id())

try:
	os.makedirs(package_dir)
except:
	pass

def usage():
	print "--help: print this usage"
	print "--release: build in release mode (without debugging symbols)"
	print "--version=X set the version number for a release build"
	sys.exit(1)

try:
	opts, args = getopt.getopt(sys.argv[1:], "hrv:", ["help", "release", "version="])
except getopt.GetoptError, err:
	print str(err)
	usage()

release = False
version = str(version.MAJOR) + "." + str(version.MINOR) + "." + str(version.MICRO) + "."
version += str(hudson_config.get_git_revision())

for option, value in opts:
	if option in ("-h", "--help"):
		usage()
	elif option in ("-r", "--release"):
		release = True
	elif option in ("-v", "--version"):
		version = value


dir_util.remove_tree(build_dir)

build_command = "scons -D"
if not release:
	build_command += " debug=1"

package_command = build_command + " package=1"
build_testsuite_command = build_command + " testsuite=1"

os.system(build_command)
os.system(package_command)

titanium_runtime = None
if hudson_config.platform.is_linux():
	titanium_runtime = "titanium_runtime.bin"
elif hudson_config.platform.is_win32():
	titanium_runtime = "titanium_runtime.exe"
elif hudson_config.platform.is_osx():
	titanium_runtime = "titanium_runtime.dmg"

runtime_target = titanium_runtime.replace("titanium_runtime.", "titanium_runtime_%s." % version)
shutil.copy(os.path.join(build_dir, titanium_runtime), os.path.join(package_dir, runtime_target))

# TODO: enable testsuite
#os.system(build_testsuite_command)
#
#if hudson_config.platform.is_linux():
#	os.system(os.path.join(build_dir, "titanium_testsuite", "titanium_testsuite"))
#elif hudson_config.platform.is_win32():
#	os.system(os.path.join(build_dir, "titanium_testsuite", "titanium_testsuite.exe"))
#elif hudson_config.platform.is_osx():
#	os.system(os.path.join(build_dir, "titanium_testsuite", "Contents", "MacOS", "titanium_testsuite"))