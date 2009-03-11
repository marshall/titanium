/*!(c) 2006-2009 Appcelerator, Inc. http://appcelerator.org
 * Licensed under the Apache License, Version 2.0. Please visit
 * http://license.appcelerator.com for full copy of the License.
 **/

 // A collection of JS patches for various UI functionality in Titanium
 
// Add app:// support to jquery's http success function
Titanium.API.debug("HI!");
Titanium.API.debug("this="+this);
Titanium.API.debug("window="+window);


if ('jQuery' in window) {
 	Titanium.API.debug("jQuery in window");
 	Titanium.API.debug("Extending HTTP success..");
	var originalHttpSuccess = jQuery.httpSuccess;
	jQuery.extend({
		httpSuccess: function(r){
			if (location.protocol == 'app:' && r.status === 0) {
				return true;
			}
			return originalHttpSuccess.call(this,r);
		}
	});
	Titanium.API.debug("Extended."); 
 }
