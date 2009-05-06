# Copyright (c) 2009 Appcelerator, Inc. All Rights Reserved.
# Released under Apache Public License V2.
#
# Titanium application desktop Packager class
#
# Original author: Jeff Haynie 04/02/09
#
#
import os, shutil, distutils.dir_util as dir_util, zipfile, tarfile

class DesktopPackager(object):
	def __init__(self, builder):
		self.options = builder.options
		self.log = builder.log
		
		self.log("Executing desktop packager")
		
		if self.options.package:
			if self.options.platform == 'osx':
				self.package = self.create_dmg(builder)
			elif builder.options.platform == 'linux':
				self.package = self.create_sea(builder)
			elif builder.options.platform == 'win32':
				self.package = self.create_zip(builder)

	def create_zip(self, builder):
		extractor = os.path.join(self.options.assets_dir, 'self_extractor.exe')
		exe = os.path.join(self.options.destination,builder.options.executable)
		shutil.copy(extractor,exe)
		builder.log("making win32 binary at %s, this will take a sec..." % exe)
		zf = zipfile.ZipFile(exe, 'a', zipfile.ZIP_DEFLATED)

		# add the shitty ass MSVCRT crap
		for f in glob.glob(os.path.join(self.options.assets_dir,'Microsoft.VC80.CRT')+'/*'):
			zf.write(f,'Microsoft.VC80.CRT/'+os.path.basename(f))

		kboot = os.path.join(builder.options.runtime_dir, 'template', 'kboot.exe')
		zf.write(kboot,'template/kboot.exe')

		for walk in os.walk(builder.base_dir):
			for file in walk[2]:
				file = os.path.join(walk[0], file)
				arcname = file.replace(builder.base_dir + '/', "")
				zf.write(file, arcname)
		zf.close()
		return exe

	def walk_dir(self, dir, callback):
		files = os.walk(dir)
		for walk in files:
			for file in walk[2]:
				callback(os.path.join(walk[0], file))
				
	def create_sea(self, builder):
		outtarfile = os.path.join(builder.options.destination, builder.appname + '.tgz')
		tar = tarfile.open(outtarfile, 'w:gz')
		def tarcb(f):
			arcname = f.replace(builder.base_dir + os.sep, "")
			tar.add(f, arcname)
		self.walk_dir(builder.base_dir, tarcb)
		tar.close()

		outfile = os.path.join(builder.options.destination, builder.appname + '.bin')
		out = open(outfile, 'wb')
		extractor = open(os.path.join(builder.options.assets_dir, 'self_extracting.sh'), 'r').read()

		sane_name = builder.appname.replace("\"", "\\\"")
		extractor = extractor.replace('APPNAME', sane_name)
		out.write(extractor)
		out.write(open(outtarfile, 'rb').read())
		out.close()

		return outfile
		
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
