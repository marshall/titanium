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


include Appcelerator
module Appcelerator
  class Installer

    def Installer.install_web_project(options,tx,update=false,sdk=nil)
      
      raise "Invalid options, must specify :web option" unless options[:web]
      raise "Invalid options, must specify :javascript option" unless options[:javascript]
      raise "Invalid options, must specify :images option" unless options[:images]
      raise "Invalid options, must specify :widgets option" unless options[:widgets]

      if sdk.nil?
        sdk = Installer.require_component(:websdk, 'websdk', nil,
                                          :tx=>tx, :force=>update,
                                          :quiet_if_installed=>true)
      end
      source_dir = Installer.get_component_directory(sdk)
      web_version = sdk[:version]
      
      options[:websdk] = web_version
      options[:installed_widgets] = []

      puts "Using websdk #{web_version}" unless OPTIONS[:quiet]

      event = {:options=>options,:source_dir=>source_dir,:version=>web_version,:tx=>tx}
      PluginManager.dispatchEvents('copy_web',event) do
        
        Installer.copy(tx, "#{source_dir}/javascripts/.", options[:javascript])
        Installer.copy(tx, "#{source_dir}/swf/.", options[:web] + '/swf')
        Installer.copy(tx, "#{source_dir}/common/.", options[:widgets] + '/common')

        add_thing_options = {
          :quiet=>true,
          :quiet_if_installed=>true,
          :tx=>tx,
          :ignore_path_check=>true,
          :no_save=>false,
          :verbose=>false,
          :force_overwrite=>true
        }

        puts "Installing components ... (this may take a few seconds)" unless OPTIONS[:quiet]
        
        cur_quiet = OPTIONS[:quiet]
        OPTIONS[:quiet] = true
        
        count = 0
        
        # Count components to install first then re-iterate and actually install/add them (so we get an interactive progress bar)
        # include any bundled components automagically
        Dir["#{source_dir}/_install/*.zip"].each do |filename|
          type = File.basename(filename).split('_')
          next unless type.length > 0
          count+=1
        end
        
        progress = TitaniumProgressBar.new("create_project_progress", count)
        
        Dir["#{source_dir}/_install/*.zip"].each do |filename|
          type = File.basename(filename).split('_')
          next unless type.length > 0
          type = type.first
          progress.setMessage("Installing #{type} " + File.basename(filename) + " ...")
          CommandRegistry.execute("install:#{type}",[filename],add_thing_options)
          CommandRegistry.execute("add:#{type}",[filename,options[:project]],add_thing_options)
          progress.increment(1)
        end
        
        progress.close()

        puts "#{count} components installed ... " unless OPTIONS[:quiet]
        
        OPTIONS[:quiet] = cur_quiet
        
        if not update
          Installer.copy(tx, "#{source_dir}/images/.", options[:images])
          Installer.copy(tx, Dir.glob("#{source_dir}/*.html"), options[:web])
          
          widgets = Installer.find_dependencies_for(sdk) || []
          # install our widgets
          widgets.each do |widget|
            add_widget_options = {
              :version=>widget[:version],
              :quiet=>true,
              :quiet_if_installed=>true,
              :tx=>tx,
              :ignore_path_check=>true,
              :no_save=>true
            }
            CommandRegistry.execute('add:widget',[widget[:name],options[:project]],add_widget_options)
            options[:installed_widgets] << {:name=>widget[:name],:version=>widget[:version]}
          end
        end
      end
      
      options
    end
  end
end