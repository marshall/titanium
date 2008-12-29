package com.titaniumapp.project.launch;

import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.debug.core.ILaunchConfiguration;
import org.eclipse.debug.core.ILaunchConfigurationWorkingCopy;
import org.eclipse.debug.ui.AbstractLaunchConfigurationTab;
import org.eclipse.swt.SWT;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Combo;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Label;

import com.titaniumapp.project.TitaniumProjectNature;
import com.titaniumapp.project.TitaniumSharedImages;

public class TitaniumLaunchTab extends AbstractLaunchConfigurationTab {

	protected Combo tiProject;
	protected String projectName;
	
	@Override
	public Image getImage() {
		return TitaniumSharedImages.getImage(TitaniumSharedImages.TITANIUM_16);
	}
	
	public void createControl(Composite parent) {
		Composite main = new Composite(parent, SWT.NONE);
		main.setLayout(new GridLayout(2, false));
		main.setLayoutData(new GridData(SWT.FILL, SWT.FILL, true, true));
		
		new Label(main, SWT.NONE).setText("Titanium project: ");
		tiProject = new Combo(main, SWT.NONE);
		int index = 0;
		boolean selected = false;
		for (IProject project : ResourcesPlugin.getWorkspace().getRoot().getProjects()) {			
			try {
				if (project.hasNature(TitaniumProjectNature.NATURE_ID)) {
					index++;
					tiProject.add(project.getName());
					if (project.getName().equals(projectName)) {
						tiProject.select(index);
						selected = true;
					}
				}
			} catch (CoreException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
		
		if (!selected && index > 0) {
			tiProject.select(0);
		} else {
			setErrorMessage("Error: no Titanium projects found");
		}
		
		setControl(main);
	}

	public String getName() {
		return "Titanium";
	}

	public void initializeFrom(ILaunchConfiguration configuration) {
		try {
			projectName =
				configuration.getAttribute(TitaniumLaunchDelegate.PROPERTY_PROJECT, (String)null);
			
		} catch (CoreException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

	public void performApply(ILaunchConfigurationWorkingCopy configuration) {
		if (tiProject.getText() != "") {
			configuration.setAttribute(TitaniumLaunchDelegate.PROPERTY_PROJECT, tiProject.getText());
		}
	}

	public void setDefaults(ILaunchConfigurationWorkingCopy configuration) {

	}

}
