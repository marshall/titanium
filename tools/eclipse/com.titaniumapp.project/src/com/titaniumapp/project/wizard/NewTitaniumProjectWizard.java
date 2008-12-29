package com.titaniumapp.project.wizard;

import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;
import java.util.Arrays;

import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IProjectDescription;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.core.runtime.SubProgressMonitor;
import org.eclipse.debug.core.DebugPlugin;
import org.eclipse.debug.core.Launch;
import org.eclipse.debug.core.model.IProcess;
import org.eclipse.debug.internal.ui.views.console.ProcessConsole;
import org.eclipse.debug.ui.console.ConsoleColorProvider;
import org.eclipse.jface.operation.IRunnableWithProgress;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.wizard.Wizard;
import org.eclipse.swt.widgets.Display;
import org.eclipse.ui.INewWizard;
import org.eclipse.ui.IWorkbench;
import org.eclipse.ui.console.ConsolePlugin;
import org.eclipse.ui.console.IConsole;
import org.eclipse.ui.dialogs.WizardNewProjectCreationPage;

import com.titaniumapp.project.TitaniumProjectNature;
import com.titaniumapp.project.TitaniumSharedImages;
import com.titaniumapp.project.appc.AppCommand;

public class NewTitaniumProjectWizard extends Wizard implements INewWizard {

	WizardNewProjectCreationPage page;
	
	public NewTitaniumProjectWizard() {

	}
	
	@Override
	public void addPages() {
		page = new WizardNewProjectCreationPage("Create a Titanium project");
		page.setImageDescriptor(TitaniumSharedImages.getImageDescriptor(TitaniumSharedImages.TITANIUM_64));
		page.setInitialProjectName("DemoApp");
		page.setTitle("Create a Titanium project");
		
		addPage(page);
	}
	
	void addTitaniumNature(IProject project, IProgressMonitor monitor)
		throws CoreException
	{
		if (!project.hasNature(TitaniumProjectNature.NATURE_ID)) {
			IProjectDescription desc = project.getDescription();
			String[] oldNatures = desc.getNatureIds();
			String[] newNatures = new String[oldNatures.length+1];
			System.arraycopy(oldNatures, 0, newNatures, 0, oldNatures.length);
			newNatures[oldNatures.length] = TitaniumProjectNature.NATURE_ID;
			
			desc.setNatureIds(newNatures);
			project.setDescription(desc, monitor);
		}
	}
	
	@Override
	public boolean performFinish() {
		final String projectName = page.getProjectName();
		final String targetDir = page.getLocationPath().toOSString();
		
		try {
			getContainer().run(false, false, new IRunnableWithProgress() {
				 public void run(IProgressMonitor monitor)
						throws InvocationTargetException, InterruptedException {
					
					 try {

						IProjectDescription desc = ResourcesPlugin.getWorkspace().newProjectDescription(projectName);
						
						if (!page.useDefaults()) {
							desc.setLocationURI(page.getLocationURI());
						}
						
						IProject project = ResourcesPlugin.getWorkspace().getRoot().getProject(projectName);
						if (!project.exists())
							project.create(desc, new SubProgressMonitor(monitor, 1));
						if (!project.isOpen())
							project.open(monitor);
						
						addTitaniumNature(project, monitor);
						
						Process p = AppCommand.createProject(targetDir, projectName);
						IProcess process = DebugPlugin.newProcess(new Launch(null,"run",null), p, "app create:project");
						ProcessConsole console = new ProcessConsole(process, new ConsoleColorProvider());
						ConsolePlugin.getDefault().getConsoleManager().addConsoles(new IConsole[] { console });
						
						boolean running = true;
						do {
							try {
								p.exitValue();
								running = false;
							} catch (IllegalThreadStateException e) {
								Display.getDefault().readAndDispatch();
							}
						} while(running);
						
						project.refreshLocal(IResource.DEPTH_INFINITE, monitor);
					} catch (CoreException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
				
					 
				}
			});
		} catch (InvocationTargetException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		return true;
	}

	public void init(IWorkbench workbench, IStructuredSelection selection) {
		// TODO Auto-generated method stub

	}

}
