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

titanium.debug = function (msg) {
	TiNative.debug(msg);
}

titanium.include = function(path) {
	TiNative.include(path);
}

titanium.pluginsLoaded = function ()
{
	if (titanium.window.title)
	{
		document.title = titanium.window.title;
	}
}

titanium.appName = TiNative.getAppName();
titanium.endPoint = TiNative.getEndpoint();

titanium.window = {};
titanium.window.width = TiNative.getWindowWidth();
titanium.window.height = TiNative.getWindowHeight();
titanium.window.title = TiNative.getWindowTitle();

