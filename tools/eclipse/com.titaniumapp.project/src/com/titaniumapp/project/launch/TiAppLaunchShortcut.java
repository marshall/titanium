package com.titaniumapp.project.launch;

import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.IAdaptable;
import org.eclipse.core.runtime.NullProgressMonitor;
import org.eclipse.debug.core.DebugPlugin;
import org.eclipse.debug.core.ILaunchConfiguration;
import org.eclipse.debug.core.ILaunchConfigurationType;
import org.eclipse.debug.core.ILaunchConfigurationWorkingCopy;
import org.eclipse.debug.core.ILaunchManager;
import org.eclipse.debug.ui.ILaunchShortcut;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.ui.IEditorPart;
import org.eclipse.ui.IFileEditorInput;

public class TiAppLaunchShortcut implements ILaunchShortcut {
	public void launch(ISelection selection, String mode) {
		IStructuredSelection s = (IStructuredSelection)selection;
		if (!s.isEmpty()) {
			Object element = s.getFirstElement();
			if (element instanceof IResource) {
				IResource resource = (IResource)element;
				launchProject(resource.getProject(), mode);
			}
			else if (element instanceof IAdaptable) {
				IAdaptable adaptable = (IAdaptable)element;
				IProject project = (IProject) adaptable.getAdapter(IProject.class);
				if (project != null) {
					launchProject(project, mode);
				}
			}
		}
	}

	public void launch(IEditorPart editor, String mode) {
		
		if (editor.getEditorInput() instanceof IFileEditorInput) {
			IFileEditorInput input = (IFileEditorInput) editor.getEditorInput();
			IProject project = input.getFile().getProject();
	
			launchProject(project, mode);
		}
	}

	protected void launchProject(IProject project, String mode) {
		ILaunchManager manager = DebugPlugin.getDefault().getLaunchManager();
		
		ILaunchConfigurationType launchConfigType =
			manager.getLaunchConfigurationType("com.titaniumapp.project.tiLaunch");
		ILaunchConfiguration config = null;

		try {
			for (ILaunchConfiguration existingConfig : manager.getLaunchConfigurations()) {
				if (existingConfig.getFile() != null && existingConfig.getFile().getProject().equals(project)) {
					config = existingConfig;
				}
			}
			
			if (config == null) {
				String launchConfigName = manager.generateUniqueLaunchConfigurationNameFrom(project.getName());
				config = launchConfigType.newInstance(project, launchConfigName);
				ILaunchConfigurationWorkingCopy wc = config.getWorkingCopy();
				
				wc.setAttribute(TitaniumLaunchDelegate.PROPERTY_PROJECT, project.getName());
				config = wc.doSave();
				
			}
			
			if (config != null) {
				config.launch(mode, new NullProgressMonitor());
			}
		} catch (CoreException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
	}

}
