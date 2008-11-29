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
        copy_executable = File.join(pieces_dir,'titanium.exe')
        
        app_folder = File.join(dest_dir,"#{project.name}.win32")
        FileUtils.mkdir_p app_folder unless File.exists? app_folder
        
        # copy all the win32 support files
        Dir["#{pieces_dir}/win32/*"].each do |file|
          next unless File.file? file
          name = file.to_s.gsub "#{pieces_dir}/", ''
          FileUtils.cp_r file, File.join(app_folder,name)
        end
        
        # copy the titanium js files
        Dir["#{pieces_dir}/titanium/*"].each do |file|
          next unless File.file? file
          name = file.to_s.gsub "#{pieces_dir}/titanium/", ''
          FileUtils.cp_r file, File.join(app_folder,'titanum',name)
        end

        # copy the gears files
        Dir["#{pieces_dir}/gears/*"].each do |file|
          next unless File.file? file
          name = file.to_s.gsub "#{pieces_dir}/gears/", ''
          FileUtils.cp_r file, File.join(app_folder,name)
        end
      
        # copy all the public files
        Dir["public/**/*"].each do |file|
          next unless File.file? file
          name = file.to_s.gsub 'public/',''
          FileUtils.cp_r file, File.join(app_folder,name)
        end
        
      end
    end
  end
end

