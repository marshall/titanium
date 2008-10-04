Appcelerator.Titanium.Core = {};

Appcelerator.Titanium.Core.Console = {
	
	appendMessage: function (message, level, color)
	{
		var console = $('#console');
		
		var span = document.createElement("span");
		if (color != null) {
			span.setAttribute("style", "color: " + color + ";");
		}
		
		span.innerHTML = "["+level+"] " + message;
		$("#console").append(span).append("<br/>").attr("scrollTop", $("#console").attr("scrollHeight"));

	},
	
	appendInfo: function (message)
	{
		Appcelerator.Titanium.Core.Console.appendMessage(message, "INFO", null);
	},
	
	appendWarning: function (message)
	{
		Appcelerator.Titanium.Core.Console.appendMessage(message, "WARN", '#ff0');
	},
	
	appendError: function (message)
	{
		Appcelerator.Titanium.Core.Console.appendMessage(message, "ERROR", '#f00');
	},
	
	appendDebug: function (message)
	{
		Appcelerator.Titanium.Core.Console.appendMessage(message, "DEBUG", null);
	}
};