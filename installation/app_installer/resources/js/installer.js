

$(function(){
	
	var appname = 'Tweetanium';
	var appurl = 'http://tweetanium.com';
	var appfile = 'foo.zip';
	var apppub = 'Appcelerator, Inc.';
	
	Titanium.UI.currentWindow.setTitle(appname + ' Installation');

	$('.appname').html(appname);
	$('#appfile').html(appfile);
	$('#apppub').html(apppub);
	$('#appurl').html(appurl);
	
	switch(Titanium.platform)
	{
		case 'osx':
		{
			$('#install_location').val('/Applications/Foo');
			break;
		}
		case 'win32':
		{
			break;
		}
		case 'linux':
		{
			break;
		}
	}
});