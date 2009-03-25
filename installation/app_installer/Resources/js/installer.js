var TFS = Titanium.Filesystem;
var src = Titanium.Process.getEnv('KR_APP_INSTALL_FROM');
var appname, appid, appurl, apppub, appimg;
var launch;
var win32 = false;

function launchApp()
{
	// cause the app to exit with special restart
	// code which will re-launch the original app
	if (launch)
	{
		try
		{
			Titanium.Desktop.openApplication(String(launch));
		}
		catch(E)
		{
			alert("Error launching: "+launch+". Message: "+E);
		}
	}
	Titanium.App.exit(0);
}

var install_started;

function finishInstall()
{
	var ts = new Date().getTime()-install_started;
	if (ts < 1000)
	{
		setTimeout(function()
		{
			finishInstall();
		},1000);
		
		return;
	}
	$('#install_app').slideUp(200);
	$('#install_app_finished').fadeIn();
}

function installApp(dir)
{
	if (!win32)
	{
		var dest = TFS.getFile(src,'.installed');
		dest.write("#"+new Date());
		try
		{
			Titanium.Process.restart();
		}
		catch(E)
		{
			alert(E);
		}
		return;
	}
	install_started = new Date().getTime();

	var runtime = TFS.getFile(Titanium.Process.getEnv('KR_RUNTIME'));
	var dest = TFS.getFile(dir);

	// create app structure
	var result = Titanium.createApp(runtime,dest,appname,appid,false);
	
	// remember in case we launch
	launch = result.executable;
	
	var resource_files = TFS.getFile(src, 'Resources');
	var module_files = TFS.getFile(src, 'modules');
	var runtime_files = TFS.getFile(src, 'runtime');

	// manifest + tiapp.xml (below) + app resources dir
	var count = 3;
	if (module_files.isDirectory())
	{
		count++;
	}
	if (runtime_files.isDirectory())
	{
		count++;
	}

	var increment = 100 / count;
	var value = 0;
	var total = 0;

	function fileCopied()
	{
		value += increment;
		total++;
		$('#progressbar_install').progressbar('value',value)

		if (count == total)
		{
			// complete
			finishInstall();
		}
	}

	TFS.getFile(src,'manifest').copy(result.base);
	fileCopied();
	TFS.getFile(src,'tiapp.xml').copy(result.base);
	fileCopied();

	TFS.asyncCopy(resource_files,result.resources,function(fn,c,t)
	{
		fileCopied(); 
		if (c == t)
		{
			if (module_files.isDirectory())
			{
				var module_dest = TFS.getFile(result.base, 'modules');
				TFS.asyncCopy(module_files,module_dest,function(fn,c,t) { fileCopied(); });
			}
			if (runtime_files.isDirectory())
			{
				var runtime_dest = TFS.getFile(result.base, 'runtime');
				TFS.asyncCopy(runtime_files,runtime_dest,function(fn,count,total) { fileCopied(); });
			}
		}
	});
}

$(function(){
	
	if (Titanium.platform != 'win32')
	{
		$('#install_screen').hide();
		$('#install_btn').html('Continue');
		Titanium.UI.currentWindow.setHeight(300);
	}
	else
	{
		win32 = true;
		Titanium.UI.currentWindow.setHeight(510);
	}
	Titanium.UI.currentWindow.setWidth(675);
	Titanium.UI.currentWindow.setX(Titanium.UI.currentWindow.CENTERED);
	Titanium.UI.currentWindow.setY(Titanium.UI.currentWindow.CENTERED);

	// show install info screen
	$('#installation_info').fadeIn();
	$('#install').removeClass('disabled');
	
	// create progress bars
	$('#progressbar').progressbar({value:0})
	$('#progressbar_install').progressbar({value:0})

	// cancel install click
	$('#cancel').click(function()
	{
		if (confirm('Are you sure you want to cancel?')==true)
		{
			Titanium.UI.currentWindow.close();
		}
	});
	
	// finish install click
	$('#finish').click(function()
	{
		if ($('#open_after_install').is(':checked'))
		{
			launchApp();
		}
		else
		{
			Titanium.App.exit(0);
		}
	})

	// install app click
	$('#install').click(function()
	{
		if (win32)
		{
			$('#installation_info').slideUp(100);
			Titanium.UI.currentWindow.setHeight(220)
			Titanium.UI.currentWindow.setX(Titanium.UI.currentWindow.CENTERED);

			// show installation screen
			$('#install_app').fadeIn()
		}
		else
		{
			Titanium.UI.currentWindow.setVisible(false);
		}
		// run the installer
		installApp($('#install_location').val());
	});
	
	$('#file_browse').click(function()
	{
		var props = {multiple:false};
		props.directories = true;
		props.files = false;

		Titanium.UI.openFiles(function(f)
		{
			if (f.length)
			{
				$('#install_location').val(f[0]);
			}
		},
		props);	
	});
	
	
	var manifest = TFS.getFile(src,'manifest');
	if (!manifest.isFile())
	{
		alert("Invalid application bundle. Couldn't find manifest!");
		return;
	}
	var results = Titanium.Project.getManifest(manifest);
	if (!results.success)
	{
		alert("Invalid application bundle. " + results.message);
		return;
	}
	
	appid = results.properties.appid;
	appname = results.properties.appname;
	appurl = results.properties.url || 'Not specified';
	apppub = results.properties.publisher || 'Unknown';
	appimg = results.properties.image;
	
	if (win32)
	{
		Titanium.UI.currentWindow.setTitle(appname + ' Installation');
		$('#intro').html('Information about the application you are about to install.');
	}
	else
	{
		Titanium.UI.currentWindow.setTitle(appname + ' Verification');
		$('#intro').html('Information about the application you are run.');
	}

	$('.appname').html(appname);
	$('#apppub').html(apppub).attr('href',appurl).click(function()
	{
		Titanium.Desktop.openURL(appurl);
	});
	$('#appurl').html(appurl).attr('href',appurl).click(function()
	{
		Titanium.Desktop.openURL(appurl);
	});
	if (appimg)
	{
		appimg = 'file://' + TFS.getFile(src,'Resources',appimg).nativePath();
		$('.appimg').attr('src',appimg);	
	}
	else
	{
		$('.appimg').css('visibility','hidden');
	}
	
	$('#install_location').val(TFS.getProgramsDirectory());
});
