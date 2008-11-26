// this begins the hiding process of things we want exposed to our 
// internal code but not to the outside world (i.e. the application)
(function($)
{
	// internally map the tiRuntime object into a local variable
	// so we can continue to use it locally within our titanium.js
	// however, we want to remove it from the global scope so that
	// it's not accessible outside (i.e. the application)
	var runtime = tiRuntime;
	window.tiRuntime = null;

	// map our main native objects into the ti namespace
	// and hang ti off the global scope
	window.ti =
	{
		version: "<%=version%>",	// replaced at build time
		platform: "unknown",		// replaced below
		App: runtime.App,
		Dock: runtime.Dock,
		Menu: runtime.Menu,
		Window: runtime.Window
	};

	if (navigator.appVersion.indexOf("Win")!=-1) ti.platform = "win32";
	if (navigator.appVersion.indexOf("Mac")!=-1) ti.platform = "osx";
	if (navigator.appVersion.indexOf("Linux")!=-1) ti.platform = "linux";

	
	// expose jQuery into the ti namespace in case anyone wants to use
	// it outside of titanium.js
	ti.jQuery = $;

	// in the case that you have transparency on the window, we have to hack the browser's window
	// to cause the content view to take up 100% of the window (normally in HTML, it will only 
	// take up the height of the contained element size)
	var windowTransparency = ti.Window.currentWindow.getTransparency();
	if (windowTransparency < 1.0)
 	{
		var style = "margin:auto;padding:auto;";
		if (ti.Window.currentWindow.isUsingChrome())
		{
			// if we're using custom chrome, setup without borders, margin, etc.
			style = "margin:0;padding:0;border:none;";
		}
		document.write('<style>body { opacity:' + windowTransparency + '; ' + style + ' } body > DIV { height:100% }</style>');
	}
	
	var readies = [], loaded = false;
	
	ti.ready = function(fn)
	{
		if (!loaded)
		{
			readies.push(fn);
		}
		else
		{
			// make the scope the ti object and pass 
			// a reference to our jQuery pointer
			try
			{
				fn.call(ti,$);
			}
			catch (EE)
			{
				alert("Exception caught in ti.ready handler. Exception was: "+EE+" at line: "+EE.line);
			}
		}
	};
	
	$(function()
	{
		try
		{
			// we wrap in a try/catch
