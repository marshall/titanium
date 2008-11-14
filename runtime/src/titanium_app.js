var TIAPP_XML = "tiapp.xml";

function TitaniumApp() {}

TitaniumApp.prototype.parseXML = function ()
{
	var tiapp_xml = ti.Path.resource(TIAPP_XML);
	var file = ti.File.open(tiapp_xml);
	var contents = file.read();	
	var doc = new ti.XML.Document(contents);

	this.name = doc.root().elementValue("name");
	
	var endpoint = doc.root().element("appc:endpoint");
	if (endpoint != null) {
		this.appceleratorEndpoint = endpoint.value;
	} else {
		this.appceleratorEndpoint = null;
	}
	
	this.windows = [];
	
	var app = this;
	doc.root().eachElement("window", function(window) {
		app.windows.push({
			width: window.attr("width"), height: window.attr("height"),
			title: window.elementValue("title"), start: window.elementValue("start")
		});
	});
};

TitaniumApp.prototype.getName = function ()
{
	return this.name;
};

TitaniumApp.prototype.getWindows = function ()
{
	return this.windows;
};

TitaniumApp.prototype.getAppceleratorEndpoint = function()
{
	return this.appceleratorEndpoint;
};

ti.App = new TitaniumApp();