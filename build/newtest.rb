#!/usr/bin/env ruby
#
# this is a simple test builder for packaging a new app
#
#
require 'rubygems'
require 'zip/zip'
require 'fileutils'

FileUtils.rm_rf "/Library/Application Support/Titanium"

NAME='newtest'

SRCDIR = File.dirname(__FILE__)
BUILDDIR = File.join(SRCDIR,'osx')
TOPDIR = File.expand_path(File.join(BUILDDIR,'/../../'))
OUTDIR = File.join(BUILDDIR,NAME)
SUPPORTDIR = File.join(TOPDIR,'support','osx')
FileUtils.mkdir_p OUTDIR

APPDIR = File.join(OUTDIR,"#{NAME}.app")

FileUtils.mkdir_p APPDIR

CONTENTS = File.join(APPDIR,'Contents')
RESOURCES = File.join(CONTENTS,'Resources')
MACOS = File.join(CONTENTS,'MacOS')
INSTALLER = File.join(CONTENTS,'installer')
NIBS = File.join(RESOURCES,'English.lproj')

FileUtils.mkdir_p [RESOURCES,MACOS,INSTALLER,NIBS]


FileUtils.cp File.join(BUILDDIR,'kboot'),File.join(MACOS,NAME)


plist = File.read(File.join(SUPPORTDIR,'Info.plist'))
plist.gsub! 'APPNAME',"#{NAME}.app"
plist.gsub! 'APPICON','titanium.icns'
plist.gsub! 'APPID','com.test'
plist.gsub! 'APPNIB','MainMenu'
plistf = File.open(File.join(CONTENTS,'Info.plist'),'w')
plistf.puts plist
plistf.close


manifestf = File.open(File.join(CONTENTS,'manifest'),'w')
manifestf.puts <<-END
appname:#{NAME}
appid:0123456789
runtime:0.1
END

# PLUGINS =
#   %w(api javascript foo foojs foorb foopy ruby python
#     tiplatform tiapp tiui tinetwork tifilesystem 
#     timedia tidesktop tigrowl tidatabase tiprocess)

PLUGINS =
      %w(api tiapp tiui)

PLUGINS.each do |p|
  manifestf.puts "#{p}:0.1"
end

manifestf.close

# create installer app
installapp = File.join(INSTALLER,'Installer App.app','Contents')
lproj = File.join(installapp,'Resources','English.lproj')
tinstall = File.join(BUILDDIR,'tinstaller')

FileUtils.mkdir_p(File.join(installapp,'MacOS'))
FileUtils.mkdir_p(File.join(installapp,'Resources'))
FileUtils.mkdir_p(lproj)

FileUtils.cp(File.join(BUILDDIR,'installer'),File.join(installapp,'MacOS','Installer App'))
FileUtils.cp(File.join(tinstall,'Info.plist'),installapp)
FileUtils.cp(File.join(tinstall,'MainMenu.nib'),lproj)

FileUtils.cp(File.join(SUPPORTDIR,'titanium.icns'),File.join(installapp,'Resources'))

website = File.join(ENV['HOME'],'Sites','titanium')
FileUtils.mkdir_p website unless File.exists? website
Dir["#{BUILDDIR}/*.zip"].each do |file|
  FileUtils.cp file,website
end

FileUtils.cp_r(SRCDIR+'/testapp/.', CONTENTS)
