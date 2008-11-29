module Titanium
  class Project
    def Project.update_project(from_version, to_version, tx, project)
      if (from_version==to_version)
        return Project.create_project(tx,project)
      end
      true
    end
    def Project.create_project(tx, project)
      mydir = File.expand_path(File.dirname(__FILE__))
      config_dir = "#{project.path}/config"
      FileUtils.mkdir_p(config_dir) unless File.exists?(config_dir) 
      
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
end