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

titanium = {};
var ti = titanium;

titanium.debug = function (msg) {
	TiNative.debug(msg);
}

titanium.include = function(path) {
	TiNative.include(path);
}

titanium.eachPlugin = function (f) {
	for (var i = 0; i < ti.plugins.length; i++) {
		f(ti.plugins[i]);
	}
}

titanium.pluginEvent = function(event) {
	titanium.eachPlugin(function(plugin) {
		if (event in plugin) {
			plugin[event]();
		}
	});
}

titanium.pluginsLoaded = function ()
{
	$(document).ready(function() {
		ti.pluginEvent("documentReady");
		
		ti.include("ti:///titanium_file.js");	
		ti.include("ti:///titanium_xml.js");
		ti.include("ti:///titanium_app.js");
		ti.include("ti:///titanium_chrome.js");
		ti.App.parseXML();
		ti.Chrome.run();
	});
}

if (navigator.appVersion.indexOf("Win")!=-1) titanium.platform = "win32";
if (navigator.appVersion.indexOf("Mac")!=-1) titanium.platform = "osx";
if (navigator.appVersion.indexOf("Linux")!=-1) titanium.platform = "linux";

ti.include("ti:///plugins.js");
ti.pluginsLoaded();