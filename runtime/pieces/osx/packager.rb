module Titanium
  class OSX
    class Packager
      def Packager.package(project,dest_dir,version)
        pieces_dir = File.join(File.expand_path(File.dirname(__FILE__)),'pieces')
        executable = project.name
        copy_executable = File.join(pieces_dir,'titanium')
        
        app_folder = File.join(dest_dir,"#{project.name}.app")
        FileUtils.mkdir_p app_folder unless File.exists? app_folder

        contents_folder = File.join(app_folder, 'Contents')
        macos_folder = File.join(contents_folder, 'MacOS')
        resources_folder = File.join(contents_folder, 'Resources')
        plugins_folder = File.join(contents_folder, 'Plug-ins')
        titanium_folder = File.join(resources_folder, 'titanium')

        FileUtils.mkdir_p [app_folder, contents_folder, macos_folder, resources_folder, titanium_folder, plugins_folder]

        FileUtils.cp_r File.join(pieces_dir,'Titanium.app','Contents'), app_folder

        FileUtils.mv File.join(macos_folder,'Titanium'), File.join(macos_folder,executable)
        FileUtils.chmod 0755, File.join(macos_folder, executable)

        FileUtils.cp_r File.join(pieces_dir,'titanium'), resources_folder
        
        ipl = File.join(contents_folder,'Info.plist')
        infoplist = File.read(ipl)
        infoplist.gsub! '<string>Titanium</string>',"<string>#{executable}</string>"
        
        icns = File.join(Dir.pwd,'config',"#{executable}.icns")
        if File.exists?(icns) 
          FileUtils.cp icns, File.join(resources_folder,File.basename(icns))
          infoplist.gsub! '<string>titanium.icns</string>', "<string>#{File.basename(icns)}</string>"
        end
        
        f = File.open ipl, 'w'
        f.puts infoplist
        f.close
        
        Dir["public/**/*"].each do |file|
          next unless File.file? file
          name = file.to_s.gsub 'public/',''
          FileUtils.cp_r file, "#{resources_folder}/#{name}"
        end

      end
    end
  end
end
