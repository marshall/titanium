//
// Copyright 2006-2008 Appcelerator, Inc.
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//    http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

ti = {};

ti.eachPlugin = function (f) {
	for (var i = 0; i < ti.plugins.length; i++) {
		f(ti.plugins[i]);
	}
}

ti.pluginEvent = function(event) {
	ti.eachPlugin(function(plugin) {
		if (event in plugin) {
			plugin[event]();
		}
	});
}

ti.pluginsLoaded = function ()
{
	$(document).ready(function() {
		
		ti.pluginEvent("documentReady");
		
		//ti.App.include("ti://titanium_dock.js");
		//ti.App.include("ti://titanium_window.js");
		//ti.App.include("ti://titanium_menu.js");
		//ti.App.include("ti://titanium_file.js");	
		//ti.App.include("ti://titanium_xml.js");
	});
}

if (navigator.appVersion.indexOf("Win")!=-1) ti.platform = "win32";
if (navigator.appVersion.indexOf("Mac")!=-1) ti.platform = "osx";
if (navigator.appVersion.indexOf("Linux")!=-1) ti.platform = "linux";

// include this first so definition for include is setup
TiApp.include("ti://titanium_wrappers.js");
ti.App.include("ti://plugins.js");
ti.pluginsLoaded();