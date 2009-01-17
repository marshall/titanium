#!/usr/bin/env python

# common SConscripts
import os, re, sys, inspect, os.path as path
import subprocess

sys.path.append(path.abspath('.')+'/kroll')
from build.common import BuildConfig

build = BuildConfig(
	INSTALL_PREFIX = '/usr/local',
	PRODUCT_NAME = 'Titanium',
	GLOBAL_NS_VARNAME = 'ti',
	CONFIG_FILENAME = 'tiapp.xml',
	BUILD_DIR = path.abspath('build'),
	THIRD_PARTY_DIR = path.join(path.abspath('kroll'), 'thirdparty')
)

build.titanium_source_dir = path.abspath('.')
build.kroll_source_dir = path.abspath('kroll')
build.kroll_third_party = build.third_party
build.kroll_include_dir = path.join(build.dir, 'include')

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

#
#if build.env.GetOption('clean'):
#	os.chdir(path.abspath('kroll'))
#	os.system('scons.bat -c PRODUCT_NAME=Titanium')
#	os.chdir(build.abstopdir)
#else:
#	os.chdir(path.abspath('kroll'))
#	os.system('scons.bat PRODUCT_NAME=Titanium')
#	os.chdir(build.abstopdir)

# debug build flags
if ARGUMENTS.get('debug', 0) == 1:
	build.env.Append(CPPDEFINES = {'DEBUG' : 1})
	if not build.is_win32():
		build.env.Append(CCFLAGS = ['-g'])
else:
	build.env.Append(CPPDEFINES = {'NDEBUG' : 1 })
	if not build.is_win32():
		build.env.Append(CCFLAGS = ['-O9'])

# turn on special debug printouts for reference counting
if ARGUMENTS.get('debug_refcount', 0) == 1:
	build.env.Append(CPPDEFINES = {'DEBUG_REFCOUNT': 1})


if build.is_win32():
	build.env.Append(CCFLAGS=['/EHsc'])
	build.env.Append(CPPDEFINES={'WIN32_CONSOLE': 1})
	build.env.Append(LINKFLAGS=['/DEBUG', '/PDB:${TARGET}.pdb'])

if build.is_linux() or build.is_osx():
    build.env.Append(CPPFLAGS=['-Wall', '-Werror','-fno-common','-fvisibility=hidden'])

if build.is_osx():
	OSX_SDK = '/Developer/SDKs/MacOSX10.4u.sdk'
	OSX_UNIV_COMPILER = '-isysroot '+OSX_SDK+' -arch i386'
	OSX_UNIV_LINKER = '-isysroot '+OSX_SDK+' -syslibroot,'+OSX_SDK
	build.env.Append(CXXFLAGS=OSX_UNIV_COMPILER)
	build.env.Append(LDFLAGS=OSX_UNIV_LINKER)


tiBuild = build
Export ('tiBuild')
Export ('build')

SConscript('kroll/SConscript')

# Kroll library is now built (hopefully)
build.env.Append(LIBS=['kroll']) 
SConscript('modules/SConscript')
SConscript('launcher/SConscript')
