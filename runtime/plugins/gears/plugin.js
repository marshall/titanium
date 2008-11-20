
function GearsPlugin() {

}

GearsPlugin.prototype.create = function(module)
{
	return google.gears.factory.create(module);
};

GearsPlugin.prototype.documentReady = function()
{
	ti.App.include("ti://plugin/gears_" + ti.platform + "/gears_init.js");
	
	ti.App.debug("factory.create="+google.gears.factory.create);
	ti.Desktop = google.gears.factory.create("beta.desktop");
	
	//ti.localServer = google.gears.factory.create("beta.localserver");
	//ti.database = google.gears.factory.create("beta.database");
	//ti.geolocation = google.gears.factory.create("beta.geolocation");
};

ti.gearsPlugin = new GearsPlugin();
ti.plugins.push(ti.gearsPlugin);