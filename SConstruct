#!/usr/bin/env python

# common SConscripts
import os, re, sys, inspect, os.path as path
import subprocess

from kroll import BuildConfig
build = BuildConfig(
	PRODUCT_VERSION = '0.2',
	INSTALL_PREFIX = '/usr/local',
	PRODUCT_NAME = 'Titanium',
	GLOBAL_NS_VARNAME = 'Titanium',
	CONFIG_FILENAME = 'tiapp.xml',
	BUILD_DIR = path.abspath('build'),
	THIRD_PARTY_DIR = path.join(path.abspath('kroll'), 'thirdparty'),
	BOOT_RUNTIME_FLAG = '--runtime',
	BOOT_HOME_FLAG = '--start',
	BOOT_UPDATESITE_ENVNAME = 'TI_UPDATESITE',
	BOOT_UPDATESITE_URL = 'http://updatesite.titaniumapp.com'
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

if build.is_linux() and build.arch == '64':
    build.env.Append(CPPFLAGS=['-m64', '-Wall', '-Werror','-fno-common','-fvisibility=hidden'])
    build.env.Append(LINKFLAGS=['-m64'])

elif build.is_linux() or build.is_osx():
    build.env.Append(CPPFLAGS=['-m32', '-Wall', '-Werror','-fno-common','-fvisibility=hidden'])
    build.env.Append(LINKFLAGS=['-m32'])

if build.is_osx():
	OSX_SDK = '/Developer/SDKs/MacOSX10.5.sdk'
#	OSX_SDK = '/Developer/SDKs/MacOSX10.4u.sdk'
#	OSX_UNIV_LINKER = '-isysroot '+OSX_SDK+' -syslibroot,'+OSX_SDK+' -arch i386 -mmacosx-version-min=10.4 -lstdc++'
	OSX_UNIV_LINKER = '-isysroot '+OSX_SDK+' -syslibroot,'+OSX_SDK+' -arch i386 -arch ppc -mmacosx-version-min=10.4 -lstdc++'
	build.env.Append(CXXFLAGS=['-isysroot',OSX_SDK,'-arch','i386','-mmacosx-version-min=10.5','-x','objective-c++'])
#	build.env.Append(CPPFLAGS=['-isysroot',OSX_SDK,'-mmacosx-version-min=10.4','-x','objective-c++'])
	build.env.Append(CPPFLAGS=['-arch','i386'])
	build.env.Append(CPPFLAGS=['-arch','ppc'])
	build.env.Append(LINKFLAGS=OSX_UNIV_LINKER)
	build.env.Append(FRAMEWORKS=['Foundation'])
Export('build')

# Linux can package and build at the same time now
if not(ARGUMENTS.get('package',0)) or build.is_linux():

	## Kroll *must not be required* for installation
	SConscript('installation/SConscript')

	SConscript('kroll/SConscript', exports='debug')

	# Kroll library is now built (hopefully)
	if not build.is_linux():
		build.env.Append(LIBS=['kroll']) 
	SConscript('modules/SConscript')

if ARGUMENTS.get('package',0):
	print "building packaging ..."
	SConscript('installation/runtime/SConscript')

