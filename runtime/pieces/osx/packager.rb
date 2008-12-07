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
require "rexml/document"
include REXML

module Titanium
  class OSX
    class Packager
      def Packager.die(msg)
        $stderr.puts msg
        puts
        exit 1
      end
      def Packager.must_have_element(doc,name)
        el = XPath.first(doc,"//#{name}")
        self.die "<#{name}> must be present in tiapp.xml" unless (el and el.text)
        yield el if block_given?
      end
      def Packager.validate
        # VALIDATE XML
        puts "Validating tiapp.xml ... "
        file = File.new( File.join('config','tiapp.xml') )
        doc = Document.new file
        %w(id name updatesite description copyright homepage version window).each do |name|
          self.must_have_element doc,name do |el|
            if name == 'id'
              re = /([\w\._]+)/.match(el.text)
              die "<id> format is invalid. Must be in the format: [a-zA-Z0-9_.]" unless (re and re[0]==el.text) #exact match
            end
          end
        end
        XPath.each(doc,'window') do |el|
          %w(id title url width height).each do |name|
            self.must_have_element el,name
          end
        end
        puts "Looks good ... Let's packager up!"
      end
      def Packager.package(project,dest_dir,version)
        self.validate
        
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

        # we use the AID to create a unique URL hostname in app://
        aid = project.config[:aid] || Appcelerator::AID.generate_new(project)
        project.config[:aid] ||= aid
        
        # write out the projects AID
        aidf = File.open File.join(resources_folder,'aid'), 'w'
        aidf.write aid
        aidf.close
        
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
        
        
        # copy over tiapp.xml
        FileUtils.cp File.join('config','tiapp.xml'), File.join(resources_folder,'tiapp.xml')
        
        # copy all the public files
        Dir["public/**/*"].each do |file|
          next unless File.file? file
          name = file.to_s.gsub 'public/',''
          target = File.join(resources_folder,name)
          dir = File.dirname(target)
          FileUtils.mkdir_p dir unless File.exists? dir
          FileUtils.cp_r file, target
        end

      end
    end
  end
end
