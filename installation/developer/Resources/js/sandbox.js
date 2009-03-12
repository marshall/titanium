$(document).ready(function()	{
	$('#text_editor').markItUp(mySettings);
});


$MQL('l:launch.sandbox',function(msg)
{
	Titanium.Project.create('sandbox','/Users/nwright/Appcelerator','Titanium','www.titaniumapp.com','/Users/nwright/Documents/app_logo.png',{}, $('#text_editor').val());
	var project = {};
	project.name = "sandbox";
	project.url = "http://www.titaniumapp.com";
	project.dir = "/Users/nwright/Appcelerator/sandbox";
	project.appid = 'com.titanium.sandbox';
	project.publisher = "titanium developer"
	project.image = '/Users/nwright/Documents/app_logo.png'
	TiDeveloper.Projects.launchProject(project,false)
})
