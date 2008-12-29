package com.titaniumapp.project.wizard;

import java.io.File;

import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.jface.wizard.WizardPage;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.ModifyEvent;
import org.eclipse.swt.events.ModifyListener;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.DirectoryDialog;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Text;

import com.titaniumapp.project.TitaniumSharedImages;

public class TitaniumProjectConfigPage extends WizardPage {

	protected Text targetDirText, projectNameText;
	protected String targetDir, projectName;
	protected IProject targetProject;
	
	public TitaniumProjectConfigPage()
	{
		super("Titanium Project Configuration",
				"Titanium Project Configuration",
				TitaniumSharedImages.getImageDescriptor(TitaniumSharedImages.TITANIUM_64));
	}
	
	public void createControl(Composite parent) {
		Composite main = new Composite(parent, SWT.NONE);
		main.setLayoutData(new GridData(SWT.FILL, SWT.FILL, true, true));
		main.setLayout(new GridLayout(3, false));
	
		ModifyListener m = new ModifyListener() {
			public void modifyText(ModifyEvent e) {
				verify();
			}
		};
		
		projectName = "DemoApp";
		new Label(main, SWT.NONE).setText("Project name:");
		projectNameText = new Text(main, SWT.BORDER);
		GridData data = new GridData(SWT.FILL, SWT.FILL, true, false);
		data.horizontalSpan = 2;
		projectNameText.setLayoutData(data);
		projectNameText.setText(projectName);
		projectNameText.selectAll();
		projectNameText.setFocus();
		
		targetDir = ResourcesPlugin.getWorkspace().getRoot().getLocation().toOSString();
		
		new Label(main, SWT.NONE).setText("Project directory:");
		targetDirText = new Text(main, SWT.BORDER | SWT.SINGLE);
		data = new GridData(SWT.FILL, SWT.FILL, true, false);
		data.widthHint = 100;
		targetDirText.setLayoutData(data);
		targetDirText.setText(targetDir);
		
		projectNameText.addModifyListener(m);
		targetDirText.addModifyListener(m);
		
		Button browse = new Button(main, SWT.PUSH);
		browse.addSelectionListener(new SelectionAdapter() {
			public void widgetSelected(SelectionEvent e) {
				browseForTargetDir();
			}
		});
		browse.setText("Browse...");
		
		setControl(main);
	}
	
	protected void verify()
	{
		projectName = projectNameText.getText();
		targetDir = targetDirText.getText();
		targetProject = ResourcesPlugin.getWorkspace().getRoot().getProject(projectName);
		
		if (projectName == null || projectName.length() == 0)
		{
			setErrorMessage("Please provide the project name");
		}
		else if (!(new File(targetDir).exists())) {
			setErrorMessage("The target project directory does not exist");
		}
		else if (targetProject.exists()) {
			setErrorMessage("Project \"" + projectName + "\" already exists.");
		}
		else {
			setErrorMessage(null);
		}
	}

	protected void browseForTargetDir ()
	{
		DirectoryDialog dialog = new DirectoryDialog(getShell());
		String dir = dialog.open();
	
		if (dir != null) {
			targetDirText.setText(dir);
		}
	}

	public String getTargetDir() {
		return targetDir;
	}

	public void setTargetDir(String targetDir) {
		this.targetDir = targetDir;
	}

	public String getProjectName() {
		return projectName;
	}

	public void setProjectName(String projectName) {
		this.projectName = projectName;
	}

	public IProject getTargetProject() {
		return targetProject;
	}

	public void setTargetProject(IProject targetProject) {
		this.targetProject = targetProject;
	}
}
