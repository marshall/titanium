if (typeof(Titanium)=='undefined') Titanium = {};

Titanium.AppCreator = {
	
	osx: function(runtime,destination,name,appid,installed)
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
		
		// set our marker file
		if (installed)
		{
			var marker = TFS.getFile(contents,'.installed');
			marker.write(String(new Date()));
		}

		return {
			resources:resources,
			base:contents,
			executable:src
		};
	},

	linux: function(runtime,destination,name,appid,installed)
	{

	},

	win32: function(runtime,destination,name,appid,installed)
	{
		var appDir = TFS.getFile(destination,name);
		appDir.createDirectory(true);
		var resources = TFS.getFile(appDir,'Resources');
		resources.createDirectory(true);

		var templates = TFS.getFile(runtime,'template');
		var kboot = TFS.getFile(templates,'kboot.exe');
		var appExecutable = TFS.getFile(appDir, name + '.exe');
		kboot.copy(appExecutable);
		
		// set our marker file
		if (installed)
		{
			var marker = TFS.getFile(appDir,'.installed');
			marker.write(String(new Date()));
		}

		return {
			resources:resources,
			base:appDir,
			executable:appExecutable
		};
	}
};


Titanium.createApp = function(runtime,destination,name,appid,installed)
{
	installed = (typeof(installed)=='undefined') ? true : installed;
	var platform = Titanium.platform;
	var fn = Titanium.AppCreator[platform];
	return fn(runtime,destination,name,appid,installed);
};

Titanium.linkLibraries = function(runtimeDir)
{
	if (Titanium.platform == 'osx')
	{
		var fw = ['WebKit','WebCore','JavaScriptCore'];
		for (var c=0;c<fw.length;c++)
		{
			var fwn = fw[c];
			var fwd = TFS.getFile(runtimeDir,fwn+'.framework');
			var fwd_name = fwd.name();
			var versions = TFS.getFile(fwd,'Versions');
			var ver = TFS.getFile(versions,'A');
			if (ver.exists()) continue; // skip if already linked
			var current = TFS.getFile(fwd,'Versions','Current');
			ver.createShortcut('Current',versions);
			var hf = TFS.getFile(fwd,'Headers');
			hf.createShortcut('Versions/Current/Headers',fwd);
			var ph = TFS.getFile(fwd,'PrivateHeaders');
			ph.createShortcut('Versions/Current/PrivateHeaders',fwd);
			var rf = TFS.getFile(fwd,'Resources');
			rf.createShortcut('Versions/Current/Resources',fwd);
		}
	}
};