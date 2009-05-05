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
	window.onload=function()
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
		
		// Add app:// support to MooTool's Request class
		if (window.MooTools && typeof(Request) == "function")
		{
			Request.prototype.isSuccess = function()
			{
				return (((this.status >= 200) && (this.status < 300))
					|| (!!(window.Titanium) && (this.status == 0)));
			};
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
	};

	
	//
	// override console.log to also send into our API logger
	//
	var old_log = console.log;
	console.log = function(msg)
	{
		Titanium.API.debug(msg);
		return old_log(msg);
	};
	
	//
	// UI Dialog class
	//
	
	/**
	 * @tiapi(property=True,name=UI.isDialog,since=0.4) true if the current window is a UI Dialog
	 */
	Titanium.UI.isDialog = typeof(window._isDialog)=='undefined' ? false : window._isDialog;
	if (window._isDialog) try { delete window._isDialog } catch(e) {};

	var Dialog = function(params)
	{
		Titanium.API.debug("creating dialog with url: "+params.url);
		var kv = params.parameters || {};
		params.visible = true;
		this.window = Titanium.UI.createWindow(params);
		this.result = null;
		this.sub_dom_window = null;
		var w = this.window;
		var closed = false;
		var self = this;
		self.onclose = params.onclose;
		w.addEventListener(function(name,ev)
		{
			try
			{
				if (name == 'page.init')
				{
					self.sub_dom_window = ev.scope;
					self.sub_dom_window._isDialog = true;
					self.sub_dom_window._DialogParams = kv;
					self.sub_dom_window.Titanium.UI.isDialog = true;
					var old_close = self.sub_dom_window.Titanium.UI.currentWindow.close;
					self.sub_dom_window.Titanium.UI.currentWindow.close = function(r)
					{
						if (arguments.length > 0)
						{
							// only set it if it was passed in
							self.result = r;
						}
						old_close();
					};
				}
				else if (name == 'closed' && !closed)
				{
					closed = true;
					if (self.onclose)
					{
						self.onclose(self.result);
					}
					self.sub_dom_window = null;
				}
			}
			catch(e)
			{
				Titanium.API.error("error in "+name+", exception: "+e);
			}
		});
		w.open();
	};
	
	/**
	 * @tiapi(method=True,name=UI.Dialog.getResult,since=0.4) get results from UI dialog
	 */
	Dialog.prototype.getResult = function()
	{
		return this.result;
	};
	
	/**
	 * @tiapi(method=True,name=UI.Dialog.close,since=0.4) close UI dialog
	 */
	Dialog.prototype.close = function()
	{
		this.window.close();
	};
	
	/**
	 * @tiapi(method=True,name=UI.showDialog,since=0.4) create a UI dialog
	 * @tiarg(for=UI.showDialog,name=params,type=object) options to pass in to create window
	 * @tiresult(for=UI.showDialog,type=object) dialog instance
	 */
	Titanium.UI.showDialog = function(params)
	{
		return new Dialog(params);
	};
	
	/**
	 * @tiapi(method=True,name=UI.getDialogParam,since=0.4) get an incoming UI dialog parameter
	 * @tiarg(for=UI.getDialogParam,name=name,type=string) name of the parameter
	 * @tiarg(for=UI.getDialogParam,name=default,type=string,optional=True) default value if not found
	 * @tiresult(for=UI.getDialogParam,type=string) result
	 */
	Titanium.UI.getDialogParam = function(name,defvalue)
	{
		if (typeof(window._DialogParams)!='undefined')
		{
			var v = window._DialogParams[name];
			if (typeof(v)!='undefined')
			{
				return v;
			}
		}
		return defvalue;
	};

})();
 