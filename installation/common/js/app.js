if (typeof(Titanium)=='undefined') Titanium = {};

Titanium.AppCreator = {
	
	osx: function(runtime,destination,name,appid)
	{
		var src = TFS.getFile(destination,name+'.app');
		src.createDirectory(true);
		var contents = TFS.getFile(src,'Contents');
		contents.createDirectory(true);
		var resources = TFS.getFile(contents,'Resources');
		resources.createDirectory(true);
		var macos = TFS.getFile(contents,'MacOS');
		macos.createDirectory(true);
		var lproj = TFS.getFile(resources,'English.lproj');
		lproj.createDirectory(true);

		var templates = TFS.getFile(runtime,'template');
		var fromMacos = TFS.getFile(templates,'kboot');
		fromMacos.copy(macos);
		var boot = TFS.getFile(macos,'kboot');
		boot.rename(name);
		boot.setExecutable(true);

		var mainMenu = TFS.getFile(templates,'MainMenu.nib');
		mainMenu.copy(lproj);

		var icns = TFS.getFile(templates,'titanium.icns');
		icns.copy(lproj);

		//TIXML, LINK, INFO, MAINMENU
		var plist = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"+
		"<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"+
		"<plist version=\"1.0\">\n"+
		"<dict>\n"+
		"	<key>CFBundleDevelopmentRegion</key>\n"+
		"	<string>English</string>\n"+
		"	<key>CFBundleExecutable</key>\n"+
		"	<string>"+name+"</string>\n"+
		"	<key>CFBundleIconFile</key>\n"+
		"	<string>titanium.icns</string>\n"+
		"	<key>CFBundleIdentifier</key>\n"+
		"	<string>"+appid+"</string>\n"+
		"	<key>CFBundleInfoDictionaryVersion</key>\n"+
		"	<string>6.0</string>\n"+
		"	<key>CFBundleName</key>\n"+
		"	<string>"+name+"</string>\n"+
		"	<key>CFBundlePackageType</key>\n"+
		"	<string>APPL</string>\n"+
		" 	<key>CFBundleSignature</key>\n"+
		"  	<string>WRUN</string>\n"+
		"  	<key>CFBundleVersion</key>\n"+
		"  	<string>0.1</string>\n"+
		"	<key>NSMainNibFile</key>\n"+
		"	<string>MainMenu</string>\n"+
		"	<key>NSPrincipalClass</key>\n"+
		"	<string>NSApplication</string>\n"+
		"</dict>\n"+
		"</plist>\n";

		var infoplist = TFS.getFile(contents,'Info.plist');
		infoplist.write(plist);

		return {
			resources:resources,
			base:contents,
			executable:src
		};
	},

	linux: function(runtime,destination,name,appid)
	{

	},

	win32: function(runtime,destination,name,appid)
	{

	}
};


Titanium.createApp = function(runtime,destination,name,appid)
{
	var platform = Titanium.platform;
	var fn = Titanium.AppCreator[platform];
	return fn(runtime,destination,name,appid);
};

