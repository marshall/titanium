/*!(c) 2006-2009 Appcelerator, Inc. http://appcelerator.org
 * Licensed under the Apache License, Version 2.0. Please visit
 * http://license.appcelerator.com for full copy of the License.
 **/

 // A collection of JS patches for various UI functionality in Titanium
 
// Add app:// support to jquery's http success function
if ('jQuery' in window) {
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
if(Titanium) {
	if(Titanium.platform == "win32") {
		if(Titanium.UI.currentWindow.getTransparency() < 1) {
			var c = Titanium.UI.currentWindow.getTransparencyColor();
			document.body.style.background='#' + c;
		}
	}
}
