Appcelerator.Titanium.isDebugging = true;
Appcelerator.Titanium.Core = {};
Appcelerator.Titanium.Core.XML = {};

Appcelerator.Titanium.Core.XML.parseFromString = function (string)
{
	var doc = Sarissa.getDomDocument();
	return (new DOMParser()).parseFromString(string, "text/xml");	
};

Appcelerator.Titanium.Core.XML.parseFromURL = function (url) {
  var req = new XMLHttpRequest();
	req.open('GET', url, false); 
	req.send(null);
	return req.responseXML;
};

Appcelerator.Titanium.Core.XML.newDocument = function () {
	return Sarissa.getDomDocument();
}

Appcelerator.Titanium.Core.Console = {
	
	appendMessage: function (message, level, color)
	{
		var span = document.createElement("span");
		if (color != null) {
			span.setAttribute("style", "color: " + color + ";");
		}
		
		span.innerHTML = "["+level+"] " + message;
		$("#console_text").append(span).append("<br/>");
		$("#console_text").attr("scrollTop", $("#console_text").attr("scrollHeight"));
	},
	
	info: function (message)
	{
		Appcelerator.Titanium.Core.Console.appendMessage(message, "INFO", null);
	},
	
	warning: function (message)
	{
		Appcelerator.Titanium.Core.Console.appendMessage(message, "WARN", '#e5680c');
	},
	
	error: function (message)
	{
		Appcelerator.Titanium.Core.Console.appendMessage(message, "ERROR", '#e13e3e');
	},
	
	debug: function (message)
	{
		Appcelerator.Titanium.Core.Console.appendMessage(message, "DEBUG", "#d8e50c");
	},
	
	clear: function ()
	{
		$("#console_text").html("");
	}
};

var console = Appcelerator.Titanium.Core.Console;