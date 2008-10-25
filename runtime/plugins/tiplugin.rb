require 'yaml'

TIPLUGINDIR = File.join(SYSTEMDIR, 'titanium', 'plugins')

module Titanium
  class Plugin
    attr_accessor :data
    
    def initialize(name)
     @plugindir = File.join(TIPLUGINDIR, name)
     @name = name
     @data = YAML::load(File.open(File.join(@plugindir, 'build.yml')))
     puts @data.inspect
    end
    
    def get_project_path(basedir, executable_name)
      if is_mac?
        return File.join(basedir, executable_name+".app",
          'Contents', 'Resources', 'public')
      end
    end
    
    def install(project, basedir, executable_name)
      project_path = get_project_path(basedir, executable_name)
      titanium_path = File.join project_path, 'titanium'
      plugin_path = File.join titanium_path, @name
      
      FileUtils.mkdir_p [titanium_path, plugin_path]
      @data[:files].each do |glob|
        glob = File.join(@plugindir, glob)
        Dir.glob(glob) do |filename|
          FileUtils.cp filename, plugin_path 
        end
      end
    end
    
  end
end
