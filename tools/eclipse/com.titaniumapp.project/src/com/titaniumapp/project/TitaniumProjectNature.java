package com.titaniumapp.project;

import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IProjectNature;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.IPath;

import com.titaniumapp.project.appc.AppCommand;

public class TitaniumProjectNature implements IProjectNature {

	public static final String NATURE_ID = "com.titaniumapp.project.tiProjectNature";
	
	protected IProject project;
	
	public void configure() throws CoreException {
	}

	public void deconfigure() throws CoreException {
		// TODO Auto-generated method stub

	}

	public IProject getProject() {
		return project;
	}

	public void setProject(IProject project) {
		this.project = project;
	}

}
