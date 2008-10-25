titanium.include("titanium/gears/gears_init.js");

function GearsPlugin() {

	
	titanium.desktop = google.gears.factory.create("beta.desktop");
	titanium.localServer = google.gears.factory.create("beta.localserver");
	titanium.database = google.gears.factory.create("beta.database");
	titanium.geolocation = google.gears.factory.create("beta.geolocation");
}

titanium.plugins.push(new GearsPlugin());

debug("gears plugin loaded");