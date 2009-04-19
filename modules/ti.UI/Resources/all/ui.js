/*!(c) 2008-2009 Appcelerator, Inc. http://appcelerator.org
 * Licensed under the Apache License, Version 2.0. Please visit
 * http://license.appcelerator.com for full copy of the License.
 **/

// A collection of JS patches for various UI functionality in Titanium

//
// execute in anonymous function block so now variables leak into the
// global scope
//
(function()
{
	// Add app:// support to jquery's http success function
	if (window.jQuery) 
	{
	 	var originalHttpSuccess = jQuery.httpSuccess;
		jQuery.extend({
			httpSuccess: function(r){
				if (location.protocol == 'app:' && r.status === 0) {
					return true;
				}
				return originalHttpSuccess.call(this,r);
			}
		}); 
	 }

	// adjust background transparency for window if needed
	if(Titanium.platform == "win32") {
		if(Titanium.UI.currentWindow.getTransparency() < 1) {
			var c = Titanium.UI.currentWindow.getTransparencyColor();
			document.body.style.background='#' + c;
		}
	}

	// append the platform (osx, linux, win32) to the body so we can dynamically
	// use platform specific CSS such as body.win32 div { } 
	var cn = (document.body.className || '');
	document.body.className =  cn + (cn ? ' ': '') + Titanium.platform;

	//
	// insert our user specific stylesheet in a generic way
	//
	var link = document.createElement('link');
	link.setAttribute('rel','stylesheet');
	link.setAttribute('href','ti://tiui/default.css');
	link.setAttribute('type','text/css');
	
	
	var headNodes = document.getElementsByTagName("head");
	if (headNodes && headNodes.length > 0)
	{
		var head = headNodes[0];
		// if we have children, insert at the top
		if (head.childNodes.length > 0)
		{
			head.insertBefore(link,head.childNodes[0]);
		}
		else
		{
			head.appendChild(link);
		}
	}
	else
	{
		// else we don't have a <head> element, just insert
		// in the body at the top
		if (document.body.childNodes.length > 0)
		{
			document.body.insertBefore(link,document.body.childNodes[0]);
		}
		else
		{
			document.body.appendChild(link);
		}
	}
	
	//
	// override console.log to also send into our API logger
	//
	var old_log = console.log;
	console.log = function(msg)
	{
		Titanium.API.debug(msg);
		return old_log(msg);
	};
	
})();
 