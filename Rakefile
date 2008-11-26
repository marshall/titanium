require 'fileutils'
require 'build.rb'
require 'yaml'

task :default => [:runtime,:plugins]
task :dev_install => [:runtime,:runtime_install,:plugins,:plugins_install]

task :runtime do
	rake(RUNTIME_DIR)
end

task :runtime_install do
  runtime_build = {}
  File.open(File.join(STAGE_YML_DIR, 'runtime_build.yml')) { |f| runtime_build = YAML::load(f.read) }
  
  system "app install:titanium --force #{STAGE_DIR}/titanium_runtime_" + runtime_build[:version].to_s + "_" + runtime_build[:platform] + ".zip"
end

task :plugins do
	Dir[PLUGINS_DIR+"/*"].each do |path|
		rake(path)
	end
end

task :plugins_install do
  Dir[PLUGINS_DIR+"/*"].each do |path|
		plugin_name = File.basename path
		
		plugin_build = {}
    File.open(File.join(STAGE_YML_DIR, "tiplugin_#{plugin_name}_#{platform_string}_build.yml")) { |f| plugin_build = YAML::load(f.read) }
    if plugin_build.has_key?(:platform)
		  system "app install:tiplugin --force #{STAGE_DIR}/tiplugin_#{plugin_build[:basename]}_#{plugin_build[:version]}_#{plugin_build[:platform]}.zip"
	  else
	    system "app install:tiplugin --force #{STAGE_DIR}/tiplugin_#{plugin_build[:basename]}_#{plugin_build[:version]}.zip"
    end
	end
end
