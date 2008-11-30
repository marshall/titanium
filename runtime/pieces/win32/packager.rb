#
# Copyright 2008 Appcelerator, Inc.
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#    http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License. 

def is_cygwin?
	RUBY_PLATFORM.downcase.include?("cygwin")
end

module Titanium
  class WIN32
    class Packager
      def Packager.package(project,dest_dir,version)
        pieces_dir = File.join(File.expand_path(File.dirname(__FILE__)),'pieces')
        executable = project.name
        app_folder = File.join(dest_dir,"#{project.name}.win32")
        resources_folder = File.join(app_folder,'Resources')
        titanium_folder = File.join(app_folder,'titanium')
        
        FileUtils.mkdir_p [app_folder, resources_folder, titanium_folder]
        
        # copy all the win32 support files
        Dir["#{pieces_dir}/win32/*"].each do |file|
          next unless File.file? file
          name = file.to_s.gsub "#{pieces_dir}/win32", ''
          if name == "titanium.exe"
          	FileUtils.cp file, File.join(app_folder, project.name + ".exe") 
          else
          	FileUtils.cp_r file, File.join(app_folder, name)
        	end
        end
        
        # copy the titanium js files
        Dir["#{pieces_dir}/titanium/*"].each do |file|
          next unless File.file? file
          name = file.to_s.gsub "#{pieces_dir}/titanium/", ''
          target = File.join(titanium_folder,name)
          dir = File.dirname(target)
          FileUtils.mkdir_p dir unless File.exists? dir
          FileUtils.cp_r file, target
        end

        # copy the gears files
        Dir["#{pieces_dir}/gears/*"].each do |file|
          next unless File.file? file
          name = file.to_s.gsub "#{pieces_dir}/gears/", ''
          target = File.join(app_folder,name)
          dir = File.dirname(target)
          FileUtils.mkdir_p dir unless File.exists? dir
          FileUtils.cp_r file, target
        end
      
        # copy all the public files
        Dir["public/**/*"].each do |file|
          next unless File.file? file
          name = file.to_s.gsub 'public/',''
          target = File.join(resources_folder,name)
          dir = File.dirname(target)
          FileUtils.mkdir_p dir unless File.exists? dir
          FileUtils.cp_r file, target
        end

        # copy over tiapp.xml
        FileUtils.cp File.join('config','tiapp.xml'), File.join(resources_folder,'tiapp.xml')

        # we use the AID to create a unique URL hostname in app://
        aid = project.config[:aid] || Appcelerator::AID.generate_new(project)
        project.config[:aid] ||= aid
        
        # write out the projects AID
        aidf = File.open File.join(resources_folder,'aid'), 'w'
        aidf.write aid
        aidf.close
        
        if is_cygwin?
          FileUtils.chmod 0755, File.join(app_folder, 'titanium.exe')
        end
        
      end
    end
  end
end

