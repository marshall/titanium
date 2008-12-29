package com.titaniumapp.project.appc;

import java.io.File;
import java.io.IOException;

import org.eclipse.core.resources.IProject;
import org.eclipse.core.runtime.IPath;
import org.eclipse.core.runtime.Platform;

public class AppCommand {

	protected static String getAppceleratorPath()
	{
		if (Platform.getOS() == Platform.OS_WIN32) {
			return "C:\\Program Files\\Appcelerator\\app.bat";
		}
		return "/usr/bin/app";
	}
	
	public static Process createProject(String targetDir, String projectName)
	{
		String[] command = new String[] {
				getAppceleratorPath(),
				"create:project",
				targetDir,
				projectName,
				"titanium"
		};
		
		try {
			return Runtime.getRuntime().exec(command);
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		return null;
	}
	
	public static Process packageProject(String projectDir) {
		if (Platform.getOS().equals(Platform.OS_MACOSX)) {
			return packageProject(projectDir, "osx");
		}
		else if (Platform.getOS().equals(Platform.OS_LINUX)) {
			return packageProject(projectDir, "linux");
		}
		else if (Platform.getOS().equals(Platform.OS_WIN32)) {
			return packageProject(projectDir, "win32");
		}
		return null;
	}
	
	public static IPath getExecutablePath(IProject project) {
		if (Platform.getOS().equals(Platform.OS_MACOSX)) {
			IPath path = project.getLocation();
			return path.append("stage").append(project.getName()+".app")
					.append("Contents").append("MacOS").append(project.getName());
		}
		else if (Platform.getOS().equals(Platform.OS_WIN32)) {
			IPath path = project.getLocation();
			return path.append("stage").append(project.getName()+".win32")
					.append(project.getName()+".exe");
		}
		
		return null;
	}
	
	public static Process packageProject(String projectDir, String os)
	{
		String[] command = new String[] {
			getAppceleratorPath(),
			"package:project",
			os,
			"--force"
		};
		
		try {
			return Runtime.getRuntime().exec(command, null, new File(projectDir));
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		return null;
	}
	
	public static Process runPackagedProject(IProject project)
	{
		String[] command = new String[] {
			getExecutablePath(project).toOSString()
		};
		
		try {
			return Runtime.getRuntime().exec(command);
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		return null;
	}
}
