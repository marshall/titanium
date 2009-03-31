#!/usr/bin/env python

# common SConscripts
import os, re, sys, inspect, os.path as path
from sets import Set
import subprocess, distutils.dir_util as dir_util

from kroll import BuildConfig
build = BuildConfig(
	PRODUCT_VERSION = '0.4',
	INSTALL_PREFIX = '/usr/local',
	PRODUCT_NAME = 'Titanium',
	GLOBAL_NS_VARNAME = 'Titanium',
	CONFIG_FILENAME = 'tiapp.xml',
	BUILD_DIR = path.abspath('build'),
	THIRD_PARTY_DIR = path.join(path.abspath('kroll'), 'thirdparty'),
	BOOT_RUNTIME_FLAG = '--runtime',
	BOOT_HOME_FLAG = '--start',
	BOOT_UPDATESITE_ENVNAME = 'TI_UPDATESITE'
)

build.titanium_source_dir = path.abspath('.')
build.kroll_source_dir = path.abspath('kroll')
build.kroll_third_party = build.third_party
build.kroll_include_dir = path.join(build.dir, 'include')
build.titanium_support_dir = path.join(build.titanium_source_dir, 'support', build.os)

build.sdk_build_dir = path.join(build.dir, 'titanium-sdk-' + build.version)
if build.is_osx(): build.sdk_build_dir = path.join(build.sdk_build_dir + '.app')

# This should only be used for accessing various
# scripts in the kroll build directory. All resources
# should instead be built to build.dir
build.kroll_build_dir = path.join(build.kroll_source_dir, 'build')

build.env.Append(CPPPATH=[
	build.titanium_source_dir,
	build.kroll_source_dir,
	build.kroll_include_dir
])

# debug build flags
if ARGUMENTS.get('debug', 0):
	build.env.Append(CPPDEFINES = ('DEBUG', 1))
	debug = 1
	if not build.is_win32():
		build.env.Append(CCFLAGS = ['-g'])  # debug
	else:
		build.env.Append(CCFLAGS = ['/Z7','/GR'])  # max debug, C++ RTTI
else:
	build.env.Append(CPPDEFINES = ('NDEBUG', 1 ))
	debug = 0
	if not build.is_win32():
		build.env.Append(CCFLAGS = ['-O9']) # max optimizations
	else:
		build.env.Append(CCFLAGS = ['/GR']) # C++ RTTI

# turn on special debug printouts for reference counting
if ARGUMENTS.get('debug_refcount', 0) == 1:
	build.env.Append(CPPDEFINES = ('DEBUG_REFCOUNT', 1))


if build.is_win32():
	execfile('kroll/site_scons/win32.py')
	build.env.Append(CCFLAGS=['/EHsc'])
	if build.debug:
		build.env.Append(CPPDEFINES=('WIN32_CONSOLE', 1))
	build.env.Append(LINKFLAGS=['/DEBUG', '/PDB:${TARGET}.pdb'])

	
Export('build')
targets = COMMAND_LINE_TARGETS
clean = 'clean' in targets or ARGUMENTS.get('clean', 0)
qclean = 'qclean' in targets or ARGUMENTS.get('qclean', 0)

if clean or qclean:
	print "Obliterating your build directory: %s" % build.dir
	if path.exists(build.dir):
		dir_util.remove_tree(build.dir)
	if not qclean: os.system('scons -c')
	Exit(0)

## Kroll *must not be required* for installation
SConscript('installation/SConscript')

# After Kroll builds, the environment will  link 
# against libkroll, so anything that should not be
# linked against libkroll should be above this point.
SConscript('kroll/SConscript', exports='debug')
SConscript('modules/SConscript')
SConscript('SConscript.dist')
SConscript('SConscript.docs')

run = ARGUMENTS.get('run', 0)
Export('run')
SConscript('apps/SConscript')
