var TIAPP_XML = "tiapp.xml";

function TitaniumApp() {}

ti.debug("defining parseXML..");
TitaniumApp.prototype.parseXML = function ()
{
	ti.debug('in parseXML');
	var file = ti.File.open(ti.Path.resource(TIAPP_XML));
	ti.debug("have file: " + file);
	var contents = file.read();
	ti.debug("contents="+contents);
	
	var doc = new ti.XML.Document(contents);
	ti.debug("initialized doc: " + doc);
	
	this.name = doc.root().elementValue("name");
	this.windows = [];
	ti.debug("have doc: " + doc);
	
	var app = this;
	doc.root().eachElement("window", function(window) {
		app.windows.push({
			width: window.attr("width"), height: window.attr("height"),
			title: window.elementValue("title"), start: window.elementValue("start")
		});
	});
	
	ti.debug("done parsing");
};

ti.debug("getName..");
TitaniumApp.prototype.getName = function ()
{
	return this.name;
};

ti.debug("getWindows...")
TitaniumApp.prototype.getWindows = function ()
{
	return this.windows;
};

ti.debug("ti.App...");
ti.App = new TitaniumApp();