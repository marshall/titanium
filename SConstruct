#!/usr/bin/env python

# common SConscripts
import os, re, sys, inspect, os.path as path

filename = inspect.currentframe().f_code.co_filename

OS = ''
os_define = ''
install_prefix = '/usr/local'
product_name = 'Titanium'

class BuildConfig(object): 
	def __init__(self):
		if not hasattr(os, 'uname') or self.matches('CYGWIN'):
			self.os = 'win32'
		elif self.matches('Darwin'):
			self.os = 'osx'
		elif self.matches('Linux'):
			self.os = 'linux'
		self.abstopdir = path.abspath('.')
		self.dir = '#build/%s' % self.os 
		self.krolldir = '#kroll/build/%s' % self.os
		self.absdir = path.abspath('build/%s' % self.os)
		self.krollabsdir = path.abspath('kroll/build/%s' % self.os)
		self.third_party = path.abspath('thirdparty/%s' % self.os)
	def matches(self, n): return bool(re.match(os.uname()[0], n))
	def is_linux(self): return self.os == 'linux'
	def is_osx(self): return self.os == 'osx'
	def is_win32(self): return self.os == 'win32'
build = BuildConfig()

build.include_dir = path.abspath(path.join('kroll', 'build', 'include'))
build.env = Environment(
    CPPDEFINES = {
                  'OS_' + build.os.upper(): 1,
                  '_INSTALL_PREFIX': install_prefix,
                  '_PRODUCT_NAME': product_name
                 },
    CPPPATH=['#.', build.include_dir],
    LIBPATH=[build.dir, build.krolldir],
		LIBS=['kroll']
)

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
	execfile('kroll/build/win32.py')

if build.is_linux() or build.is_osx():
    build.env.Append(CPPFLAGS=['-Wall', '-Werror','-fno-common'])

if build.is_osx():
	OSX_SDK = '/Developer/SDKs/MacOSX10.4u.sdk'
	OSX_UNIV_COMPILER = '-isysroot '+OSX_SDK+' -arch i386'
	OSX_UNIV_LINKER = '-isysroot '+OSX_SDK+' -syslibroot,'+OSX_SDK
	build.env.Append(CXXFLAGS=OSX_UNIV_COMPILER)
	build.env.Append(LDFLAGS=OSX_UNIV_LINKER)


Export ('build')
SConscript('modules/SConscript')
