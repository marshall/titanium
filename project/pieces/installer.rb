#
# This file is part of Appcelerator.
#
# Copyright 2008 Appcelerator, Inc.
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
require 'erb'

# don't put this in a module, it will automatically inherit the 
# Appcelerator module because we're using include to dynamically load this
class TitaniumProject
  def update_project(from_version, to_version, tx, project)
    if (from_version==to_version)
      return Project.create_project(tx,project)
    end
    true
  end
  def create_project(tx, project)
    mydir = File.expand_path(File.dirname(__FILE__))
    config_dir = "#{project.path}/config"
    FileUtils.mkdir_p(config_dir) unless File.exists?(config_dir) 

    aid = Appcelerator::AID.generate_new(project)
    project.config[:aid] = aid
    
    Dir["#{mydir}/config/*"].each do |file|
      next unless File.file? file
    	template = ERB.new File.read(file)
    	config = project.config
    	out = template.result(binding)
    	f = File.open "#{config_dir}/#{File.basename(file)}",'w'
    	f.puts out
    	f.close
    end

    Installer.copy(tx, "#{mydir}/public", project.get_path(:web)) 
    true
  end
end
