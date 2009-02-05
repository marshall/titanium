function updateProgressMessage(msg)
{
	$('#statusbar').html(msg);
}

function finishInstall()
{
	$('#install').slideUp();
	$('#install_completed').fadeIn();
}

function parseEntry(entry)
{
	if (entry[0]=='#' || entry[0]==' ') return null;
	var i = entry.indexOf(':');
	if (i < 0) return null;
	var key = jQuery.trim(entry.substring(0,i));
	var value = jQuery.trim(entry.substring(i+1));
	return {
			key: key,
			value: value
	};
}

var tasks = [];
var total = 0;
var moveby = 0;
var current = 0;
var count = 0;

function runCopyTasks()
{
	if (tasks.length > 0)
	{
		var task = tasks.shift();
		var onFileCopy = function(filename,_count,_total)
		{
			current+=moveby;
			count++;
			updateProgressMessage('Copying '+count+' of '+total+' files...');
			$('#progressbar').progressbar('value',current);
			if (count == total)
			{
				copiers = null;
				finishInstall();
			}
			else
			{
				runCopyTasks();
			}
		};
		//FIXME: copy actually copies symlinks inside of moving them...
		Titanium.Filesystem.asyncCopy(task.files,task.dest,onFileCopy);
	}
}

function runInstaller()
{
	updateProgressMessage('Gathering installation details ... ');

	var src = Titanium.Process.getEnv('KR_HOME');
	var dest = Titanium.Process.getEnv('KR_RUNTIME_HOME');
	var runtimeDir = Titanium.Filesystem.getFile(dest);
	
	
	if (!runtimeDir.isDirectory())
	{
		runtimeDir.createDirectory(true);
	}
	var manifest = Titanium.Filesystem.getFile(src,'manifest');
	if (!manifest.isFile())
	{
		alert("Invalid runtime installer. Couldn't find manifest!");
		return false;
	}
	try
	{
		var line = manifest.readLine(true);
		var map = {};
		var entry = parseEntry(line);
		if (entry) map[entry.key]=entry.value;
		while (true)
		{
			line = manifest.readLine();
			if(!line) break;
			entry = parseEntry(line);
			if (entry) map[entry.key]=entry.value;
		}
		var runtime = map['runtime'];
		if (!runtime)
		{
			alert("Invalid runtime installer. Couldn't find runtime in manifest!");
			return false;
		}
		var runtimeSrc = Titanium.Filesystem.getFile(src,'runtime');
		if (!runtimeSrc.exists())
		{
			alert("Invalid runtime installer. Couldn't find runtime source directory!");
			return false;
		}
		var runtimeDir = Titanium.Filesystem.getFile(dest,'runtime',Titanium.platform,runtime);
		//TODO: do we need to overwrite or confirm?
		if (!runtimeDir.exists())
		{
			if (!runtimeDir.createDirectory(true))
			{
				alert("Error installing runtime. Couldn't create directory: \n\n"+runtimeDir);
				return false;
			}
		}

		for (p in map)
		{
			if (p == 'runtime' || p == 'appid' || p == 'appname') continue;
			var moduleSrc = Titanium.Filesystem.getFile(src,'modules',p);
			if (!moduleSrc.exists()) continue;
			var moduleDest = Titanium.Filesystem.getFile(dest,'modules',p,map[p]);
			moduleDest.createDirectory(true);
			tasks.push({
				dest:moduleDest,
				files:[moduleSrc]
			});
			total++;
		}
		
		var files = runtimeSrc.getDirectoryListing();
		total += files.length;
		moveby = 100/total;

		updateProgressMessage(total+' files to install ... ');
		
		tasks.push({
			dest:runtimeDir,
			files:files
		});
		
		//TODO: add installer, developer product, etc.
		//TODO: module and runtime directories not quite correct
		//TODO: fix symlink problem
		
		runCopyTasks();
	}
	catch(E)
	{
		alert('Error = '+E);
	}
	return true;
}

