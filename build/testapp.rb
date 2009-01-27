#!/usr/bin/env ruby
#
# this is a simple test builder for packaging a new app
#
#
require 'rubygems'
require 'zip/zip'
require 'fileutils'

VER = 0.1
PLUGINS =
  %w(api javascript foo foojs foorb foopy python ruby
    tiplatform tiapp tiui tinetwork tifilesystem 
    timedia tidesktop tigrowl tidatabase tiprocess)
NAME = 'test'

if RUBY_PLATFORM=~/darwin/
  OS='osx'
elsif RUBY_PLATFORM=~/linux/
  OS='linux'
else
  OS='win32'
end

thisdir = File.expand_path(File.dirname(__FILE__))
outdir = File.join(thisdir, OS)
rootdir = File.join(outdir, NAME + (OS=='osx' ? '.app':'' ))
appdir = OS=='osx' ? File.join(rootdir,'Contents') : rootdir
installdir = File.join(rootdir,'installer');
ext = OS=='win32' ? '.exe' : ''
thirdparty = File.join(File.expand_path(File.dirname(__FILE__)+'/../kroll/thirdparty'), OS)
resourcesdir = File.join(appdir, 'Resources')
frameworksdir = File.join(appdir, 'Frameworks')

FileUtils.rm_rf [installdir,appdir,rootdir]
FileUtils.mkdir_p [outdir,appdir,installdir, resourcesdir]
if OS=='osx'
	FileUtils.mkdir_p frameworksdir
end

if OS=='osx'
	FileUtils.rm_rf "~/Library/Application Support/Titanium"
	FileUtils.rm_rf "/Library/Application Support/Titanium"
end
if OS=='linux'
	FileUtils.rm_rf File.expand_path("~/Titanium")
end
if OS=='win32'
  FileUtils.rm_rf "c:/ProgramData/Titanium"  
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

  paths = ['webkit', 'poco', 'poco/lib', 'libcurl']
  paths.each do |path|
    path = "#{thirdparty}/#{path}"
    Dir["#{path}/**/**"].each do |f|
      name = f.gsub(path+'/','')
      zipfile.add name,f unless (name=~/\.h$/ or name=~/\.defs$/)
    end
  end

end

def script?(f)
  e = File.extname(f)
  return e == '.rb' || e == '.py' || e == '.js'
end

LANGS = {
  '.rb'=>'ruby',
  '.py'=>'python',
  '.js'=>'javascript'
}

FileUtils.cp File.join(outdir,'kinstall'+ext), File.join(installdir,'kinstall'+ext)

PLUGINS.each do |plugin|
  plugin.strip!
  plugin_zip = File.join(installdir,"module-#{plugin}-0.1.zip")
  FileUtils.rm_rf plugin_zip

  Zip::ZipFile.open(plugin_zip, Zip::ZipFile::CREATE) do |zipfile|
    Dir["#{outdir}/**"].each do |f|
      name = f.gsub(outdir+'/','')
		  next if File.extname(name)=='.o'
		  lang = LANGS[File.extname(f)]
		  if lang
		    e = File.extname(f)[1..-1]
		    re = "^#{plugin}module(.*?)#{e}$"
		    r = Regexp.new re
		    next unless r.match(name)
	    else
  	    next unless name.index(plugin) and name.index('module')
	    end
      zipfile.add name,f
    end
		Dir["#{outdir}/modules/*"].each do |f|
			modname = f.gsub(outdir+'/modules/', '')
			modname_nodot = modname.gsub(/\./, '')
			next unless modname_nodot.downcase.index(plugin)
			Dir["#{outdir}/modules/#{modname}/Resources/**/*"].each do |r|
				name = r.gsub(outdir+'/modules/'+modname+'/', '')
				zipfile.add name, r
			end
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

#copy test app files
FileUtils.cp_r(thisdir+'/testapp/.', appdir)
