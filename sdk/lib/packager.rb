#
# Copyright 2006-2008 Appcelerator, Inc.
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

require 'erb'

module Titanium
  class Packager
    def Packager.copy_template(template_file, dest)
      contents = ''
      File.open(template_file, "r") { |f| contents = f.read }

      f_binding = binding
      template = ERB.new contents
      File.open(dest, 'w') { |f| f.write(template.result(f_binding)) }
    end
    
    def Packager.create_osx_app
      # store these as class variables for now so we can access them in other functions
      app_folder = File.join(@@dest, @@executable_name + ".app")
      contents_folder = File.join(app_folder, 'Contents')
      macos_folder = File.join(contents_folder, 'MacOS')
      resources_folder = File.join(contents_folder, 'Resources')
      titanium_folder = File.join(resources_folder, 'public', 'titanium')
      osx_support_folder = File.join(Titanium.get_support_dir(), 'osx')

      FileUtils.mkdir_p [app_folder, contents_folder, macos_folder, resources_folder, titanium_folder]
      
      FileUtils.cp Titanium.get_webkit_shell_executable(), macos_folder
      FileUtils.chmod 0755, File.join(macos_folder, 'webkit_shell')
      FileUtils.cp_r File.join(@@project.path, 'public'), resources_folder
      FileUtils.cp File.join(osx_support_folder, 'appcelerator.icns'), resources_folder
      
      Packager.copy_template(
        File.join(osx_support_folder, 'Info.plist.template'),
        File.join(contents_folder, 'Info.plist'))
      
      Packager.copy_template(
        File.join(Titanium.get_support_dir(), 'default_tiapp.xml.template'),
        File.join(resources_folder, 'tiapp.xml'))
      
      Packager.copy_template(
        File.join(Titanium.get_support_dir(), 'plugins.js.template'),
        File.join(titanium_folder, 'plugins.js'))
      
      FileUtils.cp File.join(Titanium.get_component_dir(), 'titanium.js'), titanium_folder
      FileUtils.cp_r File.join(Titanium.get_webkit_shell_path(), 'Contents', 'Resources', "English.lproj"), resources_folder
      
      Titanium.each_plugin do |plugin|
        plugin.install(@@project, @@dest, @@executable_name)
      end
      
      puts "#{@@executable_name}.app created in #{@@dest}"
    end
    
    def Packager.launch_osx_app
      system "open " + File.join(@@dest, @@executable_name) + ".app"
    end
    
    def Packager.create_win_exe
    end
    
    def Packager.create_linux_dist
    end
    
    def Packager.package_project(project, executable_name, dest, endpoint, launch)
      @@project = project
      @@executable_name = executable_name
      @@dest = dest
      @@endpoint = endpoint
      
      if is_mac?
        Packager.create_osx_app()
        Packager.launch_osx_app() if launch
      elsif is_win?
        Packager.create_win_exe()
      else
        Packager.create_linux_dist()
      end
    end
  end
end