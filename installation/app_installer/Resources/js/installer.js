var TFS = Titanium.Filesystem;
var src = Titanium.Process.getEnv('KR_APP_INSTALL_FROM');
var appname, appid, appurl, apppub, appimg;
var launch;

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

function installApp(dir)
{
	var runtime = TFS.getFile(Titanium.Process.getEnv('KR_RUNTIME'));
	var dest = TFS.getFile(dir);

	// create app structure
	var result = Titanium.createApp(runtime,dest,appname,appid,false);
	
	// remember in case we launch
	launch = result.executable;
	
	var files = TFS.getFile(src,'Resources').getDirectoryListing();

	var count = files.length + 2; // 2 files below
	var increment = 100 / count;
	var value = 0;
	
	TFS.getFile(src,'manifest').copy(result.base);
	value+=increment;
	$('#progressbar_install').progressbar('value',value)

	TFS.getFile(src,'tiapp.xml').copy(result.base);
	value+=increment;
	$('#progressbar_install').progressbar('value',value)
	
	TFS.asyncCopy(files,result.resources,function(fn,count,total)
	{
		value+=increment;
		$('#progressbar_install').progressbar('value',value)
		if (count == total)
		{
			// complete
			$('#install_app_finished').fadeIn();
			$('#install_app').slideUp(100);
		}
	});
}

$(function(){
	
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
	
	Titanium.UI.currentWindow.setTitle(appname + ' Installation');

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
