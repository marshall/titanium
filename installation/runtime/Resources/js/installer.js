var TFS = Titanium.Filesystem;

if (Titanium.platform=='osx')
{
	Titanium.UI.currentWindow.setHeight(500);
}

function updateProgressMessage(msg)
{
	$('#statusbar').html(msg);
}

function updateProgressValue(value)
{
	$('#progressbar').progressbar('value',value);
}

var tasks = [];
var total = 0;
var moveby = 0;
var current = 0;
var count = 0;
var taskTimer = null;

function runCopyTasks(fn)
{
	if (tasks.length > 0)
	{
		var task = tasks.shift();
		var onFileCopy = function(filename,_count,_total)
		{
			if (taskTimer) clearTimeout(taskTimer);
			$("#ajax").fadeOut();
			current+=moveby;
			count++;
			updateProgressMessage('Copying '+count+' of '+total+' files...');
			updateProgressValue(current);
			if (count == total)
			{
				fn();
			}
			else
			{
				runCopyTasks(fn);
			}
		};
		setTimeout(function()
		{
			$('#ajax').fadeIn();
		},2000)
		TFS.asyncCopy(task.files,task.dest,onFileCopy);
	}
}

function runInstaller()
{
	updateProgressMessage('Gathering installation details ... ');

	var src = Titanium.Process.getEnv('KR_HOME');
	var dest = Titanium.Process.getEnv('KR_RUNTIME_HOME');
	
	var runtimeDir = TFS.getFile(dest);
	if (!runtimeDir.isDirectory())
	{
		runtimeDir.createDirectory(true);
	}
	var manifest = TFS.getFile(src,'install');
	if (!manifest.isFile())
	{
		alert("Invalid runtime installer. Couldn't find manifest!");
		return false;
	}
	var results = Titanium.Project.getManifest(manifest);
	if (!results.success)
	{
		alert(results.message);
		return false;
	}
	var runtime = results.map['runtime'];
	if (!runtime)
	{
		alert("Invalid runtime installer. Couldn't find runtime in manifest!");
		return false;
	}
	var runtimeSrc = TFS.getFile(src,'runtime');
	if (!runtimeSrc.exists())
	{
		alert("Invalid runtime installer. Couldn't find runtime source directory!");
		return false;
	}
	var runtimeDir = TFS.getFile(dest,'runtime',Titanium.platform,runtime);
	//TODO: do we need to overwrite or confirm?
	if (!runtimeDir.exists())
	{
		if (!runtimeDir.createDirectory(true))
		{
			alert("Error installing runtime. Couldn't create directory: \n\n"+runtimeDir);
			return false;
		}
	}
	var appname = results.properties['appname'];

	for (p in results.map)
	{
		if (p == 'runtime') continue;
		var moduleSrc = TFS.getFile(src,'modules',p);
		if (!moduleSrc.exists()) continue;
		var moduleDest = TFS.getFile(dest,'modules',p,results.map[p]);
		moduleDest.createDirectory(true);
		tasks.push({
			dest:moduleDest,
			files:moduleSrc
		});
		total++;
	}
	var files = runtimeSrc.getDirectoryListing();
	total += 1;
	moveby = 100 / (total+4);

	updateProgressMessage(total+' files to install ... ');
	
	tasks.push({
		dest:runtimeDir,
		files:runtimeSrc
	});
	
	runCopyTasks(function()
	{
		updateProgressMessage('Configuring system paths ...');
		
		// create templates
		try
		{
			var template = TFS.getFile(runtimeDir,'template');
			template.createDirectory(true);

			switch(Titanium.platform)
			{
				case 'osx':
				 
					// link up WebKit libraries
					Titanium.linkLibraries(runtimeDir);
					var boot = TFS.getFile(src,'MacOS',appname);
					boot.copy(template);
					var target = TFS.getFile(template,appname);
					target.rename('kboot');
					target.setExecutable(true);
					var lproj = TFS.getFile(src,'Resources','English.lproj');
					var menu = TFS.getFile(lproj,'MainMenu.nib');
					var icons = TFS.getFile(lproj,'titanium.icns');
					menu.copy(template);
					icons.copy(template);
					
					break;
				case 'win32':
					// copy titanium_runtime.exe to template/kboot.exe
					var boot = TFS.getFile(src,appname+'.exe');
					var target = TFS.getFile(template,'kboot.exe');
					boot.copy(target);
					break;
				case 'linux':
					var boot = TFS.getFile(src,appname);
					boot.copy(template);
					var target = TFS.getFile(template,appname);
					target.rename('kboot');
					target.setExecutable(true);
					break;
			}
			
			current+=moveby;
			updateProgressValue(current);
			
			// developer product
			var devDest = TFS.getProgramsDirectory();
			var developer = Titanium.createApp(runtimeDir,devDest,'Titanium Developer','com.titaniumapp.developer',false);
			var devsrc = TFS.getFile(src,'developer');
			var devresources = TFS.getFile(devsrc,'resources');
			var devtiapp = TFS.getFile(devsrc,'tiapp.xml');
			devtiapp.copy(developer.base);
			var devmanifest = TFS.getFile(devsrc,'manifest');
			devmanifest.copy(developer.base);
			TFS.asyncCopy(devresources,developer.resources,function()
			{
				current+=moveby;
				updateProgressValue(current);
				// net installer product
				var net_installer_dest = TFS.getFile(runtimeDir,'installer');
				var net_installer = TFS.getFile(src,'installer');
				TFS.asyncCopy(net_installer,net_installer_dest,function()
				{
					current+=moveby;
					updateProgressValue(current);
					
					// app installer product
					var app_installer_dest = TFS.getFile(runtimeDir,'appinstaller');
					var app_installer = TFS.getFile(src,'appinstaller');
					TFS.asyncCopy(app_installer,app_installer_dest,function()
					{
						current+=moveby;
						updateProgressValue(current);
						finishInstall(developer.executable);
					});
				});
			});
					

		}
		catch(E)
		{
			alert("error="+E);
		}
	});
	return true;
}


