#!/usr/bin/env py
##
## Common hudson module -- provides a unified location for hudson scripts args, directories, platform info, etc
##

import os, sys, re, platform, getopt
import subprocess, hashlib, version

class Platform:
	def __init__(self):
		if 'Darwin' in platform.platform():
			mac_ver = platform.mac_ver()
			version = mac_ver[0][0:-2]
			self.id = 'MacOSX-%s-Universal' % version
			self.runtime_ext = "dmg"
		elif 'Windows' in platform.platform():
			self.id = 'Windows'
			self.runtime_ext = "exe"
		elif 'Linux' in platform.platform():
			if os.path.exists('/etc/lsb-release'):
				self.parse_lsb_release()
			dist = platform.dist()
			if dist[0] is 'fedora':
				self.id = 'Fedora-%s-%s' % (dist[1], self.unix_arch())
			self.runtime_ext = "bin"

	def unix_arch(self):
		arch = os.uname()[4]
		if arch is 'x86_64':
			return '64bit'
		return '32bit'

	def parse_lsb_release(self):
		lsb_release = open('/etc/lsb-release', 'r')
		lines = lsb_release.readlines()
		distro_id = None
		distro_release = None
		for line in lines:
			if 'DISTRIBUTION_ID' in line:
				m = re.match('DISTRIBUTION_ID=(.+)', line)
				distro_id = m.group(1)
			elif 'DISTRIBUTION_RELEASE' in line:
				m = re.match('DISTRIBUTION_RELEASE=(.+)', line)
				distro_release = m.group(1)
		self.id = '%s-%s-%s' % (distro_id, distro_release, self.unix_arch())

	def is_linux(self): return 'Linux' in platform.platform()
	def is_osx(self): return 'Darwin' in platform.platform()
	def is_win32(self): return 'Windows' in platform.platform()
	def shortname(self):
		if self.is_linux(): return "linux"
		elif self.is_osx(): return "osx"
		elif self.is_win32(): return "win32"

class HudsonScriptArgs:
	def usage(self):
		print sys.argv[0]
		print "--help: print this usage"
		print "--mode=(nightly|integration|release): build mode (default: nightly)"
		print "--noclean: don't do a clean build (for debugging)"
		print "--version=X set the version number for a release build"
		sys.exit(1)
			
	def __init__(self,hudson_config):
		try:
			opts, args = getopt.getopt(sys.argv[1:], "hm:v:", ["help", "mode", "noclean","version="])
		except getopt.GetoptError, err:
			print str(err)
			self.usage()
		
		self.mode = "nightly"
		self.version = hudson_config.get_nightly_version()
		self.clean = True
		for option, value in opts:
			if option in ("-h", "--help"):
				self.usage()
			elif option in ("-m", "--mode"):
				self.mode = value
			elif option in ("--noclean"):
				self.clean = False
			elif option in ("-v", "--version"):
				self.version = value
	
	def get_mode(self): return self.mode
	def get_version(self): return self.version
	def is_clean(self): return self.clean
	
	def is_release_mode(self): return self.mode is 'release'
	def is_integration_mode(self): return self.mode is 'integration'
	def is_nightly_mode(self): return self.mode is 'nightly'
	
class HudsonConfig:
	def __init__(self):
		self.platform = Platform()
		self.args = HudsonScriptArgs(self)
		self.workspace_dir = os.getcwd()
		if os.environ.has_key("WORKSPACE"):
			self.workspace_dir = os.environ["WORKSPACE"]
		self.kroll_dir = self.get_workspace_dir("kroll")
		self.hudson_dir = os.getcwd()
		self.build_dir = self.get_workspace_dir('build', self.platform.shortname())
		self.distribution_dir = os.path.abspath(self.get_workspace_dir("distribution"))
		if os.environ.has_key("DISTRIBUTION"):
			self.distribution_dir = os.environ["DISTRIBUTION"]
		self.titanium_dist_dir = os.path.join(self.distribution_dir, "titanium")
		self.results_dir = os.path.join(self.titanium_dist_dir, self.args.get_mode(), self.args.get_version())
		self.package_dir = os.path.join(self.results_dir, self.get_platform_id())
		try:
			os.makedirs(self.package_dir)
		except:
			pass

	def call(self, cmd):
		try:
			retcode = subprocess.call(cmd, shell=True)
			if retcode < 0:
				print >>sys.stderr, "Child was terminated by signal", -retcode
			else:
				print >>sys.stderr, "Child returned", retcode
		except OSError, e:
			print >>sys.stderr, "Execution failed:", e


	def get_platform_id(self):
		return self.platform.id

	def get_version(self):
		return self.args.get_version()
	
	def get_nightly_version(self):
		v = str(version.MAJOR) + "." + str(version.MINOR) + "." + str(version.MICRO) + "."
		v += str(self.get_git_revision())
		return v
	
	def get_git_revision(self):
		log_lines = subprocess.Popen(['git', 'log', '--pretty=oneline'], stdout=subprocess.PIPE).communicate()[0]
		return len(log_lines.splitlines())
	
	def get_workspace_dir(self, *args):
		return os.path.join(self.workspace_dir, *args)
	
	def get_runtime_extension(self):
		return self.platform.runtime_ext
	
	def get_runtime_binary(self):
		return "titanium_runtime_%s.%s" % (self.get_version(), self.get_runtime_extension())
	
	def get_sha1(self, path):
		file = open(path, "rb")
		# read in chunks so we don't overfill memory
		buf = file.read(512)
		sha1 = hashlib.sha1()
		while buf is not '':
			sha1.update(buf)
			buf = file.read(512)
		file.close()
		return sha1.hexdigest()
		