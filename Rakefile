require 'fileutils'
require 'build.rb'

task :default => [:runtime,:plugins]
task :dev_install => [:runtime,:runtime_install,:plugins,:plugins_install]

task :runtime do
	rake(RUNTIME_DIR)
end

task :runtime_install do
  system "app install:titanium --force #{RUNTIME_DIR}/stage/titanium_runtime*.zip"
end

task :plugins do
	Dir[PLUGINS_DIR+"/*"].each do |path|
		rake(path)
	end
end

task :plugins_install do
  Dir[PLUGINS_DIR+"/*"].each do |path|
		plugin_name = File.basename path
		system "app install:tiplugin --force #{RUNTIME_DIR}/stage/tiplugin_#{plugin_name}*.zip"
	end
end
