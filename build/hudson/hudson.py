#!/usr/bin/env py
##
## Hudson build script -- mostly os / platform configuration
##

import os, re, platform
import subprocess

class Platform:
	def __init__(self):
		if 'Darwin' in platform.platform():
			mac_ver = platform.mac_ver()
			version = mac_ver[0][0:-2]
			self.id = 'MacOSX-%s-Universal' % version
		elif 'Windows' in platform.platform():
			self.id = 'Windows'
		elif 'Linux' in platform.platform():
			if os.path.exists('/etc/lsb-release'):
				self.parse_lsb_release()
			dist = platform.dist()
			if dist[0] is 'fedora':
				self.id = 'Fedora-%s-%s' % (dist[1], self.unix_arch())

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

class HudsonConfig:
	def __init__(self):
		self.platform = Platform()

	def get_platform_id(self):
		return self.platform.id

	def get_git_revision(self):
		log_lines = subprocess.Popen(['git', 'log', '--pretty=oneline'], stdout=subprocess.PIPE).communicate()[0]
		return len(log_lines.splitlines())
