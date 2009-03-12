$(document).ready(function()	
{
	$('#text_editor').markItUp(mySettings);
});


$MQL('l:launch.sandbox',function(msg)
{
	var project = {};
	project.name = "sandbox";
	project.url = "http://www.titaniumapp.com";
	project.rootdir = "/Users/nwright/Appcelerator"
	project.dir = "/Users/nwright/Appcelerator/" + project.name;
	project.appid = 'com.titanium.sandbox';
	project.publisher = "Titanium";
	project.image = '/Users/nwright/Documents/app_logo.png'

	var jsLibs = {};

	var outdir = TFS.getFile(project.rootdir,project.app);
	if (outdir.isDirectory())
	{
		outdir.deleteDirectory(true);
	}
	
	Titanium.Project.create(project.name,project.dir,project.publisher,project.url,project.image,jsLibs, $('#text_editor').val());

	TiDeveloper.Projects.launchProject(project,false)
})
