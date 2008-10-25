class TitaniumEventsPlugin < Appcelerator::Plugin
	def before_create_project(event)
		$titanium.debug "before_create_project: #{event}"
	end

	def after_create_project(event)
	    projectName = event[:name]
	    projectDir = event[:project_dir]
	    projectService = event[:service]
	    
			$titanium.debug "after_create_project: #{projectName}"
	    $titanium.finishedCreatingProject(projectName, projectDir)
	end
end
