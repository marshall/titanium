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
	
	// generate machine id file if it doesn't exist
	var midf = TFS.getFile(dest,'.titanium');
	if (!midf.exists())
	{
		midf.write(Titanium.Platform.createUUID());
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
		var moduleDest = TFS.getFile(dest,'modules',Titanium.platform,p,results.map[p]);
		moduleDest.createDirectory(true);
		tasks.push({
			dest:moduleDest,
			files:moduleSrc
		});
		total++;
	}

	var files = runtimeSrc.getDirectoryListing();
	total++; // One more for the runtime
	moveby = 100 / total;

	tasks.push({
		dest:runtimeDir,
		files:runtimeSrc
	});
	updateProgressMessage(total+' files to install ... ');
	
	runCopyTasks(function()
	{
		updateProgressMessage('Configuring system paths ...');
		try
		{
			// Runtime template files are now in the
			// runtime distribution by default

			// developer product
			var devDest = TFS.getProgramsDirectory();
			if (Titanium.platform == 'linux')
			{
				devDest = TFS.getFile(Titanium.Process.getEnv('HOME'), "TitaniumApps");
				devDest.createDirectory(true);
			}
			var developer = Titanium.createApp(runtimeDir,devDest,'Titanium Developer','com.titaniumapp.developer',false);
			var devsrc = TFS.getFile(src,'developer');
			var devresources = TFS.getFile(devsrc,'Resources');
			var devtiapp = TFS.getFile(devsrc,'tiapp.xml');
			devtiapp.copy(developer.base);
			var devmanifest = TFS.getFile(devsrc,'manifest');
			devmanifest.copy(developer.base);

			TFS.asyncCopy(devresources,developer.resources,function()
			{
				current+=moveby;
				updateProgressValue(current);
				Titanium.Analytics.addEvent('ti.install');
				
				// add app shortcut in Start menu
				if (Titanium.platform == 'win32')
				{
					var userDir = TFS.getUserDirectory();
					var startMenu = TFS.getFile(userDir, "Start Menu");
					var to = TFS.getFile(startMenu, 'Titanium Developer');
					
					var exeFile = TFS.getFile(developer.executable);
					exeFile.createShortcut(to);
				}
				
				finishInstall(developer.executable);
			});

		}
		catch(E)
		{
			alert("error="+E);
		}
	});
	return true;
}


