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

titanium.pluginsLoaded = function ()
{
	ti.debug("including titanium_file");
	ti.include("titanium/titanium_file.js");
	ti.debug("ti.File = " + ti.File + ", ti.Path.resource = " + ti.Path.resource + ", ti.Path.join = " + ti.Path.join);
	
	ti.debug("including titanium_xml");
	ti.include("titanium/titanium_xml.js");
	ti.debug("ti.XML = " + ti.XML + ", ti.XML.Document=" + ti.XML.Document + ", ti.XML.Element="+ti.XML.Element);
	
	ti.debug("including titanium_app");
	ti.include("titanium/titanium_app.js");
	
	ti.debug("parsing xml... ti.App=" + ti.App);
	ti.App.parseXML();
}

if (navigator.appVersion.indexOf("Win")!=-1) titanium.platform = "win32";
if (navigator.appVersion.indexOf("Mac")!=-1) titanium.platform = "osx";
if (navigator.appVersion.indexOf("Linux")!=-1) titanium.platform = "linux";