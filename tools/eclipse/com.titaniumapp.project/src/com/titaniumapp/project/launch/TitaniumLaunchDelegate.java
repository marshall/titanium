package com.titaniumapp.project.launch;

import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.debug.core.ILaunch;
import org.eclipse.debug.core.ILaunchConfiguration;
import org.eclipse.debug.core.model.ILaunchConfigurationDelegate;
import org.eclipse.debug.core.model.RuntimeProcess;

import com.titaniumapp.project.appc.AppCommand;

public class TitaniumLaunchDelegate implements ILaunchConfigurationDelegate {

	public static final String PROPERTY_PROJECT = "titanium.project";

	public void launch(ILaunchConfiguration configuration, String mode,
			ILaunch launch, IProgressMonitor monitor) throws CoreException {

		try {
			monitor.beginTask("Launching Titanium App...", 2);
			monitor.subTask("Packaging project");
			String projectName = configuration.getAttribute(PROPERTY_PROJECT, (String)null);
			IProject project = ResourcesPlugin.getWorkspace().getRoot().getProject(projectName);
			
			Process p = AppCommand.packageProject(project.getLocation().toOSString());
			RuntimeProcess process = new RuntimeProcess(launch, p, "app package:project", null);
			launch.addProcess(process);
			p.waitFor();
			
			monitor.worked(1);
			
			monitor.subTask("Executing app");
			p = AppCommand.runPackagedProject(project);	
			process = new RuntimeProcess(launch, p, AppCommand.getExecutablePath(project).toOSString(), null);
			launch.addProcess(process);
			
			monitor.done();
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
	}

}
