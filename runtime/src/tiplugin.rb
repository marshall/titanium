require 'yaml'

module Titanium
  class Plugin
    attr_accessor :data
    attr_accessor :name
    attr_accessor :component
    
    def initialize(name)
      @name = name
      @component = Installer.get_current_installed_component({:name=>@name, :type=>'tiplugin'})
      @plugindir = Titanium.get_plugin_dir(@component)
      @data = YAML::load(File.open(File.join(@plugindir, 'build.yml')))
    end
    
    def has_native_plugin?
      return get_native_plugin() != nil
    end
    
    def get_native_plugin
      return nil
    end
    
    def get_project_path(basedir, executable_name)
      if is_mac?
        return File.join(basedir, executable_name+".app", 'Contents', 'Resources')
      elsif is_win?
	  	  return File.join(basedir, executable_name, 'Resources')
      end
    end
    
    def get_plugins_path(basedir, executable_name)
    	if is_mac?
    		return File.join(basedir, executable_name+".app", 'Contents', 'Plug-ins')
		  elsif is_win?
			  return File.join(basedir, executable_name, 'plugins')
		  end
	  end
    
    def install(project, basedir, executable_name)
      @project_path = get_project_path(basedir, executable_name)
      @plugins_path = get_plugins_path(basedir, executable_name)

      @titanium_path = File.join @project_path, 'titanium'
      @plugin_path = File.join @titanium_path, @name
      
      FileUtils.mkdir_p [@titanium_path, @plugins_path, @plugin_path]
      @data[:files].each do |glob|
        glob = File.join(@plugindir, glob)
        Dir.glob(glob) do |filename|
          FileUtils.cp filename, @plugin_path 
        end
      end
    end
    
  end
end
