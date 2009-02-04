#!/usr/bin/env ruby
#
# Titanium Runtime Bundle Builder
#
#
require 'rubygems'
require 'fileutils'
require 'zip/zip'

NAME="titanium_runtime"
VER = 0.1
RUNTIME_VER = 0.2

#TODO: this need to use the module builder
MODULES =
  %w(api javascript foo ruby python
    tiplatform tiapp tiui tinetwork tifilesystem 
    timedia tidesktop tigrowl tidatabase tiprocess)

if RUBY_PLATFORM=~/darwin/
  OS='osx'
elsif RUBY_PLATFORM=~/linux/
  OS='linux'
else
  OS='win32'
end

THISDIR = File.expand_path(File.dirname(__FILE__))
TOPDIR = File.expand_path(File.join(THISDIR,'/../'))
OUTDIR = File.join(THISDIR, OS)
SUPPORTDIR = File.join(TOPDIR,'support',OS)

manifest=<<-END
appname:#{NAME}
appid:#{NAME}
runtime:#{RUNTIME_VER}
END

THIRDPARTY = %w(webkit poco)
RUNTIME = %w(khost kroll)

case OS
	when 'win32'
		zf = File.join(OUTDIR,"#{NAME}.zip")
		FileUtils.rm_rf zf if File.exists?(zf)
		Zip::ZipFile.open(zf, Zip::ZipFile::CREATE) do |zipfile|
			RUNTIME.each do |file|
				zipfile.add "runtime/#{file}.dll",File.join(OUTDIR,"#{file}.dll")
			end
			THIRDPARTY.each do |lib|
				Dir["#{TOPDIR}/kroll/thirdparty/#{OS}/#{lib}/bin/*"].each do |f|
					zipfile.add "runtime/#{File.basename(f)}",f
				end
			end
			MODULES.each do |m|
				mf = File.join(OUTDIR,"#{m}module.dll")
				next unless File.exists?(mf)
				zipfile.add "modules/#{m}/#{m}module.dll",mf
				manifest << "#{m}:#{VER}\n"
			end
			zipfile.get_output_stream('manifest') do |f|
				f.puts manifest
			end
			base = "#{TOPDIR}/installation/runtime/"
			Dir["#{base}**/**"].each do |f|
				next if File.directory?(f)
				p = f.gsub(base,'')
				zipfile.add p,f
			end
			#TODO: installer
		end

		exe = File.join(OUTDIR,"#{NAME}.exe")
		FileUtils.rm_rf exe if File.exists?(exe)

		out = File.open(exe,'ab')
		File.open(File.join(OUTDIR,'kboot.exe'), 'rb') {|io| out.write(io.read) }
		File.open(zf, 'rb') {|io| out.write(io.read) }
		out.flush
		out.close
	when 'osx'
		app = File.join(OUTDIR,"#{NAME}.app")
		FileUtils.rm_rf app if File.exists?(app)
		contents = File.join(app,'Contents')
		macos = File.join(contents,'MacOS')
		resources = File.join(contents,'Resources')
		runtime = File.join(contents,'runtime')
		modules = File.join(contents,'modules')
		lproj = File.join(resources,'English.lproj')
		
		FileUtils.mkdir_p [contents,macos,resources,runtime,modules,lproj]

		RUNTIME.each do |file|
			FileUtils.cp File.join(OUTDIR,"lib#{file}.dylib"), runtime
		end
		THIRDPARTY.each do |lib|
			Dir["#{TOPDIR}/kroll/thirdparty/#{OS}/#{lib}/lib/*"].each do |f|
			  next if File.basename(f)=~/6/  #FIXME: deal with this
				FileUtils.cp f,runtime
			end
			#FIXME! deal with symlinks
			Dir["#{TOPDIR}/kroll/thirdparty/#{OS}/#{lib}/*.framework"].each do |f|
			  FileUtils.mkdir File.join(runtime,File.basename(f))
				FileUtils.cp_r "#{f}/.",File.join(runtime,File.basename(f))
				FileUtils.rm_rf File.join(runtime,File.basename(f),'Headers')
				FileUtils.rm_rf File.join(runtime,File.basename(f),'PrivateHeaders')
			end
		end
		MODULES.each do |m|
			mf = File.join(OUTDIR,"lib#{m}module.dylib")
			next unless File.exists?(mf)
			dir = File.join(modules,m)
			FileUtils.mkdir dir
			FileUtils.cp mf,dir
			manifest << "#{m}:#{VER}\n"
		end
		manf = File.open "#{contents}/manifest",'w'
		manf.puts manifest
		manf.close
		FileUtils.cp "#{OUTDIR}/kboot","#{macos}/#{NAME}"
		FileUtils.cp_r "#{TOPDIR}/installation/runtime/.",contents
		FileUtils.cp_r "#{SUPPORTDIR}/titanium.icns",lproj
		FileUtils.cp_r "#{OUTDIR}/modules/ti.UI/MainMenu.nib",lproj
		plist = File.read "#{SUPPORTDIR}/Info.plist"
		plist.gsub! 'APPNAME',NAME
		plist.gsub! 'APPICON','titanium.icns'
		plist.gsub! 'APPID','com.titaniumapp.installer.runtime'
		plist.gsub! 'APPNIB','MainMenu'
		plist.gsub! 'APPVER',RUNTIME_VER.to_s
		plistf = File.open(File.join(contents,'Info.plist'),'w')	
		plistf.puts plist
		plistf.close
		
		#TODO: build DMG
		
	when 'linux'
end
