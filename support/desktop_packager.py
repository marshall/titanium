# Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
# Released under Apache Public License V2.
#
# Titanium application desktop Packager class
#
# Original author: Jeff Haynie 04/02/09
#
#
import os, shutil, distutils.dir_util as dir_util

class DesktopPackager(object):
	def __init__(self, builder):
		self.options = builder.options
		self.log = builder.log
		
		self.log("Executing desktop packager")
		
		if self.options.package:
			if self.options.platform == 'osx':
				self.package = self.create_dmg(builder)
			elif builder.options.platform == 'linux':
				pass
			elif builder.options.platform == 'win32':
				pass
		
	def create_dmg(self,builder):	

		dmg = os.path.join(self.options.destination, builder.appname)
		temp_dmg = dmg+'_'

		volname = "/Volumes/%s" % builder.appname

		try:
			builder.invoke("hdiutil detach \"%s\" 2>/dev/null" % volname)
		except OSError:
			True

		try:
			if os.path.exists(volname):
				os.unlink(volname)
		except OSError:
			True

		for d in [temp_dmg,dmg]:
			n = d+'.dmg'
			if os.path.isfile(n):
				os.remove(n)

		self.log("DMG Source: %s" % builder.base_dir)
		self.log("Temp DMG: %s.dmg" % temp_dmg)
		self.log("Building: %s.dmg" % dmg)

		# now run the DMG packager
		builder.invoke("hdiutil create -srcfolder \"%s\" -scrub -volname \"%s\" -fs HFS+ -fsargs \"-c c=64,a=16,e=16\" -format UDRW \"%s\"" % (builder.base_dir,builder.appname,temp_dmg))
		builder.invoke("hdiutil attach -readwrite -noverify -noautoopen \"%s.dmg\"" % temp_dmg)
		builder.invoke("bless --folder \"%s\" --openfolder \"%s\"" % (volname,volname))
		builder.invoke("ditto \"%s/installer_DS_Store\" \"%s/.DS_Store\"" % (self.options.assets_dir,volname))
		builder.invoke("ditto \"%s/background.jpg\" \"%s/background.jpg\"" % (self.options.assets_dir,volname))
		builder.invoke("ditto \"%s/titanium.icns\" \"%s/.VolumeIcon.icns\"" % (self.options.assets_dir,volname))
		builder.invoke("/Developer/Tools/SetFile -a C \"%s\"" % volname)
		builder.invoke("/Developer/Tools/SetFile -a V \"%s/background.jpg\"" % volname)
		builder.invoke("hdiutil detach \"%s\"" % volname)
		builder.invoke("hdiutil convert \"%s.dmg\" -format UDBZ -imagekey zlib-level=9 -o \"%s.dmg\"" % (temp_dmg,dmg))
		os.remove(temp_dmg+'.dmg')

		self.log("osx packager created: \"%s.dmg\"" % dmg)

		try:
			if os.path.exists(volname):
				os.unlink(volname)
		except OSError:
			True

		return '%s.dmg' % dmg
