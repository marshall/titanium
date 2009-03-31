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

# This should only be used for accessing various
# scripts in the kroll build directory. All resources
# should instead be built to build.dir
build.kroll_build_dir = path.join(build.kroll_source_dir, 'build')

build.env.Append(CPPPATH=[
	build.titanium_source_dir,
	build.kroll_source_dir,
	build.kroll_include_dir
])

build.env.Append(LIBPATH=[build.dir])

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
package = 'package' in targets or ARGUMENTS.get('package', 0)
testapp = 'testapp' in targets or ARGUMENTS.get('testapp', 0)
testsuite = 'testsuite' in targets or ARGUMENTS.get('testsuite', 0)
clean = 'clean' in targets or ARGUMENTS.get('clean', 0)
qclean = 'qclean' in targets or ARGUMENTS.get('qclean', 0)
dist = 'dist' in targets or ARGUMENTS.get('dist', 0)
uploader = 'uploader' in targets or ARGUMENTS.get('uploader', 0)
run = 'run' in targets or ARGUMENTS.get('run', 0)
apicoverage = 'apicoverage' in targets or ARGUMENTS.get('apicoverage',0)
textmate = 'textmate' in targets or ARGUMENTS.get('textmate',0)

Export('run')

if clean or qclean:
	print "Obliterating your build directory: %s" % build.dir
	if path.exists(build.dir):
		dir_util.remove_tree(build.dir)
	if not qclean: os.system('scons -c')
	Exit(0)

# Linux can package and build at the same time now
if not(package) or build.is_linux():

	## Kroll *must not be required* for installation
	SConscript('installation/SConscript')

	SConscript('kroll/SConscript', exports='debug')

	# Kroll library is now built (hopefully)
	if not build.is_linux():
		build.env.Append(LIBS=['kroll']) 
	SConscript('modules/SConscript')

if package:
	print "Building packaging ..."
	SConscript('installation/runtime/SConscript')

if dist:
	print 'Building dist files...'
	dist = build.build_dist_files()

if testapp:
	print "Building testapp ..."
	SConscript('apps/testapp/SConscript')

if uploader:
	SConscript('apps/uploader/SConscript')

if testsuite:
	print 'building testsuite...'
	SConscript('apps/apivalidator/SConscript')
  
if apicoverage:
	print 'building API coverage report...'
	import apicoverage
	f = open(path.join(build.dir,'apicoverage.json'),'w')
	apicoverage.generate_api_coverage('.',f)

if textmate:
	print 'building Textmate bundle...'
	import textmate
	f = open(path.join(build.dir,'apicoverage.json'),'r')
	outdir = path.join(build.dir,'Titanium.tmbundle')
	if not os.path.exists(outdir):
		os.makedirs(outdir)
	textmate.generate_textmate_bundle(f,outdir)
		