package com.titaniumapp.project;

import org.eclipse.ui.IStartup;

public class TiPluginStartup implements IStartup {

	public void earlyStartup() {
		Activator.getDefault();
	}

}
