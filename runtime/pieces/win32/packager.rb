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
          name = file.to_s.gsub "#{pieces_dir}/", ''
          FileUtils.cp_r file, File.join(app_folder, name)
        end
        
        # copy the titanium js files
        Dir["#{pieces_dir}/titanium/*"].each do |file|
          next unless File.file? file
          name = file.to_s.gsub "#{pieces_dir}/titanium/", ''
          FileUtils.cp_r file, File.join(titanium_folder, name)
        end

        # copy the gears files
        Dir["#{pieces_dir}/gears/*"].each do |file|
          next unless File.file? file
          name = file.to_s.gsub "#{pieces_dir}/gears/", ''
          FileUtils.cp_r file, File.join(app_folder, name)
        end
      
        # copy all the public files
        Dir["public/**/*"].each do |file|
          next unless File.file? file
          name = file.to_s.gsub 'public/',''
          FileUtils.cp_r file, File.join(resources_folder, name)
        end

        # copy over tiapp.xml
        FileUtils.cp File.join('config','tiapp.xml'), resources_folder
        
        if is_cygwin?
          FileUtils.chmod 0755, File.join(app_folder, 'titanium.exe')
        end
        
      end
    end
  end
end

