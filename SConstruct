#!/usr/bin/env python

# common SConscripts
import os, re, sys, inspect, os.path as path
import subprocess

install_prefix = '/usr/local'
product_name = 'Titanium'
global_variable_name = 'ti'
config_filename = 'tiapp.xml'

vars = Variables()
vars.Add('PRODUCT_NAME', 'The underlying product name that Kroll will display (default: "Kroll")', product_name)
vars.Add('INSTALL_PREFIX', 'The install prefix of binaries in the system (default: /usr/local)', install_prefix)
vars.Add('GLOBAL_NS_VARNAME','The name of the Kroll global variable', global_variable_name)
vars.Add('CONFIG_FILENAME','The name of the Kroll config file', config_filename)

class BuildConfig(object): 
	def __init__(self):
		if not hasattr(os, 'uname') or self.matches('CYGWIN'):
			self.os = 'win32'
		elif self.matches('Darwin'):
			self.os = 'osx'
		elif self.matches('Linux'):
			self.os = 'linux'

	def matches(self, n): return bool(re.match(os.uname()[0], n))
	def is_linux(self): return self.os == 'linux'
	def is_osx(self): return self.os == 'osx'
	def is_win32(self): return self.os == 'win32'

build = BuildConfig()
build.dir = path.abspath('build/' + build.os)
build.titanium_source_dir = path.abspath('.')
build.kroll_source_dir = path.abspath('kroll')
build.third_party = path.join(build.kroll_source_dir, 'thirdparty', build.os)
build.kroll_third_party = build.third_party
build.kroll_include_dir = path.join(build.dir, 'include')

# This should only be used for accessing various
# scripts in the kroll build directory. All resources
# should instead be built to build.dir
build.kroll_build_dir = path.join(build.kroll_source_dir, 'build')

build.env = Environment(variables=vars)
build.env.Append(CPPDEFINES = {
	'OS_' + build.os.upper(): 1,
	'_INSTALL_PREFIX': '${INSTALL_PREFIX}',
	'_PRODUCT_NAME': '${PRODUCT_NAME}',
	'_GLOBAL_NS_VARNAME': '${GLOBAL_NS_VARNAME}',
	'_CONFIG_FILENAME' : '${CONFIG_FILENAME}'
	})

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
