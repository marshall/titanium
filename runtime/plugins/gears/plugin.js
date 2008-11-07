
function GearsPlugin() {
	ti.debug("in gears plugin constructor");
	ti.include("titanium/gears_" + titanium.platform + "/gears_init.js");
	ti.debug("inclded gears_"+titanium.platform+"/gears_init.js");
	
	ti.debug("init beta.desktop..google="+google);
	ti.desktop = google.gears.factory.create("beta.desktop");
	ti.debug("beta.desktop initialized..");
	
	//ti.localServer = google.gears.factory.create("beta.localserver");
	//ti.database = google.gears.factory.create("beta.database");
	//ti.geolocation = google.gears.factory.create("beta.geolocation");
	ti.debug("gears plugin loaded");
}

GearsPlugin.prototype.create = function(module)
{
	return google.gears.factory.create(module);
}

ti.gearsPlugin = new GearsPlugin();
ti.plugins.push(ti.gearsPlugin);
