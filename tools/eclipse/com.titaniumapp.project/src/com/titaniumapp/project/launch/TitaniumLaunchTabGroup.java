package com.titaniumapp.project.launch;

import org.eclipse.debug.ui.AbstractLaunchConfigurationTabGroup;
import org.eclipse.debug.ui.CommonTab;
import org.eclipse.debug.ui.ILaunchConfigurationDialog;
import org.eclipse.debug.ui.ILaunchConfigurationTab;

public class TitaniumLaunchTabGroup extends AbstractLaunchConfigurationTabGroup {

	public TitaniumLaunchTabGroup() {
		// TODO Auto-generated constructor stub
	}

	public void createTabs(ILaunchConfigurationDialog dialog, String mode) {
		ILaunchConfigurationTab tabs[] = new ILaunchConfigurationTab[] {
				new TitaniumLaunchTab(),
				new CommonTab()
		};
		setTabs(tabs);
	}

}
