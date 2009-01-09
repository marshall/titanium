#!/usr/bin/env ruby
#
# this is a simple test builder for packaging a new app
#
#
require 'rubygems'
require 'zip/zip'
require 'fileutils'

VER = 0.1
PLUGINS = %w(api foo ruby python fooruby foopy tiapp tiwindow)
NAME = 'test'

if RUBY_PLATFORM=~/darwin/
  OS='osx'
elsif RUBY_PLATFORM=~/linux/
  OS='linux'
else
  OS='win32'
end

outdir = File.join(File.expand_path(File.dirname(__FILE__)), OS)
rootdir = File.join(outdir, NAME + (OS=='osx' ? '.app':'' ))
appdir = OS=='osx' ? File.join(rootdir,'Contents') : rootdir
installdir = File.join(rootdir,'installer');
ext = OS=='win32' ? '.exe' : ''
thirdparty = File.join(File.expand_path(File.dirname(__FILE__)+'/../kroll/thirdparty'), OS)

FileUtils.rm_rf [installdir,appdir,rootdir]
FileUtils.mkdir_p [outdir,appdir,installdir]


if OS=='osx'
	FileUtils.rm_rf "~/Library/Application Support/Titanium"
	FileUtils.rm_rf "/Library/Application Support/Titanium"
end
if OS=='linux'
	FileUtils.rm_rf "~/Titanium"
end

def installer?(name)
  if name=~/(kkernel|kroll|khost)/
	  return true if OS!='win32'
	  return true if OS=='win32' and (name.index('.exe') or name.index('.dll'))
  end
  false
end

case OS
when 'osx'
	EXT='.dylib'
when 'linux'
	EXT='.so'
when 'win32'
	EXT='.dll'
end



runtime_zip = File.join(installdir,"runtime-#{OS}-#{VER}.zip")
FileUtils.rm_rf runtime_zip

Zip::ZipFile.open(runtime_zip, Zip::ZipFile::CREATE) do |zipfile|
  Dir["#{outdir}/**"].each do |f|
    name = f.gsub(outdir+'/','')
    zipfile.add name,f if installer?(name)
  end
  Dir["#{outdir}/*.nib"].each do |f|
    name = f.gsub(outdir+'/','')
    zipfile.add "Resources/English.lproj/#{name}",f
  end
  path = "#{thirdparty}/webkit" 
  Dir["#{path}/**/**"].each do |f|
    name = f.gsub(path+'/','')
    zipfile.add name,f unless (name=~/\.h$/ or name=~/\.defs$/)
  end
end
FileUtils.cp File.join(outdir,'kinstall'+ext), File.join(installdir,'kinstall'+ext)

PLUGINS.each do |plugin|
  plugin_zip = File.join(installdir,"module-#{plugin}-0.1.zip")
  FileUtils.rm_rf plugin_zip

  Zip::ZipFile.open(plugin_zip, Zip::ZipFile::CREATE) do |zipfile|
    Dir["#{outdir}/**"].each do |f|
      name = f.gsub(outdir+'/','')
		  next if File.extname(name)=='.o'
		  next if File.extname(name)=='.rb' and plugin == 'ruby'
		  next if File.extname(name)=='.py' and plugin == 'python'
      zipfile.add name,f if name.index(plugin+'module')
    end
  end
end

if OS == 'osx'
  bindir = File.join(appdir,'MacOS')
  FileUtils.mkdir_p File.join(appdir,'Resources')
else
  bindir = appdir
end

FileUtils.mkdir_p bindir
FileUtils.cp File.join(outdir,'kboot'+ext), File.join(bindir,NAME+ext)
FileUtils.chmod 0555,File.join(bindir,NAME+ext)

manifest = File.open File.join(rootdir,'manifest'),'w'
manifest.puts "runtime: #{VER}"
PLUGINS.each do |plugin|
  manifest.puts "#{plugin}: 0.1"
end
manifest.close

tiappxml = File.open(File.join(appdir,'tiapp.xml'),'w')
tiappxml.puts <<-END
<?xml version="1.0" encoding="UTF-8"?>
<ti:app xmlns:ti="http://ti.appcelerator.org" xmlns:appc="http://www.appcelerator.org">
   <id>com.titaniumapp.test</id>
   <name>Titanium Test App</name>
   <version>0.1</version>
   <window>
      <id>initial</id>
      <title>test</title>
      <url>app://index.html</url>
      <width>700</width>
      <height>620</height>
   </window>
</ti:app>
END
tiappxml.close


if OS=='osx'
  plist = File.open File.join(appdir,'Info.plist'),'w'
  plist.puts <<-END
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
  <dict>
  	<key>CFBundleExecutable</key>
  	<string>#{NAME}</string>
  	<key>CFBundleGetInfoString</key>
  	<string>#{NAME}</string>
  	<key>CFBundleIconFile</key>
  	<string>titanium.icns</string>
  	<key>CFBundleIdentifier</key>
  	<string>com.titaniumapp</string>
  	<key>CFBundleInfoDictionaryVersion</key>
  	<string>#{VER}</string>
  	<key>CFBundleName</key>
  	<string>#{NAME}</string>
  	<key>CFBundlePackageType</key>
  	<string>APPL</string>
  	<key>CFBundleShortVersionString</key>
  	<string>#{VER}</string>
  	<key>CFBundleSignature</key>
  	<string>WRUN</string>
  	<key>CFBundleVersion</key>
  	<string>#{VER}</string>
  	<key>CFBundleDevelopmentRegion</key>
    <string>English</string>
    <key>NSMainNibFile</key>
    <string>MainMenu</string>
  </dict>
</plist>
END
  plist.close
end
