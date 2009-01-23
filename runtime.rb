#!/usr/bin/env ruby

thisdir = File.expand_path(File.dirname(__FILE__))
outdir = File.join(thisdir, 'build', platform_string)
#rootdir = File.join(outdir, NAME + (platform_string=='osx' ? '.app':'' ))
#appdir = platform_string=='osx' ? File.join(rootdir,'Contents') : rootdir
#installdir = File.join(rootdir,'installer');
ext = platform_string=='win32' ? '.exe' : ''
thirdparty = File.join(File.expand_path(File.dirname(__FILE__)+'/../kroll/thirdparty'), platform_string)
#resourcesdir = File.join(appdir, 'Resources')
#frameworksdir = File.join(appdir, 'Frameworks')

task :runtime => [:scons]

def installer?(name)
  if name=~/(kkernel|kroll|khost)/
	  return true if platform_string!='win32'
	  return true if platform_string=='win32' and (name.index('.exe') or name.index('.dll'))
  end
  false
end

case platform_string
when 'osx'
	EXT='.dylib'
when 'linux'
	EXT='.so'
when 'win32'
	EXT='.dll'
end

task :runtime do
  #FileUtils.rm_rf [installdir,appdir,rootdir]
  FileUtils.mkdir_p [outdir]
  
  runtime_zip = File.join(STAGE_DIR,"titanium-runtime-#{platform_string}-#{TITANIUM_CONFIG[:version]}.zip")
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
end