#!/usr/bin/env python
#
# Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
# Released under Apache Public License V2.
#
# Titanium application packaging script
#
# Original author: Jeff Haynie 04/02/09
#
#
from optparse import OptionParser
from desktop_builder import DesktopBuilder
from desktop_packager import DesktopPackager
from mobile_builder import MobileBuilder
from mobile_packager import MobilePackager
import sys, os.path, platform, re, subprocess

VERSION = '0.1'
PLATFORMS = ['win32','osx','linux','iphone','android']

def log(options,msg):
	if options.verbose:
		print msg

def get_version_from_tiapp(appdir):
	f = os.path.join(appdir,'tiapp.xml')
	if not os.path.exists(f):
		print "Couldn't find tiapp.xml at: %s" % appdir
		sys.exit(1)
	xml = open(f).read()
	m = re.search('<version>(.*?)</version>',xml)
	return str(m.group(1)).strip()
	
def examine_manifest(appdir):
	f = os.path.join(appdir,'manifest')
	if not os.path.exists(f):
		print "Couldn't find manifest at: %s" % appdir
		sys.exit(1)
	lines = open(f).readlines()
	manifest = {}
	manifest['modules']={}
	for line in lines:
		if len(line)>0:
			tok = line.strip().split(':')
			if line[0:1] == '#':
				manifest[tok[0][1:].strip()]=tok[1].strip()
			else:
				manifest['modules'][tok[0].strip()]=tok[1].strip()
	return manifest
	
def find_titanium_base():
	p = get_platform()
	f = None
	
	if 'osx' in p:
		f = '/Library/Application Support/Titanium'
		if not os.path.exists(f):
			f = '~/Library/Application Support/Titanium'
	elif 'win32' in p:
		f = 'C:/ProgramData/Titanium'
		if not os.path.exists(f):
			pass
		pass
	elif 'linux' in p:
		f = os.path.expanduser('~/.titanium')
		if not os.path.exists(f):
			f = '/opt/titanium'
	return f	
	
def is_mobile(options):
	return options.platform == 'iphone' or options.platform == 'android'
	
def get_platform():
	if 'Darwin' in platform.platform():
		return 'osx'
	elif 'Windows' in platform.platform():
		return 'win32'
	elif 'Linux' in platform.platform():
		return 'linux'

def mobile_setup(options,appdir):
	return MobileBuilder(options,log)
	
def desktop_setup(options,appdir):
	# now resolve all the modules and the runtime by version
	depends = options.manifest['modules']
	
	# resolver for module paths
	module_paths = []

	runtime = depends['runtime']
	if os.path.exists(os.path.join(options.source,'runtime',options.platform,runtime)):
		options.runtime_dir = os.path.join(options.source,'runtime',options.platform,runtime)
	else:
		print("Couldn't determine your source distribution for packaging runtime version %s. Please specify." % runtime)
		sys.exit(1)
		
	options.runtime = runtime	

	for name in depends:
		if name == 'runtime':
			continue
		version = depends[name]
		entry = {'name':name,'version':version}
		if os.path.exists(os.path.join(appdir,'modules',name,version)):
			entry['path']=os.path.join(appdir,'modules',name,version)
			module_paths.append(entry)
		else:
			# if we're not network, we must find it to be bundled
			# otherwise we must fail
			if options.type != 'network':
				if os.path.exists(os.path.join(options.source,'modules',options.platform,name,version)):
					entry['path']=os.path.join(options.source,'modules',options.platform,name,version)
					module_paths.append(entry)
				else:
					print("Couldn't find required module: %s with version: %s" %(name,version))
					sys.exit(1)

	# map in the module paths	
	options.module_paths = module_paths
	
	# assign appdir
	options.appdir = appdir
	
	# convert this option
	if options.package == 'false' or options.package == 'no':
		options.package = False

	# try and find the assets directory
	if options.assets_dir == None:
		options.assets_dir = os.path.join(os.path.abspath(os.path.dirname(sys._getframe(0).f_code.co_filename)), options.platform)

	if not os.path.exists(options.assets_dir):
		print("Couldn't find assets directory at %s" % options.assets_dir)
		sys.exit(1)

	return DesktopBuilder(options,log)
			
def main(options,appdir):

	if options.platform == None:
		options.platform = get_platform()
		
	if not options.platform in PLATFORMS:
		print "Error: unsupported/unknown platform: %s" % options.platform
		print "Must be one of: %s" % str(PLATFORMS)
		sys.exit(1)

	options.mobile = is_mobile(options)

	log(options,'Packaging for target: %s' % options.platform)
	
	if options.destination == None:
		options.destination = os.path.dirname(sys.argv[0])

	if options.source == None:
		options.source = find_titanium_base()
		if options.source == None:
			print "no source directory specified for distribution files and couldn't find one"
			sys.exit(1)

	if not os.path.exists(options.source):
		print "no source directory found at %s" % options.source
		sys.exit(1)
	
	if not os.path.exists(options.destination):
		print('Invalid destination directory: %s' % options.destination)
		sys.exit(1)
	
	# read the manifest
	options.manifest = examine_manifest(appdir)

	# get the version from the tiapp.xml
	options.version = get_version_from_tiapp(appdir)
	
	# run the builders
	if options.mobile:
		builder = mobile_setup(options,appdir)
	else:
		builder = desktop_setup(options,appdir)
	
	# allow post-build scripts
	if os.path.exists("post_builder.py"):
		exec('from post_builder import PostBuilder')
		eval('PostBuilder(builder)')
	
	# run the packagers	
	if options.mobile:
		MobilePackager(builder)
	else:
		DesktopPackager(builder)
			
	log(options,"Packaging complete, location: %s"%os.path.abspath(options.destination))
	
	if not options.mobile and options.run:
		subprocess.call([options.executable])

if __name__ == '__main__':
	parser = OptionParser(usage="%prog [options] appdir", version="%prog " + VERSION)
	parser.add_option("-d","--dest",dest="destination",help="destination folder for output",metavar="FILE")	
	parser.add_option("-s","--src",dest="source",help="source folder which contains dist files",metavar="FILE")	
	parser.add_option("-v","--verbose",action="store_true",dest="verbose",default=False,help="turn on verbose logging")	
	parser.add_option("-o","--os",dest="platform",default=None,help="platform if different than %s" % get_platform())	
	parser.add_option("-t","--type",dest="type",default="network",help="package type: network or bundle")	
	parser.add_option("-a","--assets",dest="assets_dir",default=None,help="location of platform assets",metavar="FILE")	
	parser.add_option("-l","--license",dest="license_file",default=None,help="location of application license",metavar="FILE")	
	parser.add_option("-n","--noinstall",action="store_true",dest="no_install",default=False,help="don't include installer dialog in packaged app")	
	parser.add_option("-r","--run",action="store_true",dest="run",default=False,help="run the packaged app after building")	
	parser.add_option("-p","--package",dest="package",default=True,help="build the installation package")	
	(options, args) = parser.parse_args()
	if len(args) == 0:
		parser.print_help()
		sys.exit(1)
	appdir = os.path.join(args[0])
	if appdir == "":
		parser.print_help()
		sys.exit(1)
	if not os.path.exists(appdir):
		print "Couldn't find application directory at: %s" % appdir
		sys.exit(1)
	main(options,args[0])
	
	
